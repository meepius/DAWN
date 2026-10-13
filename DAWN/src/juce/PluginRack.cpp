#include "juce/PluginRack.h"

#include <algorithm>
#include <cmath>

namespace dawn {

struct PluginRack::Tail {
    std::unique_ptr<DeviceProcessor> device;
    juce::AudioBuffer<float> buffer;
    juce::MidiBuffer midi;

    static void render(void* user, float* const* output, int channels, int samples, float gain) noexcept {
        auto* self = static_cast<Tail*>(user);
        if (self->device == nullptr || output == nullptr || self->buffer.getNumChannels() <= 0) return;
        const int count = std::min(samples, self->buffer.getNumSamples());
        const int usedChannels = std::min(std::max(1, channels), self->buffer.getNumChannels());
        float* pointers[2] = {self->buffer.getWritePointer(0),
                              self->buffer.getNumChannels() > 1 ? self->buffer.getWritePointer(1) : self->buffer.getWritePointer(0)};
        juce::AudioBuffer<float> slice(pointers, usedChannels, count);
        slice.clear();
        self->midi.clear();
        self->device->process(slice, self->midi);
        for (int channel = 0; channel < channels; ++channel) {
            if (output[channel] == nullptr) continue;
            const float* source = slice.getReadPointer(std::min(channel, usedChannels - 1));
            for (int i = 0; i < count; ++i) output[channel][i] += source[i] * gain;
        }
    }
};

void PluginRack::handOff(std::unique_ptr<DeviceProcessor> device) {
    if (device == nullptr) return;
    if (engine_ == nullptr) {
        device->releaseResources();
        return;
    }
    auto* node = new Tail();
    node->buffer.setSize(2, std::max(1, maxBlock_), false, true, false);
    node->midi.ensureSize(4096);
    node->device = std::move(device);
    const int fade = std::max(1, static_cast<int>(std::lround(sampleRate_ * 3.0)));
    if (!engine_->adoptTail(node, &Tail::render, fade)) waiting_.push_back(node);
}

void PluginRack::clear() {
    for (auto& slot : slots_) handOff(std::move(slot.device));
    slots_.clear();
}

void PluginRack::retire(std::uint64_t id) {
    for (auto& slot : slots_) {
        if (slot.id != id) continue;
        handOff(std::move(slot.device));
    }
    std::erase_if(slots_, [](const Slot& slot) { return slot.device == nullptr; });
}

void PluginRack::add(std::uint64_t id, std::unique_ptr<DeviceProcessor> device) {
    if (device == nullptr || id == 0) return;
    for (auto& slot : slots_) {
        if (slot.id != id) continue;
        handOff(std::move(slot.device));
        device->prepareToPlay(sampleRate_, maxBlock_);
        slot.device = std::move(device);
        return;
    }
    device->prepareToPlay(sampleRate_, maxBlock_);
    slots_.push_back(Slot{id, std::move(device)});
}

DeviceProcessor* PluginRack::find(std::uint64_t id) noexcept {
    for (auto& slot : slots_) {
        if (slot.id == id) return slot.device.get();
    }
    return nullptr;
}

void PluginRack::prepare(double sampleRate, int maxBlock) {
    sampleRate_ = sampleRate > 0.0 ? sampleRate : 48000.0;
    maxBlock_ = maxBlock > 0 ? maxBlock : 512;
    for (auto& slot : slots_) {
        if (slot.device != nullptr) slot.device->prepareToPlay(sampleRate_, maxBlock_);
    }
}

void PluginRack::drain() {
    if (engine_ == nullptr) return;
    for (std::size_t i = 0; i < waiting_.size();) {
        const int fade = std::max(1, static_cast<int>(std::lround(sampleRate_ * 3.0)));
        if (!engine_->adoptTail(waiting_[i], &Tail::render, fade)) {
            ++i;
            continue;
        }
        waiting_.erase(waiting_.begin() + static_cast<std::ptrdiff_t>(i));
    }
    while (void* done = engine_->reclaimTail()) {
        auto* node = static_cast<Tail*>(done);
        if (node->device != nullptr) node->device->releaseResources();
        delete node;
    }
}

} // namespace dawn
