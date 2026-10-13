#pragma once

#include "engine/PlaybackSnapshot.h"

#include <atomic>
#include <cstddef>

namespace dawn {

// Single producer (message thread), single consumer (audio thread).
// push() drops the event when the ring is full. Neither side allocates.
class LiveMidiQueue {
public:
    static constexpr int capacity = 256;

    bool push(ScheduledEvent event) noexcept;
    void popInto(MidiBlock& block) noexcept;

private:
    ScheduledEvent slots_[capacity]{};
    std::atomic<int> write_{0};
    std::atomic<int> read_{0};
};

} // namespace dawn
