#include "model/MidiClip.h"

#include <algorithm>
#include <cmath>

namespace dawn {

MidiClip::MidiClip(TrackId track, double loopBeats) : track_(track), loopBeats_(loopBeats) {
    if (loopBeats_ < 0.01) loopBeats_ = 0.01;
}

void MidiClip::setLoopBeats(double beats) {
    loopBeats_ = beats < 0.01 ? 0.01 : beats;
}

void MidiClip::addNote(MidiNote note) {
    normalize(note);
    if (note.lengthBeats <= 0.0) return;
    notes_.push_back(note);
    std::sort(notes_.begin(), notes_.end(), [](const MidiNote& a, const MidiNote& b) {
        return a.startBeat < b.startBeat;
    });
}

void MidiClip::clearNotes() { notes_.clear(); }

int MidiClip::addTake() {
    takes_.emplace_back();
    return static_cast<int>(takes_.size());
}

void MidiClip::addNoteToTake(int take, MidiNote note) {
    if (take <= 0 || take > static_cast<int>(takes_.size())) return;
    normalize(note);
    if (note.lengthBeats <= 0.0) return;
    auto& lane = takes_[static_cast<std::size_t>(take - 1)];
    lane.push_back(note);
}

void MidiClip::setComp(std::vector<CompRegion> regions) { regions_ = std::move(regions); }

std::vector<MidiNote> MidiClip::activeNotes() const {
    if (regions_.empty()) return notes_;
    std::vector<MidiNote> active;
    for (const auto& region : regions_) {
        if (region.lengthBeats <= 0.0) continue;
        const auto* lane = &notes_;
        if (region.take > 0 && region.take <= static_cast<int>(takes_.size())) {
            lane = &takes_[static_cast<std::size_t>(region.take - 1)];
        }
        const double end = region.startBeat + region.lengthBeats;
        for (const auto& note : *lane) {
            if (note.startBeat >= region.startBeat && note.startBeat < end) active.push_back(note);
        }
    }
    std::sort(active.begin(), active.end(), [](const MidiNote& a, const MidiNote& b) {
        return a.startBeat < b.startBeat;
    });
    return active;
}

void MidiClip::normalize(MidiNote& note) const {
    if (note.pitch > 127) note.pitch = 127;
    if (note.velocity > 127) note.velocity = 127;
    if (note.startBeat < 0.0) note.startBeat = 0.0;
    if (note.startBeat >= loopBeats_) note.startBeat = std::fmod(note.startBeat, loopBeats_);
}

} // namespace dawn
