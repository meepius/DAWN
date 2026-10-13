#pragma once

#include <juce_audio_processors/juce_audio_processors.h>

namespace dawn {

// Shared by built-in devices and hosted plugins.
// prepareToPlay and releaseResources run on the message thread.
// process runs on the audio thread and must not allocate or take locks.
class DeviceProcessor {
public:
    virtual ~DeviceProcessor() = default;
    virtual void prepareToPlay(double sampleRate, int maxBlock) = 0;
    virtual void releaseResources() = 0;
    virtual void process(juce::AudioBuffer<float>& audio, juce::MidiBuffer& midi) noexcept = 0;
    virtual bool getStateInformation(juce::MemoryBlock& dest) { juce::ignoreUnused(dest); return false; }
    virtual void setStateInformation(const void* data, int size) { juce::ignoreUnused(data, size); }
};

} // namespace dawn
