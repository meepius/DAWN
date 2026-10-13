#pragma once

#include "engine/AudioProcessingBlock.h"
#include "engine/LiveMidiQueue.h"
#include "engine/MidiScheduler.h"
#include "engine/PlaybackSnapshot.h"
#include "model/Project.h"

#include <atomic>
#include <cstdint>
#include <vector>

namespace dawn {

struct RenderInput {
    TrackId track;
    const float* mono = nullptr;
};

// Message thread: publish() and publishPlayback() and reclaim().
// Audio thread: process(). It loads two atomic pointers and does not lock.
class AudioEngine {
public:
    void prepare(double sampleRate, int maxBlock, int channels);
    void publish(const Project& project);
    void publishPlayback(const PlaybackSnapshot& playback);
    void setAudioRunning(bool running) noexcept;
    void setLiveMidi(void* user, void (*pop)(void*, MidiBlock&) noexcept) noexcept;
    bool adoptTail(void* user, void (*render)(void*, float* const*, int, int, float) noexcept, int fadeSamples) noexcept;
    void* reclaimTail() noexcept;
    void reclaim();

    void process(float* const* master,
                 int channels,
                 int samples,
                 const RenderInput* inputs,
                 int inputCount,
                 MidiBlock* midi = nullptr) noexcept;

    double playhead() const noexcept { return playhead_.load(std::memory_order_acquire); }

private:
    struct Retired {
        void* pointer = nullptr;
        void (*destroy)(void*) = nullptr;
        std::uint64_t epoch = 0;
    };

    template <typename T>
    void retire(T* previous) {
        if (previous == nullptr) return;
        retired_.push_back(Retired{previous, [](void* pointer) { delete static_cast<T*>(pointer); }, epoch_.load(std::memory_order_acquire)});
    }

    AudioProcessingBlock block_;
    std::atomic<GraphSnapshot*> published_{nullptr};
    std::atomic<PlaybackSnapshot*> playback_{nullptr};
    std::atomic<std::uint64_t> epoch_{0};
    std::atomic<bool> audioRunning_{false};
    std::atomic<double> playhead_{0.0};
    std::atomic<void*> liveUser_{nullptr};
    std::atomic<void (*)(void*, MidiBlock&) noexcept> livePop_{nullptr};
    std::vector<Retired> retired_;
    int channels_ = 2;
    double sampleRate_ = 48000.0;
    std::vector<const float*> inputScratch_;
    std::vector<ScheduledEvent> midiScratch_;
    TransportClock clock_;
    HeldNote held_[128]{};

    struct TailSlot {
        std::atomic<int> state{0};
        void* user = nullptr;
        void (*render)(void*, float* const*, int, int, float) noexcept = nullptr;
        int samplesLeft = 0;
        int total = 1;
    };
    TailSlot tails_[32];
};

} // namespace dawn
