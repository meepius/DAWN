#pragma once

#include "model/Types.h"

#include <cstdint>
#include <vector>

namespace dawn {

struct MidiNote {
    double startBeat = 0.0;
    double lengthBeats = 0.25;
    std::uint8_t pitch = 60;
    std::uint8_t velocity = 100;
    std::uint8_t channel = 0;
};

struct CompRegion {
    int take = 0;
    double startBeat = 0.0;
    double lengthBeats = 0.0;
};

// Message-thread clip. The audio thread never reads this object.
// Publishing copies activeNotes(), never the muted lanes.
class MidiClip {
public:
    MidiClip(TrackId track, double loopBeats);

    TrackId track() const noexcept { return track_; }
    double loopBeats() const noexcept { return loopBeats_; }
    void setLoopBeats(double beats);
    void addNote(MidiNote note);
    void clearNotes();
    const std::vector<MidiNote>& notes() const noexcept { return notes_; }

    int addTake();
    void addNoteToTake(int take, MidiNote note);
    void setComp(std::vector<CompRegion> regions);
    const std::vector<CompRegion>& comp() const noexcept { return regions_; }
    std::vector<MidiNote> activeNotes() const;

private:
    void normalize(MidiNote& note) const;

    TrackId track_;
    double loopBeats_ = 4.0;
    std::vector<MidiNote> notes_;
    std::vector<std::vector<MidiNote>> takes_;
    std::vector<CompRegion> regions_;
};

} // namespace dawn
