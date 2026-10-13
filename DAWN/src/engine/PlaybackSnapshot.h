#pragma once

#include "model/MidiClip.h"

#include <cstdint>
#include <span>
#include <vector>

namespace dawn {

enum class TransportCommand : std::uint8_t { None, Play, Stop, Seek };

struct ClipLaunch {
    const MidiClip* clip = nullptr;
    double launchBeat = 0.0;
};

// Copied note data. Safe for the audio thread to read after publish.
struct PlaybackSnapshot {
    struct Note {
        double startBeat = 0.0;
        double lengthBeats = 0.0;
        std::uint8_t pitch = 0;
        std::uint8_t velocity = 0;
        std::uint8_t channel = 0;
    };

    struct Clip {
        TrackId track;
        double launchBeat = 0.0;
        double loopBeats = 4.0;
        int noteBegin = 0;
        int noteCount = 0;
    };

    std::vector<Clip> clips;
    std::vector<Note> notes;
    TransportCommand command = TransportCommand::None;
    std::uint32_t commandSerial = 0;
    double commandBeat = 0.0;
    double bpm = 120.0;

    static PlaybackSnapshot fromLaunches(std::span<const ClipLaunch> launches,
                                         TransportCommand command,
                                         std::uint32_t commandSerial,
                                         double commandBeat,
                                         double bpm);
};

struct ScheduledEvent {
    int sampleOffset = 0;
    std::uint8_t status = 0;
    std::uint8_t data1 = 0;
    std::uint8_t data2 = 0;
};

// Caller owns the storage. add() drops events instead of allocating.
struct MidiBlock {
    ScheduledEvent* events = nullptr;
    int capacity = 0;
    int count = 0;

    void add(int sampleOffset, std::uint8_t status, std::uint8_t data1, std::uint8_t data2) noexcept {
        if (events == nullptr || count >= capacity) return;
        events[count++] = ScheduledEvent{sampleOffset, status, data1, data2};
    }
};

} // namespace dawn
