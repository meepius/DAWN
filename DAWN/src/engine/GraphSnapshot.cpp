#include "engine/GraphSnapshot.h"

#include <algorithm>
#include <unordered_map>
#include <unordered_set>

namespace dawn {
namespace {

bool ancestorSolo(const Project& project, TrackId id) {
    const auto* track = project.find(id);
    while (track != nullptr) {
        if (track->solo) return true;
        if (!track->group.valid()) return false;
        track = project.find(track->group);
    }
    return false;
}

bool ancestorMute(const Project& project, TrackId id) {
    const auto* track = project.find(id);
    while (track != nullptr) {
        if (track->mute) return true;
        if (!track->group.valid()) return false;
        track = project.find(track->group);
    }
    return false;
}

} // namespace

int GraphSnapshot::indexOf(TrackId id) const noexcept {
    for (int i = 0; i < static_cast<int>(nodes.size()); ++i) {
        if (nodes[static_cast<std::size_t>(i)].id == id) return i;
    }
    return -1;
}

GraphSnapshot GraphSnapshot::fromProject(const Project& project) {
    GraphSnapshot graph;
    graph.nodes.reserve(project.tracks().size());

    bool anySolo = false;
    for (const auto& track : project.tracks()) {
        if (track.kind != TrackKind::Return && ancestorSolo(project, track.id)) anySolo = true;
    }

    for (const auto& track : project.tracks()) {
        GraphNode node;
        node.id = track.id;
        node.kind = track.kind;
        node.volume = track.volume;
        node.pan = track.pan;
        const bool muted = ancestorMute(project, track.id);
        const bool soloed = ancestorSolo(project, track.id);
        node.audible = !muted && (track.kind == TrackKind::Return || !anySolo || soloed);
        graph.nodes.push_back(node);
    }

    for (auto& node : graph.nodes) {
        const auto* track = project.find(node.id);
        if (track == nullptr) continue;
        if (track->output.valid()) node.outputIndex = graph.indexOf(track->output);
        node.sendBegin = static_cast<int>(graph.sends.size());
        for (const auto& send : track->sends) {
            const int dest = graph.indexOf(send.destination);
            if (dest < 0) continue;
            graph.sends.push_back(GraphSend{dest, send.level});
        }
        node.sendCount = static_cast<int>(graph.sends.size()) - node.sendBegin;
    }

    for (const auto& route : project.sidechains()) {
        const int dest = graph.indexOf(route.source);
        for (auto& node : graph.nodes) {
            const auto* track = project.find(node.id);
            if (track == nullptr) continue;
            if (std::find(track->deviceChain.begin(), track->deviceChain.end(), route.deviceId) != track->deviceChain.end()) {
                node.sidechainIndex = dest;
            }
        }
    }

    std::vector<int> music;
    for (int i = 0; i < static_cast<int>(graph.nodes.size()); ++i) {
        if (graph.nodes[static_cast<std::size_t>(i)].kind != TrackKind::Return) music.push_back(i);
    }
    std::unordered_map<int, int> musicDegree;
    for (int index : music) musicDegree[index] = 0;
    struct Edge { int before; int after; };
    std::vector<Edge> edges;
    for (int index : music) {
        const auto* track = project.find(graph.nodes[static_cast<std::size_t>(index)].id);
        if (track == nullptr) continue;
        int targets[2] = {-1, -1};
        int count = 0;
        if (track->output.valid()) targets[count++] = graph.indexOf(track->output);
        if (track->group.valid()) targets[count++] = graph.indexOf(track->group);
        for (int t = 0; t < count; ++t) {
            if (!musicDegree.contains(targets[t])) continue;
            if (t == 1 && targets[1] == targets[0]) continue;
            edges.push_back(Edge{index, targets[t]});
            ++musicDegree[targets[t]];
        }
    }
    std::vector<int> ready;
    for (auto [index, degree] : musicDegree) {
        if (degree == 0) ready.push_back(index);
    }
    std::unordered_set<int> placed;
    while (!ready.empty()) {
        const int index = ready.back();
        ready.pop_back();
        if (!placed.insert(index).second) continue;
        graph.musicOrder.push_back(index);
        for (const auto& edge : edges) {
            if (edge.before != index) continue;
            if (--musicDegree[edge.after] == 0) ready.push_back(edge.after);
        }
    }
    for (int index : music) {
        if (!placed.contains(index)) graph.musicOrder.push_back(index);
    }

    std::vector<int> returns;
    for (int i = 0; i < static_cast<int>(graph.nodes.size()); ++i) {
        if (graph.nodes[static_cast<std::size_t>(i)].kind == TrackKind::Return) returns.push_back(i);
    }
    std::unordered_map<int, int> indegree;
    for (int index : returns) indegree[index] = 0;
    for (int index : returns) {
        const auto& node = graph.nodes[static_cast<std::size_t>(index)];
        for (int s = 0; s < node.sendCount; ++s) {
            const auto& send = graph.sends[static_cast<std::size_t>(node.sendBegin + s)];
            if (indegree.contains(send.destination)) ++indegree[send.destination];
        }
    }
    std::vector<int> returnReady;
    for (auto [index, degree] : indegree) {
        if (degree == 0) returnReady.push_back(index);
    }
    std::unordered_set<int> returnPlaced;
    while (!returnReady.empty()) {
        const int index = returnReady.back();
        returnReady.pop_back();
        if (!returnPlaced.insert(index).second) continue;
        graph.returnOrder.push_back(index);
        const auto& node = graph.nodes[static_cast<std::size_t>(index)];
        for (int s = 0; s < node.sendCount; ++s) {
            const int dest = graph.sends[static_cast<std::size_t>(node.sendBegin + s)].destination;
            auto it = indegree.find(dest);
            if (it == indegree.end()) continue;
            if (--it->second == 0) returnReady.push_back(dest);
        }
    }
    for (int index : returns) {
        if (!returnPlaced.contains(index)) graph.returnOrder.push_back(index);
    }
    return graph;
}

} // namespace dawn
