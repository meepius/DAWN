#include "juce/InternalAnalogSynth.h"

#include <algorithm>

namespace dawn {

void InternalAnalogSynth::prepareToPlay(double sampleRate, int maxBlock) {
    maxBlock_ = std::max(1, maxBlock);
    mono_.assign(static_cast<std::size_t>(maxBlock_), 0.f);
    synth_.prepare(sampleRate, 8);
}

void InternalAnalogSynth::releaseResources() {
    mono_.clear();
    synth_.prepare(48000.0, 8);
}

void InternalAnalogSynth::process(juce::AudioBuffer<float>& audio, juce::MidiBuffer& midi) noexcept {
    const int samples = audio.getNumSamples();
    if (samples <= 0 || samples > static_cast<int>(mono_.size())) return;
    for (const auto metadata : midi) {
        const auto message = metadata.getMessage();
        if (message.isNoteOn()) synth_.noteOn(message.getNoteNumber(), message.getVelocity());
        else if (message.isNoteOff()) synth_.noteOff(message.getNoteNumber());
    }
    std::fill(mono_.begin(), mono_.begin() + samples, 0.f);
    synth_.process(mono_.data(), samples);
    for (int channel = 0; channel < audio.getNumChannels(); ++channel) {
        std::copy(mono_.begin(), mono_.begin() + samples, audio.getWritePointer(channel));
    }
}

} // namespace dawn
