#pragma once

#include "engine/AudioEngine.h"
#include "model/Project.h"

#include <juce_audio_formats/juce_audio_formats.h>
#include <atomic>
#include <cstdint>

namespace dawn {

struct BounceSettings {
    double sampleRate = 48000.0;
    int bitDepth = 24;
    std::int64_t startSample = 0;
    std::int64_t endSample = 0;
    juce::File file;
};

// Bounces on a background thread through a private AudioEngine, so the
// soundcard callback is never the writer. Progress is 0..1.
class OfflineRenderEngine : private juce::Thread {
public:
    OfflineRenderEngine();
    ~OfflineRenderEngine() override;

    void start(Project project, PlaybackSnapshot playback, BounceSettings settings);
    float progress() const noexcept { return progress_.load(std::memory_order_acquire); }
    bool finished() const noexcept { return finished_.load(std::memory_order_acquire); }
    bool ok() const noexcept { return ok_.load(std::memory_order_acquire); }
    void cancel() noexcept { cancel_.store(true, std::memory_order_release); }

private:
    void run() override;

    Project project_;
    PlaybackSnapshot playback_;
    BounceSettings settings_;
    std::atomic<float> progress_{0.f};
    std::atomic<bool> cancel_{false};
    std::atomic<bool> finished_{false};
    std::atomic<bool> ok_{false};
};

} // namespace dawn
