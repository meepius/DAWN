#include "juce/ProjectState.h"

#include "engine/PlaybackSnapshot.h"

#include <deque>
#include <vector>

namespace dawn {

ProjectState::ProjectState(AudioEngine& engine) : engine_(engine) {
    state_.setProperty(Id::bpm, 120.0, nullptr);
    state_.setProperty(Id::nextId, static_cast<juce::int64>(1), nullptr);
    state_.setProperty(Id::command, static_cast<int>(TransportCommand::None), nullptr);
    state_.setProperty(Id::commandSerial, 0, nullptr);
    state_.setProperty(Id::commandBeat, 0.0, nullptr);
    state_.addListener(this);
    publish();
}

ProjectState::~ProjectState() { state_.removeListener(this); }

juce::ValueTree ProjectState::addTrack(TrackKind kind, const juce::String& name) {
    juce::ValueTree track{Id::track};
    track.setProperty(Id::trackId, static_cast<juce::int64>(takeId().value), nullptr);
    track.setProperty(Id::name, name, nullptr);
    track.setProperty(Id::kind, static_cast<int>(kind), nullptr);
    track.setProperty(Id::volume, 0.8f, nullptr);
    track.setProperty(Id::pan, 0.0f, nullptr);
    track.setProperty(Id::mute, false, nullptr);
    track.setProperty(Id::solo, false, nullptr);
    state_.appendChild(track, &undo_);
    return track;
}

void ProjectState::setVolume(const juce::ValueTree& track, float volume) {
    track.setProperty(Id::volume, volume, &undo_);
}

juce::ValueTree ProjectState::addClip(const juce::ValueTree& track, double loopBeats) {
    juce::ValueTree clip{Id::clip};
    clip.setProperty(Id::loopBeats, loopBeats, nullptr);
    clip.setProperty(Id::launchBeat, 0.0, nullptr);
    clip.setProperty(Id::launched, false, nullptr);
    track.appendChild(clip, &undo_);
    return clip;
}

void ProjectState::addNote(const juce::ValueTree& clip, int pitch, double start, double length, int velocity) {
    juce::ValueTree note{Id::note};
    note.setProperty(Id::pitch, pitch, nullptr);
    note.setProperty(Id::start, start, nullptr);
    note.setProperty(Id::length, length, nullptr);
    note.setProperty(Id::velocity, velocity, nullptr);
    clip.appendChild(note, &undo_);
}

void ProjectState::setLaunched(const juce::ValueTree& clip, bool launched, double launchBeat) {
    clip.setProperty(Id::launchBeat, launchBeat, nullptr);
    clip.setProperty(Id::launched, launched, nullptr);
}

void ProjectState::play(double fromBeat) {
    const int serial = static_cast<int>(state_[Id::commandSerial]) + 1;
    state_.setProperty(Id::commandBeat, fromBeat, nullptr);
    state_.setProperty(Id::command, static_cast<int>(TransportCommand::Play), nullptr);
    state_.setProperty(Id::commandSerial, serial, nullptr);
}

void ProjectState::stop() {
    const int serial = static_cast<int>(state_[Id::commandSerial]) + 1;
    state_.setProperty(Id::command, static_cast<int>(TransportCommand::Stop), nullptr);
    state_.setProperty(Id::commandSerial, serial, nullptr);
}

void ProjectState::replace(juce::ValueTree next) {
    state_.removeListener(this);
    undo_.clearUndoHistory();
    state_ = std::move(next);
    if (!state_.hasType(Id::project)) state_ = juce::ValueTree{Id::project};
    state_.addListener(this);
    publish();
}

TrackId ProjectState::takeId() {
    auto next = static_cast<std::uint64_t>(static_cast<juce::int64>(state_[Id::nextId]));
    if (next == 0) next = 1;
    state_.setProperty(Id::nextId, static_cast<juce::int64>(next + 1), nullptr);
    return TrackId{next};
}

void ProjectState::valueTreePropertyChanged(juce::ValueTree&, const juce::Identifier&) { publish(); }
void ProjectState::valueTreeChildAdded(juce::ValueTree&, juce::ValueTree&) { publish(); }
void ProjectState::valueTreeChildRemoved(juce::ValueTree&, juce::ValueTree&, int) { publish(); }
void ProjectState::valueTreeChildOrderChanged(juce::ValueTree&, int, int) { publish(); }

void ProjectState::publish() {
    Project project;
    std::deque<MidiClip> clips;
    std::vector<ClipLaunch> launches;

    for (const auto& trackTree : state_) {
        if (!trackTree.hasType(Id::track)) continue;
        const TrackId id{static_cast<std::uint64_t>(static_cast<juce::int64>(trackTree[Id::trackId]))};
        const auto kind = static_cast<TrackKind>(static_cast<int>(trackTree[Id::kind]));
        project.addTrack(kind, trackTree[Id::name].toString().toStdString(), id);
        project.setVolume(id, static_cast<float>(trackTree[Id::volume]));
        project.setPan(id, static_cast<float>(trackTree[Id::pan]));
        project.setMute(id, static_cast<bool>(trackTree[Id::mute]));
        project.setSolo(id, static_cast<bool>(trackTree[Id::solo]));

        for (const auto& clipTree : trackTree) {
            if (!clipTree.hasType(Id::clip)) continue;
            clips.emplace_back(id, static_cast<double>(clipTree[Id::loopBeats]));
            auto& clip = clips.back();
            for (const auto& noteTree : clipTree) {
                if (!noteTree.hasType(Id::note)) continue;
                MidiNote note;
                note.pitch = static_cast<std::uint8_t>(static_cast<int>(noteTree[Id::pitch]));
                note.velocity = static_cast<std::uint8_t>(static_cast<int>(noteTree[Id::velocity]));
                note.startBeat = static_cast<double>(noteTree[Id::start]);
                note.lengthBeats = static_cast<double>(noteTree[Id::length]);
                clip.addNote(note);
            }
            for (const auto& takeTree : clipTree) {
                if (!takeTree.hasType(Id::take)) continue;
                const int index = clip.addTake();
                for (const auto& noteTree : takeTree) {
                    if (!noteTree.hasType(Id::note)) continue;
                    MidiNote note;
                    note.pitch = static_cast<std::uint8_t>(static_cast<int>(noteTree[Id::pitch]));
                    note.velocity = static_cast<std::uint8_t>(static_cast<int>(noteTree[Id::velocity]));
                    note.startBeat = static_cast<double>(noteTree[Id::start]);
                    note.lengthBeats = static_cast<double>(noteTree[Id::length]);
                    clip.addNoteToTake(index, note);
                }
            }
            std::vector<CompRegion> regions;
            for (const auto& compTree : clipTree) {
                if (!compTree.hasType(Id::comp)) continue;
                regions.push_back(CompRegion{static_cast<int>(compTree[Id::take]), static_cast<double>(compTree[Id::start]),
                                             static_cast<double>(compTree[Id::length])});
            }
            clip.setComp(std::move(regions));
            if (static_cast<bool>(clipTree[Id::launched])) {
                launches.push_back(ClipLaunch{&clip, static_cast<double>(clipTree[Id::launchBeat])});
            }
        }
    }

    for (const auto& trackTree : state_) {
        if (!trackTree.hasType(Id::track)) continue;
        const TrackId id{static_cast<std::uint64_t>(static_cast<juce::int64>(trackTree[Id::trackId]))};
        const auto groupId = static_cast<std::uint64_t>(static_cast<juce::int64>(trackTree[Id::group]));
        if (groupId != 0) project.assignGroup(id, TrackId{groupId});
        const auto outputId = static_cast<std::uint64_t>(static_cast<juce::int64>(trackTree[Id::output]));
        if (outputId != 0) project.setOutput(id, TrackId{outputId});
        for (const auto& child : trackTree) {
            if (!child.hasType(Id::send)) continue;
            const auto dest = static_cast<std::uint64_t>(static_cast<juce::int64>(child[Id::destination]));
            project.addSend(id, TrackId{dest}, static_cast<float>(child[Id::level]));
        }
    }

    const auto command = static_cast<TransportCommand>(static_cast<int>(state_[Id::command]));
    const auto serial = static_cast<std::uint32_t>(static_cast<int>(state_[Id::commandSerial]));
    const auto playback = PlaybackSnapshot::fromLaunches(
        launches, command, serial, static_cast<double>(state_[Id::commandBeat]), static_cast<double>(state_[Id::bpm]));
    engine_.publish(project);
    engine_.publishPlayback(playback);
}

} // namespace dawn
