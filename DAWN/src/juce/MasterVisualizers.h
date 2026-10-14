#pragma once

#include "juce/VisualizerDataQueue.h"

#include <juce_gui_basics/juce_gui_basics.h>
#include <juce_dsp/juce_dsp.h>

namespace dawn {

class Oscilloscope : public juce::Component {
public:
    void pushSamples(const float* mono, int count);
    void paint(juce::Graphics& graphics) override;

private:
    float samples_[512]{};
    int filled_ = 0;
};

class FFTSpectrumAnalyzer : public juce::Component {
public:
    void pushSamples(const float* mono, int count);
    void paint(juce::Graphics& graphics) override;

private:
    juce::dsp::FFT fft_{10};
    float fifo_[1024]{};
    int count_ = 0;
    float magnitudes_[512]{};
};

class MasterVisualizers : public juce::Component, private juce::Timer {
public:
    explicit MasterVisualizers(VisualizerDataQueue& queue);
    void resized() override;

private:
    void timerCallback() override;
    VisualizerDataQueue& queue_;
    Oscilloscope scope_;
    FFTSpectrumAnalyzer spectrum_;
};

} // namespace dawn
