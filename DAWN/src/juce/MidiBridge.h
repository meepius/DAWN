#pragma once

#include "engine/PlaybackSnapshot.h"

#include <juce_audio_basics/juce_audio_basics.h>

namespace dawn {

// juce::MidiBuffer::addEvent can allocate if the buffer was not sized in prepare.
// Call dest.ensureSize() from audioDeviceAboutToStart, never from the callback.
inline void copyToMidiBuffer(juce::MidiBuffer& dest, const MidiBlock& block) {
    dest.clear();
    for (int i = 0; i < block.count; ++i) {
        const auto& event = block.events[i];
        const unsigned char bytes[3] = {event.status, event.data1, event.data2};
        dest.addEvent(bytes, 3, event.sampleOffset);
    }
}

} // namespace dawn
