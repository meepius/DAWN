#pragma once

#include "model/Project.h"

#include <cstdint>
#include <vector>

namespace dawn {

// Immutable mixer graph. Built on the message thread, then published.
// The audio thread only reads it.
struct GraphNode {
    TrackId id = {};
    TrackKind kind = TrackKind::Midi;
    int outputIndex = -1;
    float volume = 1.f;
    float pan = 0.f;
    bool audible = true;
    int sendBegin = 0;
    int sendCount = 0;
    int sidechainIndex = -1;
};

struct GraphSend {
    int destination = -1;
    float level = 0.f;
};

struct GraphSnapshot {
    std::vector<GraphNode> nodes;
    std::vector<int> musicOrder;
    std::vector<int> returnOrder;
    std::vector<GraphSend> sends;

    static GraphSnapshot fromProject(const Project& project);
    int indexOf(TrackId id) const noexcept;
};

} // namespace dawn
