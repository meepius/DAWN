#pragma once

#include "engine/AnalogSynth.h"
#include "juce/DeviceProcessor.h"

#include <vector>

namespace dawn {

class InternalAnalogSynth : public DeviceProcessor {
public:
    void prepareToPlay(double sampleRate, int maxBlock) override;
    void releaseResources() override;
    void process(juce::AudioBuffer<float>& audio, juce::MidiBuffer& midi) noexcept override;

private:
    AnalogSynth synth_;
    int maxBlock_ = 0;
    std::vector<float> mono_;
};

} // namespace dawn
