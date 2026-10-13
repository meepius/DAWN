#pragma once

#include "engine/AudioEngine.h"

#include <juce_data_structures/juce_data_structures.h>

namespace dawn {

namespace Id {
inline const juce::Identifier project{"project"};
inline const juce::Identifier track{"track"};
inline const juce::Identifier clip{"clip"};
inline const juce::Identifier note{"note"};
inline const juce::Identifier trackId{"trackId"};
inline const juce::Identifier name{"name"};
inline const juce::Identifier kind{"kind"};
inline const juce::Identifier volume{"volume"};
inline const juce::Identifier pan{"pan"};
inline const juce::Identifier mute{"mute"};
inline const juce::Identifier solo{"solo"};
inline const juce::Identifier loopBeats{"loopBeats"};
inline const juce::Identifier launchBeat{"launchBeat"};
inline const juce::Identifier launched{"launched"};
inline const juce::Identifier pitch{"pitch"};
inline const juce::Identifier start{"start"};
inline const juce::Identifier length{"length"};
inline const juce::Identifier velocity{"velocity"};
inline const juce::Identifier bpm{"bpm"};
inline const juce::Identifier nextId{"nextId"};
inline const juce::Identifier command{"command"};
inline const juce::Identifier commandSerial{"commandSerial"};
inline const juce::Identifier commandBeat{"commandBeat"};
inline const juce::Identifier group{"group"};
inline const juce::Identifier output{"output"};
inline const juce::Identifier audio{"audio"};
inline const juce::Identifier send{"send"};
inline const juce::Identifier destination{"destination"};
inline const juce::Identifier level{"level"};
inline const juce::Identifier device{"device"};
inline const juce::Identifier deviceId{"deviceId"};
inline const juce::Identifier identifier{"identifier"};
inline const juce::Identifier pluginState{"pluginState"};
inline const juce::Identifier formatName{"formatName"};
inline const juce::Identifier userNotes{"userNotes"};
inline const juce::Identifier take{"take"};
inline const juce::Identifier comp{"comp"};
} // namespace Id

// Desktop and Android both bind widgets to this tree.
// A property change publishes a new snapshot. The audio thread never sees the tree.
class ProjectState : private juce::ValueTree::Listener {
public:
    explicit ProjectState(AudioEngine& engine);
    ~ProjectState() override;

    juce::ValueTree& state() noexcept { return state_; }
    const juce::ValueTree& state() const noexcept { return state_; }
    juce::UndoManager& undo() noexcept { return undo_; }

    juce::ValueTree addTrack(TrackKind kind, const juce::String& name);
    void setVolume(const juce::ValueTree& track, float volume);
    juce::ValueTree addClip(const juce::ValueTree& track, double loopBeats);
    void addNote(const juce::ValueTree& clip, int pitch, double start, double length, int velocity);
    void setLaunched(const juce::ValueTree& clip, bool launched, double launchBeat);
    void play(double fromBeat);
    void stop();
    void replace(juce::ValueTree next);

private:
    void valueTreePropertyChanged(juce::ValueTree&, const juce::Identifier&) override;
    void valueTreeChildAdded(juce::ValueTree&, juce::ValueTree&) override;
    void valueTreeChildRemoved(juce::ValueTree&, juce::ValueTree&, int) override;
    void valueTreeChildOrderChanged(juce::ValueTree&, int, int) override;
    void publish();
    TrackId takeId();

    AudioEngine& engine_;
    juce::ValueTree state_{Id::project};
    juce::UndoManager undo_;
};

} // namespace dawn
