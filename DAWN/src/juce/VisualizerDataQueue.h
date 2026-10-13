#pragma once

#include <juce_audio_basics/juce_audio_basics.h>

namespace dawn {

// Audio thread pushes. Message thread pops. A full queue drops the new block.
class VisualizerDataQueue {
public:
    VisualizerDataQueue();

    void push(const float* left, const float* right, int samples) noexcept;
    int pop(float* left, float* right, int maxSamples) noexcept;

private:
    juce::AbstractFifo fifo_;
    float left_[8192]{};
    float right_[8192]{};
};

} // namespace dawn
