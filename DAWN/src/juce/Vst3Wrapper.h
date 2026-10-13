#pragma once

#include "juce/DeviceProcessor.h"

#include <memory>

namespace dawn {

// Hosts one VST3. Constructed on the message thread.
// CLAP and AudioUnit wrappers use this same DeviceProcessor shape.
class Vst3Wrapper : public DeviceProcessor {
public:
    explicit Vst3Wrapper(std::unique_ptr<juce::AudioPluginInstance> plugin);

    static std::unique_ptr<Vst3Wrapper> create(juce::AudioPluginFormatManager& formats,
                                               const juce::PluginDescription& description,
                                               double sampleRate,
                                               int maxBlock,
                                               juce::String& error);

    void prepareToPlay(double sampleRate, int maxBlock) override;
    void releaseResources() override;
    void process(juce::AudioBuffer<float>& audio, juce::MidiBuffer& midi) noexcept override;
    bool getStateInformation(juce::MemoryBlock& dest) override;
    void setStateInformation(const void* data, int size) override;

private:
    std::unique_ptr<juce::AudioPluginInstance> plugin_;
};

} // namespace dawn
