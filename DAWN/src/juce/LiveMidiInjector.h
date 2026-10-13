#pragma once

#include "engine/PlaybackSnapshot.h"

#include <juce_audio_basics/juce_audio_basics.h>

namespace dawn {

// Message thread pushes. Audio thread pops inside AudioEngine::process.
// The slots are allocated in the constructor. A full queue drops the new event.
class LiveMidiInjector {
public:
    LiveMidiInjector();

    void push(const juce::MidiMessage& message) noexcept;
    static void pop(void* user, MidiBlock& block) noexcept;

private:
    void drain(MidiBlock& block) noexcept;

    juce::AbstractFifo fifo_;
    ScheduledEvent slots_[512]{};
};

} // namespace dawn
