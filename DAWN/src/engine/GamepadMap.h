#pragma once

#include "engine/PlaybackSnapshot.h"

#include <cstdint>

namespace dawn {

struct GamepadFrame {
    bool connected = false;
    float axis[6]{};
    bool button[16]{};
};

struct GamepadBinding {
    int control = 0;
    bool axis = false;
    int channel = 0;
    int number = 60;
    bool cc = false;
};

// Emits edges only. lastCc[6] remembers the last 7-bit value per axis so a
// resting stick does not refill the MIDI queue. Returns the number written.
int translateGamepad(const GamepadFrame& previous,
                     const GamepadFrame& current,
                     const GamepadBinding* bindings,
                     int bindingCount,
                     int* lastCc,
                     ScheduledEvent* out,
                     int outCapacity) noexcept;

} // namespace dawn
