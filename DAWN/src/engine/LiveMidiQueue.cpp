#include "engine/LiveMidiQueue.h"

namespace dawn {

bool LiveMidiQueue::push(ScheduledEvent event) noexcept {
    const int write = write_.load(std::memory_order_relaxed);
    const int next = (write + 1) % capacity;
    if (next == read_.load(std::memory_order_acquire)) return false;
    slots_[write] = event;
    write_.store(next, std::memory_order_release);
    return true;
}

void LiveMidiQueue::popInto(MidiBlock& block) noexcept {
    const int read = read_.load(std::memory_order_relaxed);
    const int write = write_.load(std::memory_order_acquire);
    int index = read;
    while (index != write) {
        const auto& event = slots_[index];
        block.add(event.sampleOffset, event.status, event.data1, event.data2);
        index = (index + 1) % capacity;
    }
    read_.store(index, std::memory_order_release);
}

} // namespace dawn
