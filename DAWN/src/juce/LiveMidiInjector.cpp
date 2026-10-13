#include "juce/LiveMidiInjector.h"

namespace dawn {

LiveMidiInjector::LiveMidiInjector() : fifo_(512) {}

void LiveMidiInjector::push(const juce::MidiMessage& message) noexcept {
    const auto* raw = message.getRawData();
    const int size = message.getRawDataSize();
    if (raw == nullptr || size < 2 || message.isSysEx()) return;
    int start1 = 0;
    int size1 = 0;
    int start2 = 0;
    int size2 = 0;
    fifo_.prepareToWrite(1, start1, size1, start2, size2);
    if (size1 + size2 < 1) return;
    const int index = size1 > 0 ? start1 : start2;
    slots_[index] = ScheduledEvent{0, raw[0], raw[1], size > 2 ? raw[2] : std::uint8_t{0}};
    fifo_.finishedWrite(1);
}

void LiveMidiInjector::pop(void* user, MidiBlock& block) noexcept {
    if (user != nullptr) static_cast<LiveMidiInjector*>(user)->drain(block);
}

void LiveMidiInjector::drain(MidiBlock& block) noexcept {
    const int ready = fifo_.getNumReady();
    if (ready <= 0) return;
    int start1 = 0;
    int size1 = 0;
    int start2 = 0;
    int size2 = 0;
    fifo_.prepareToRead(ready, start1, size1, start2, size2);
    for (int i = 0; i < size1; ++i) {
        const auto& event = slots_[start1 + i];
        block.add(event.sampleOffset, event.status, event.data1, event.data2);
    }
    for (int i = 0; i < size2; ++i) {
        const auto& event = slots_[start2 + i];
        block.add(event.sampleOffset, event.status, event.data1, event.data2);
    }
    fifo_.finishedRead(size1 + size2);
}

} // namespace dawn
