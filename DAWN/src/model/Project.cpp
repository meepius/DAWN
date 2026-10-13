#include "model/Project.h"

#include <algorithm>

namespace dawn {

TrackId Project::addTrack(TrackKind kind, std::string name, TrackId requested) {
    if (requested.valid() && find(requested) != nullptr) return {};
    Track track;
    track.id = requested.valid() ? requested : TrackId{nextId_++};
    if (track.id.value >= nextId_) nextId_ = track.id.value + 1;
    track.name = std::move(name);
    track.kind = kind;
    const auto id = track.id;
    tracks_.push_back(std::move(track));
    topLevel_.push_back(id);
    return id;
}

bool Project::removeTrack(TrackId id) {
    auto* track = findMutable(id);
    if (track == nullptr) return false;

    if (track->kind == TrackKind::Group) {
        const auto kids = track->children;
        auto pos = std::find(topLevel_.begin(), topLevel_.end(), id);
        std::size_t at = pos == topLevel_.end() ? topLevel_.size()
                                                : static_cast<std::size_t>(pos - topLevel_.begin());
        if (pos != topLevel_.end()) topLevel_.erase(pos);
        for (std::size_t i = 0; i < kids.size(); ++i) {
            if (auto* child = findMutable(kids[i])) {
                child->group = {};
                if (child->output == id) child->output = {};
                topLevel_.insert(topLevel_.begin() + static_cast<std::ptrdiff_t>(at + i), kids[i]);
            }
        }
    } else {
        if (track->group.valid()) {
            if (auto* group = findMutable(track->group)) {
                std::erase(group->children, id);
            }
        }
        std::erase(topLevel_, id);
    }

    for (auto& other : tracks_) {
        if (other.output == id) other.output = {};
        std::erase_if(other.sends, [&](const Send& send) { return send.destination == id; });
        std::erase(other.children, id);
    }
    std::erase_if(sidechains_, [&](const SidechainRoute& route) { return route.source == id; });
    std::erase_if(tracks_, [&](const Track& item) { return item.id == id; });
    return true;
}

bool Project::reorder(TrackId id, std::size_t index) {
    auto* track = findMutable(id);
    if (track == nullptr) return false;
    auto* list = &topLevel_;
    if (track->group.valid()) {
        auto* group = findMutable(track->group);
        if (group == nullptr) return false;
        list = &group->children;
    }
    auto it = std::find(list->begin(), list->end(), id);
    if (it == list->end()) return false;
    list->erase(it);
    if (index > list->size()) index = list->size();
    list->insert(list->begin() + static_cast<std::ptrdiff_t>(index), id);
    return true;
}

TrackId Project::groupTracks(std::span<const TrackId> members, std::string name) {
    if (members.empty()) return {};
    std::size_t insertAt = topLevel_.size();
    for (auto id : members) {
        auto* track = findMutable(id);
        auto pos = std::find(topLevel_.begin(), topLevel_.end(), id);
        if (track == nullptr || pos == topLevel_.end()) return {};
        if (track->kind == TrackKind::Return || track->kind == TrackKind::Group) return {};
        insertAt = std::min(insertAt, static_cast<std::size_t>(pos - topLevel_.begin()));
    }

    Track group;
    group.id = TrackId{nextId_++};
    group.name = std::move(name);
    group.kind = TrackKind::Group;
    group.children.assign(members.begin(), members.end());
    const auto groupId = group.id;
    tracks_.push_back(std::move(group));

    for (auto id : members) {
        auto* track = findMutable(id);
        track->group = groupId;
        track->output = groupId;
        std::erase(topLevel_, id);
    }
    if (insertAt > topLevel_.size()) insertAt = topLevel_.size();
    topLevel_.insert(topLevel_.begin() + static_cast<std::ptrdiff_t>(insertAt), groupId);
    return groupId;
}

bool Project::assignGroup(TrackId childId, TrackId groupId) {
    auto* child = findMutable(childId);
    auto* group = findMutable(groupId);
    if (child == nullptr || group == nullptr || group->kind != TrackKind::Group) return false;
    if (child->kind == TrackKind::Group || child->kind == TrackKind::Return) return false;
    if (child->group.valid() && child->group != groupId) {
        if (auto* previous = findMutable(child->group)) std::erase(previous->children, childId);
    }
    const auto previousGroup = child->group;
    std::erase(topLevel_, childId);
    child->group = groupId;
    if (!child->output.valid() || child->output == previousGroup) child->output = groupId;
    if (std::find(group->children.begin(), group->children.end(), childId) == group->children.end()) {
        group->children.push_back(childId);
    }
    if (std::find(topLevel_.begin(), topLevel_.end(), groupId) == topLevel_.end()) topLevel_.push_back(groupId);
    return true;
}

bool Project::setOutput(TrackId source, TrackId destination) {
    auto* track = findMutable(source);
    if (track == nullptr) return false;
    if (!destination.valid()) {
        track->output = {};
        return true;
    }
    auto* dest = findMutable(destination);
    if (dest == nullptr || dest->kind != TrackKind::Group) return false;
    if (sendWouldCycle(source, destination)) return false;
    track->output = destination;
    return true;
}

bool Project::addSend(TrackId source, TrackId returnTrack, float level) {
    auto* track = findMutable(source);
    auto* ret = findMutable(returnTrack);
    if (track == nullptr || ret == nullptr || ret->kind != TrackKind::Return) return false;
    if (source == returnTrack) return false;
    if (track->kind == TrackKind::Return && sendWouldCycle(source, returnTrack)) return false;
    level = std::clamp(level, 0.f, 1.f);
    for (auto& send : track->sends) {
        if (send.destination == returnTrack) {
            send.level = level;
            return true;
        }
    }
    track->sends.push_back(Send{returnTrack, level});
    return true;
}

bool Project::setSidechain(std::uint64_t deviceId, TrackId source) {
    if (source.valid() && find(source) == nullptr) return false;
    std::erase_if(sidechains_, [&](const SidechainRoute& route) { return route.deviceId == deviceId; });
    if (source.valid()) sidechains_.push_back(SidechainRoute{deviceId, source});
    return true;
}

std::uint64_t Project::addDevice(TrackId track, DeviceKind) {
    auto* target = findMutable(track);
    if (target == nullptr) return 0;
    const auto id = nextId_++;
    target->deviceChain.push_back(id);
    return id;
}

bool Project::setMute(TrackId id, bool mute) {
    auto* track = findMutable(id);
    if (track == nullptr) return false;
    track->mute = mute;
    return true;
}

bool Project::setSolo(TrackId id, bool solo) {
    auto* track = findMutable(id);
    if (track == nullptr) return false;
    track->solo = solo;
    return true;
}

bool Project::setVolume(TrackId id, float volume) {
    auto* track = findMutable(id);
    if (track == nullptr) return false;
    track->volume = std::clamp(volume, 0.f, 1.5f);
    return true;
}

bool Project::setPan(TrackId id, float pan) {
    auto* track = findMutable(id);
    if (track == nullptr) return false;
    track->pan = std::clamp(pan, -1.f, 1.f);
    return true;
}

const Track* Project::find(TrackId id) const noexcept {
    for (const auto& track : tracks_) {
        if (track.id == id) return &track;
    }
    return nullptr;
}

Track* Project::findMutable(TrackId id) noexcept {
    for (auto& track : tracks_) {
        if (track.id == id) return &track;
    }
    return nullptr;
}

bool Project::sendWouldCycle(TrackId source, TrackId destination) const {
    std::vector<TrackId> pending{destination};
    std::vector<TrackId> seen;
    while (!pending.empty()) {
        const auto id = pending.back();
        pending.pop_back();
        if (id == source) return true;
        if (std::find(seen.begin(), seen.end(), id) != seen.end()) continue;
        seen.push_back(id);
        const auto* track = find(id);
        if (track == nullptr) continue;
        if (track->output.valid()) pending.push_back(track->output);
        if (track->group.valid()) pending.push_back(track->group);
        for (const auto& send : track->sends) pending.push_back(send.destination);
    }
    return false;
}

} // namespace dawn
