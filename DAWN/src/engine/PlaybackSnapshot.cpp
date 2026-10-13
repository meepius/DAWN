#include "engine/PlaybackSnapshot.h"

namespace dawn {

PlaybackSnapshot PlaybackSnapshot::fromLaunches(std::span<const ClipLaunch> launches,
                                               TransportCommand command,
                                               std::uint32_t commandSerial,
                                               double commandBeat,
                                               double bpm) {
    PlaybackSnapshot snapshot;
    snapshot.command = command;
    snapshot.commandSerial = commandSerial;
    snapshot.commandBeat = commandBeat;
    snapshot.bpm = bpm;
    for (const auto& launch : launches) {
        if (launch.clip == nullptr) continue;
        PlaybackSnapshot::Clip clip;
        clip.track = launch.clip->track();
        clip.launchBeat = launch.launchBeat;
        clip.loopBeats = launch.clip->loopBeats();
        clip.noteBegin = static_cast<int>(snapshot.notes.size());
        for (const auto& source : launch.clip->activeNotes()) {
            PlaybackSnapshot::Note note;
            note.startBeat = source.startBeat;
            note.lengthBeats = source.lengthBeats;
            note.pitch = source.pitch;
            note.velocity = source.velocity;
            note.channel = source.channel;
            snapshot.notes.push_back(note);
        }
        clip.noteCount = static_cast<int>(snapshot.notes.size()) - clip.noteBegin;
        snapshot.clips.push_back(clip);
    }
    return snapshot;
}

} // namespace dawn
