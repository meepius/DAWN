#pragma once

#include "engine/GraphSnapshot.h"

#include <vector>

namespace dawn {

// Real-time mixer. prepare() may allocate. process() must not.
class AudioProcessingBlock {
public:
    void prepare(double sampleRate, int maxBlock, int channels, int maxNodes) noexcept;
    void process(const GraphSnapshot& graph,
                 const float* const* nodeInputs,
                 float* const* master,
                 int channels,
                 int samples) noexcept;

    double sampleRate() const noexcept { return sampleRate_; }
    bool ready() const noexcept { return ready_; }

private:
    float* plane(int node, int channel) noexcept;
    const float* plane(int node, int channel) const noexcept;

    double sampleRate_ = 48000.0;
    int maxBlock_ = 0;
    int channels_ = 0;
    int maxNodes_ = 0;
    bool ready_ = false;
    std::vector<float> storage_;
    std::vector<float> dry_;
};

} // namespace dawn
