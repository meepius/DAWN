#pragma once

#include "model/Types.h"

#include <cstddef>
#include <span>
#include <string>

namespace dawn {

// Message-thread project. The audio thread never sees this object.
// Tracks are created and destroyed freely. Routing is data, not a fixed graph.
class Project {
public:
    TrackId addTrack(TrackKind kind, std::string name, TrackId requested = {});
    bool removeTrack(TrackId id);
    bool reorder(TrackId id, std::size_t index);
    TrackId groupTracks(std::span<const TrackId> members, std::string name);
    bool assignGroup(TrackId child, TrackId group);
    bool setOutput(TrackId source, TrackId destination);
    bool addSend(TrackId source, TrackId returnTrack, float level);
    bool setSidechain(std::uint64_t deviceId, TrackId source);
    std::uint64_t addDevice(TrackId track, DeviceKind kind);

    bool setMute(TrackId id, bool mute);
    bool setSolo(TrackId id, bool solo);
    bool setVolume(TrackId id, float volume);
    bool setPan(TrackId id, float pan);

    const Track* find(TrackId id) const noexcept;
    const std::vector<Track>& tracks() const noexcept { return tracks_; }
    const std::vector<TrackId>& topLevel() const noexcept { return topLevel_; }
    const std::vector<SidechainRoute>& sidechains() const noexcept { return sidechains_; }

private:
    Track* findMutable(TrackId id) noexcept;
    bool sendWouldCycle(TrackId source, TrackId destination) const;

    std::uint64_t nextId_ = 1;
    std::vector<Track> tracks_;
    std::vector<TrackId> topLevel_;
    std::vector<SidechainRoute> sidechains_;
};

} // namespace dawn
