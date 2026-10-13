#pragma once

#include "engine/PlaybackSnapshot.h"

#include <cstdint>

namespace dawn {

struct TransportClock {
    bool playing = false;
    double beat = 0.0;
    double bpm = 120.0;
    std::uint32_t appliedSerial = 0;
};

struct HeldNote {
    bool used = false;
    TrackId track;
    std::uint8_t pitch = 0;
    std::uint8_t channel = 0;
    double offBeat = 0.0;
};

// Fills midi with the events that belong to this block and advances the clock.
// held must be a fixed array. No allocation.
void scheduleMidi(const PlaybackSnapshot& snapshot,
                  TransportClock& clock,
                  HeldNote* held,
                  int heldCapacity,
                  double sampleRate,
                  int samples,
                  MidiBlock& midi) noexcept;

void sortMidiBlock(MidiBlock& midi) noexcept;

} // namespace dawn
