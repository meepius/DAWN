#include "engine/AnalogSynth.h"

#include <algorithm>
#include <cmath>

namespace dawn {

void AnalogSynth::prepare(double sampleRate, int voices) {
    sampleRate_ = sampleRate > 0.0 ? sampleRate : 48000.0;
    voiceCount_ = std::clamp(voices, 1, 16);
    for (auto& voice : voices_) voice = {};
}

void AnalogSynth::noteOn(int pitch, int velocity) noexcept {
    if (voiceCount_ <= 0) return;
    Voice* slot = nullptr;
    for (int i = 0; i < voiceCount_; ++i) {
        if (!voices_[i].active) {
            slot = &voices_[i];
            break;
        }
    }
    if (slot == nullptr) slot = &voices_[0];
    slot->active = true;
    slot->releasing = false;
    slot->pitch = std::clamp(pitch, 0, 127);
    slot->level = std::clamp(velocity, 0, 127) / 127.f;
    slot->envelope = slot->level;
    slot->frequency = 440.0 * std::pow(2.0, (slot->pitch - 69) / 12.0);
}

void AnalogSynth::noteOff(int pitch) noexcept {
    for (int i = 0; i < voiceCount_; ++i) {
        if (voices_[i].active && voices_[i].pitch == pitch) voices_[i].releasing = true;
    }
}

void AnalogSynth::process(float* mono, int samples) noexcept {
    if (mono == nullptr || samples <= 0 || voiceCount_ <= 0) return;
    const double stepScale = 1.0 / sampleRate_;
    const float release = 0.995f;
    for (int i = 0; i < samples; ++i) {
        float sample = 0.f;
        for (int v = 0; v < voiceCount_; ++v) {
            auto& voice = voices_[v];
            if (!voice.active) continue;
            const float saw = static_cast<float>(2.0 * (voice.phase - std::floor(voice.phase)) - 1.0);
            voice.phase += voice.frequency * stepScale;
            if (voice.phase >= 1.0) voice.phase -= std::floor(voice.phase);
            if (voice.releasing) {
                voice.envelope *= release;
                if (voice.envelope < 0.0008f) {
                    voice.active = false;
                    continue;
                }
            }
            sample += saw * voice.envelope * 0.2f;
        }
        mono[i] += sample;
    }
}

} // namespace dawn
