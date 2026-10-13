#include "engine/AudioProcessingBlock.h"

#include <algorithm>
#include <cmath>

namespace dawn {
namespace {

constexpr float kPi = 3.14159265358979323846f;

void clear(float* data, int count) noexcept {
    if (data != nullptr) std::fill(data, data + count, 0.f);
}

void addScaled(float* dest, const float* source, float gain, int count) noexcept {
    if (gain == 0.f || source == nullptr || dest == nullptr) return;
    for (int i = 0; i < count; ++i) dest[i] += source[i] * gain;
}

} // namespace

void AudioProcessingBlock::prepare(double sampleRate, int maxBlock, int channels, int maxNodes) noexcept {
    sampleRate_ = sampleRate > 0.0 ? sampleRate : 48000.0;
    maxBlock_ = std::max(1, maxBlock);
    channels_ = std::clamp(channels, 1, 2);
    maxNodes_ = std::max(1, maxNodes);
    storage_.assign(static_cast<std::size_t>(maxNodes_ * channels_ * maxBlock_), 0.f);
    dry_.assign(static_cast<std::size_t>(channels_ * maxBlock_), 0.f);
    ready_ = true;
}

float* AudioProcessingBlock::plane(int node, int channel) noexcept {
    const auto index = (static_cast<std::size_t>(node) * static_cast<std::size_t>(channels_) + static_cast<std::size_t>(channel))
                       * static_cast<std::size_t>(maxBlock_);
    return storage_.data() + index;
}

const float* AudioProcessingBlock::plane(int node, int channel) const noexcept {
    return const_cast<AudioProcessingBlock*>(this)->plane(node, channel);
}

void AudioProcessingBlock::process(const GraphSnapshot& graph,
                                   const float* const* nodeInputs,
                                   float* const* master,
                                   int channels,
                                   int samples) noexcept {
    if (!ready_ || master == nullptr || samples <= 0 || channels <= 0) return;
    channels = std::min(channels, channels_);
    if (samples > maxBlock_ || static_cast<int>(graph.nodes.size()) > maxNodes_) {
        for (int ch = 0; ch < channels; ++ch) clear(master[ch], samples);
        return;
    }

    const int nodeCount = static_cast<int>(graph.nodes.size());
    for (int node = 0; node < nodeCount; ++node) {
        for (int ch = 0; ch < channels_; ++ch) clear(plane(node, ch), samples);
    }

    auto mixNode = [&](int index) noexcept {
        const auto& node = graph.nodes[static_cast<std::size_t>(index)];
        const float* input = nodeInputs != nullptr ? nodeInputs[index] : nullptr;
        const float gain = node.audible ? node.volume : 0.f;
        const float angle = (std::clamp(node.pan, -1.f, 1.f) + 1.f) * 0.25f * kPi;
        const float left = channels == 1 ? 1.f : std::cos(angle);
        const float right = std::sin(angle);
        const float gains[2] = {gain * left, gain * right};

        for (int ch = 0; ch < channels; ++ch) {
            float* dest = dry_.data() + static_cast<std::size_t>(ch * maxBlock_);
            clear(dest, samples);
            addScaled(dest, plane(index, ch), 1.f, samples);
            addScaled(dest, input, 1.f, samples);
            const float tap = gains[ch];
            if (node.outputIndex < 0) addScaled(master[ch], dest, tap, samples);
            else addScaled(plane(node.outputIndex, ch), dest, tap, samples);
            for (int s = 0; s < node.sendCount; ++s) {
                const auto& send = graph.sends[static_cast<std::size_t>(node.sendBegin + s)];
                if (send.destination < 0 || send.destination >= nodeCount) continue;
                addScaled(plane(send.destination, ch), dest, tap * send.level, samples);
            }
        }
    };

    for (int index : graph.musicOrder) mixNode(index);
    for (int index : graph.returnOrder) mixNode(index);
}

} // namespace dawn
