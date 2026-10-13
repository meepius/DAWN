#include "engine/OfflineRender.h"

#include <algorithm>
#include <vector>

namespace dawn {

bool renderOffline(AudioEngine& engine,
                   int blockSize,
                   int channels,
                   std::int64_t startSample,
                   std::int64_t endSample,
                   TrackId inputTrack,
                   const float* input,
                   std::int64_t inputSamples,
                   const OfflineWriter& writer,
                   std::atomic<float>* progress,
                   const std::atomic<bool>* cancel) {
    if (blockSize <= 0 || channels <= 0 || endSample <= startSample) return false;
    blockSize = std::min(blockSize, 4096);
    channels = std::min(channels, 2);
    std::vector<std::vector<float>> buffers(static_cast<std::size_t>(channels), std::vector<float>(static_cast<std::size_t>(blockSize), 0.f));
    std::vector<float*> planes(static_cast<std::size_t>(channels));
    std::vector<ScheduledEvent> events(1024);
    const auto span = static_cast<float>(endSample - startSample);
    std::int64_t cursor = 0;
    while (cursor < endSample) {
        if (cancel != nullptr && cancel->load(std::memory_order_acquire)) return false;
        const int count = static_cast<int>(std::min<std::int64_t>(blockSize, endSample - cursor));
        for (int channel = 0; channel < channels; ++channel) {
            std::fill(buffers[static_cast<std::size_t>(channel)].begin(), buffers[static_cast<std::size_t>(channel)].begin() + count, 0.f);
            planes[static_cast<std::size_t>(channel)] = buffers[static_cast<std::size_t>(channel)].data();
        }
        RenderInput injected{};
        const bool hasInput = input != nullptr && inputTrack.valid() && cursor < inputSamples;
        if (hasInput) injected = RenderInput{inputTrack, input + cursor};
        MidiBlock midi{events.data(), static_cast<int>(events.size()), 0};
        engine.process(planes.data(), channels, count, hasInput ? &injected : nullptr, hasInput ? 1 : 0, &midi);
        if (cursor + count > startSample && writer.write != nullptr) {
            const int skip = cursor < startSample ? static_cast<int>(startSample - cursor) : 0;
            const float* sliced[2] = {};
            for (int channel = 0; channel < channels; ++channel) sliced[channel] = planes[static_cast<std::size_t>(channel)] + skip;
            writer.write(writer.user, sliced, channels, count - skip);
        }
        cursor += count;
        if (progress != nullptr) {
            const auto done = static_cast<float>(std::max<std::int64_t>(0, cursor - startSample));
            progress->store(std::min(1.f, done / span), std::memory_order_release);
        }
    }
    if (progress != nullptr) progress->store(1.f, std::memory_order_release);
    return true;
}

} // namespace dawn
