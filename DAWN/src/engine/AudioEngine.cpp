#include "engine/AudioEngine.h"

#include <algorithm>
#include <cmath>

namespace dawn {

void AudioEngine::prepare(double sampleRate, int maxBlock, int channels) {
    channels_ = std::clamp(channels, 1, 2);
    sampleRate_ = sampleRate > 0.0 ? sampleRate : 48000.0;
    block_.prepare(sampleRate_, maxBlock, channels_, 256);
    inputScratch_.assign(256, nullptr);
    midiScratch_.assign(4096, {});
}

void AudioEngine::publish(const Project& project) {
    auto* next = new GraphSnapshot(GraphSnapshot::fromProject(project));
    retire(published_.exchange(next, std::memory_order_acq_rel));
}

void AudioEngine::publishPlayback(const PlaybackSnapshot& playback) {
    auto* next = new PlaybackSnapshot(playback);
    retire(playback_.exchange(next, std::memory_order_acq_rel));
}

void AudioEngine::setAudioRunning(bool running) noexcept {
    audioRunning_.store(running, std::memory_order_release);
}

void AudioEngine::setLiveMidi(void* user, void (*pop)(void*, MidiBlock&) noexcept) noexcept {
    liveUser_.store(user, std::memory_order_relaxed);
    livePop_.store(pop, std::memory_order_release);
}

void AudioEngine::setMasterGain(float gain) noexcept {
    masterGain_.store(std::clamp(gain, 0.0f, 1.0f), std::memory_order_relaxed);
}

void AudioEngine::setMetronome(bool enabled) noexcept {
    metronome_.store(enabled, std::memory_order_relaxed);
}

void AudioEngine::setRecording(bool recording) noexcept {
    recording_.store(recording, std::memory_order_release);
}

bool AudioEngine::adoptTail(void* user, void (*render)(void*, float* const*, int, int, float) noexcept, int fadeSamples) noexcept {
    if (user == nullptr || render == nullptr) return false;
    if (fadeSamples < 1) fadeSamples = 1;
    for (auto& slot : tails_) {
        int empty = 0;
        if (!slot.state.compare_exchange_strong(empty, 3, std::memory_order_acq_rel)) continue;
        slot.user = user;
        slot.render = render;
        slot.total = fadeSamples;
        slot.samplesLeft = fadeSamples;
        slot.state.store(1, std::memory_order_release);
        return true;
    }
    return false;
}

void* AudioEngine::reclaimTail() noexcept {
    const bool running = audioRunning_.load(std::memory_order_acquire);
    for (auto& slot : tails_) {
        const int state = slot.state.load(std::memory_order_acquire);
        if (state != 2 && !(state == 1 && !running)) continue;
        void* user = slot.user;
        slot.user = nullptr;
        slot.render = nullptr;
        slot.state.store(0, std::memory_order_release);
        return user;
    }
    return nullptr;
}

void AudioEngine::reclaim() {
    const auto seen = epoch_.load(std::memory_order_acquire);
    const bool running = audioRunning_.load(std::memory_order_acquire);
    std::erase_if(retired_, [&](Retired& item) {
        if (running && item.epoch >= seen) return false;
        if (item.destroy != nullptr) item.destroy(item.pointer);
        item.pointer = nullptr;
        item.destroy = nullptr;
        return true;
    });
}

void AudioEngine::process(float* const* master,
                          int channels,
                          int samples,
                          const RenderInput* inputs,
                          int inputCount,
                          MidiBlock* midi) noexcept {
    MidiBlock local{midiScratch_.data(), static_cast<int>(midiScratch_.size()), 0};
    MidiBlock& block = midi != nullptr && midi->events != nullptr ? *midi : local;
    block.count = 0;

    const auto* graph = published_.load(std::memory_order_acquire);
    if (graph == nullptr || !block_.ready()) {
        if (master != nullptr) {
            for (int ch = 0; ch < channels; ++ch) {
                if (master[ch] != nullptr) std::fill(master[ch], master[ch] + samples, 0.f);
            }
        }
    } else {
        const int nodes = static_cast<int>(graph->nodes.size());
        if (static_cast<int>(inputScratch_.size()) < nodes) {
            for (int ch = 0; ch < channels; ++ch) {
                if (master != nullptr && master[ch] != nullptr) std::fill(master[ch], master[ch] + samples, 0.f);
            }
        } else {
            std::fill(inputScratch_.begin(), inputScratch_.begin() + nodes, nullptr);
            for (int i = 0; i < inputCount; ++i) {
                const int index = graph->indexOf(inputs[i].track);
                if (index >= 0) inputScratch_[static_cast<std::size_t>(index)] = inputs[i].mono;
            }
            block_.process(*graph, inputScratch_.data(), master, channels, samples);
        }
    }

    if (const auto* playback = playback_.load(std::memory_order_acquire)) {
        scheduleMidi(*playback, clock_, held_, 128, sampleRate_, samples, block);
        playhead_.store(clock_.beat, std::memory_order_release);
    }

    if (auto* pop = livePop_.load(std::memory_order_acquire)) {
        pop(liveUser_.load(std::memory_order_acquire), block);
        sortMidiBlock(block);
    }

    if (master != nullptr) {
        for (auto& slot : tails_) {
            if (slot.state.load(std::memory_order_acquire) != 1 || slot.render == nullptr) continue;
            const float gain = slot.total > 0 ? static_cast<float>(slot.samplesLeft) / static_cast<float>(slot.total) : 0.f;
            slot.render(slot.user, master, channels, samples, gain);
            slot.samplesLeft -= samples;
            if (slot.samplesLeft <= 0) slot.state.store(2, std::memory_order_release);
        }
    }

    if (master != nullptr && metronome_.load(std::memory_order_relaxed) && clock_.playing) {
        const double beat = clock_.beat;
        if (beat > metronomeBeat_ && std::floor(beat) != std::floor(metronomeBeat_)) {
            const bool downbeat = std::fmod(std::floor(beat), 4.0) < 0.5;
            clickLevel_ = downbeat ? 0.32f : 0.14f;
        }
        metronomeBeat_ = beat;
    }
    if (master != nullptr && clickLevel_ > 0.0008f) {
        for (int i = 0; i < samples; ++i) {
            const float tick = clickLevel_;
            clickLevel_ *= 0.94f;
            for (int ch = 0; ch < channels; ++ch) {
                if (master[ch] != nullptr) master[ch][i] += tick;
            }
        }
    }
    if (master != nullptr) {
        const float gain = masterGain_.load(std::memory_order_relaxed);
        if (gain != 1.0f) {
            for (int ch = 0; ch < channels; ++ch) {
                if (master[ch] == nullptr) continue;
                for (int i = 0; i < samples; ++i) master[ch][i] *= gain;
            }
        }
    }

    epoch_.fetch_add(1, std::memory_order_release);
}

} // namespace dawn
