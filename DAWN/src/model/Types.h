#pragma once

#include <cstdint>
#include <string>
#include <vector>

namespace dawn {

struct TrackId {
    uint64_t value = 0;
    constexpr bool valid() const noexcept { return value != 0; }
    friend constexpr bool operator==(TrackId a, TrackId b) noexcept { return a.value == b.value; }
};

enum class TrackKind : std::uint8_t { Midi, Audio, Group, Return };

enum class DeviceKind : std::uint8_t {
    Analog,
    DrumRack,
    Eq,
    Compressor,
    Delay,
    Reverb,
    Plugin,
};

struct Send {
    TrackId destination;
    float level = 0.f;
};

struct SidechainRoute {
    std::uint64_t deviceId = 0;
    TrackId source;
};

struct Track {
    TrackId id;
    std::string name;
    TrackKind kind = TrackKind::Midi;
    TrackId output;
    TrackId group;
    std::vector<TrackId> children;
    std::vector<Send> sends;
    std::vector<std::uint64_t> deviceChain;
    bool mute = false;
    bool solo = false;
    bool arm = false;
    float volume = 0.8f;
    float pan = 0.f;
};

} // namespace dawn
