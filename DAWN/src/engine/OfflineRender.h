#pragma once

#include "engine/AudioEngine.h"

#include <atomic>
#include <cstdint>

namespace dawn {

struct OfflineWriter {
    void* user = nullptr;
    void (*write)(void* user, const float* const* planes, int numChannels, int numSamples) = nullptr;
};

// Runs AudioEngine::process in a tight loop. This is the same call the
// soundcard callback makes. Do not pass the live engine; use a second instance.
// Returns false when cancelled. Progress is fraction of the requested range.
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
                   const std::atomic<bool>* cancel);

} // namespace dawn
