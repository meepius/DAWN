#include "juce/InputMapper.h"

namespace dawn {
namespace {

int whiteNote(int octave, int step) {
    static constexpr int semitone[] = {0, 2, 4, 5, 7, 9, 11, 12};
    return juce::jlimit(0, 127, 12 * octave + semitone[step]);
}

} // namespace

InputMapper::InputMapper(LiveMidiInjector& injector, InputProfile& profile) : injector_(injector), profile_(profile) {}

void InputMapper::armLearn(std::function<void(InputCapture)> learned) { learned_ = std::move(learned); }

void InputMapper::cancelLearn() { learned_ = nullptr; }

bool InputMapper::keyPressed(const juce::KeyPress& key, juce::Component*) {
    if (learned_) {
        auto callback = std::move(learned_);
        callback(InputCapture{"key", key.getKeyCode()});
        return true;
    }
    if (key.getKeyCode() == 'Z') {
        octave_ = juce::jlimit(0, 8, octave_ - 1);
        return true;
    }
    if (key.getKeyCode() == 'X') {
        octave_ = juce::jlimit(0, 8, octave_ + 1);
        return true;
    }
    const int note = noteFor(key);
    if (note < 0) return false;
    const int slot = key.getKeyCode() & 127;
    keyDown_[slot] = true;
    keyNote_[slot] = note;
    pushNote(note, true, 1);
    return true;
}

bool InputMapper::keyStateChanged(bool isKeyDown, juce::Component* originatingComponent) {
    juce::ignoreUnused(isKeyDown, originatingComponent);
    for (int slot = 0; slot < 128; ++slot) {
        if (!keyDown_[slot]) continue;
        if (juce::KeyPress::isKeyCurrentlyDown(slot)) continue;
        keyDown_[slot] = false;
        pushNote(keyNote_[slot], false, 1);
    }
    return false;
}

void InputMapper::pollGamepad(const GamepadFrame& frame) {
    if (learned_ && frame.connected) {
        for (int button = 0; button < 16; ++button) {
            const bool edge = frame.button[button] && !(previous_.connected && previous_.button[button]);
            if (!edge) continue;
            auto callback = std::move(learned_);
            previous_ = frame;
            callback(InputCapture{"button", button});
            return;
        }
    }
    auto bindings = profile_.gamepadBindings();
    ScheduledEvent events[16]{};
    const int count = translateGamepad(previous_, frame, bindings.data(), static_cast<int>(bindings.size()), lastCc_, events, 16);
    previous_ = frame;
    for (int i = 0; i < count; ++i) {
        const auto& event = events[i];
        injector_.push(juce::MidiMessage(event.status, event.data1, event.data2));
    }
}

int InputMapper::noteFor(const juce::KeyPress& key) const {
    switch (key.getKeyCode()) {
    case 'A': return whiteNote(octave_, 0);
    case 'W': return whiteNote(octave_, 0) + 1;
    case 'S': return whiteNote(octave_, 1);
    case 'E': return whiteNote(octave_, 1) + 1;
    case 'D': return whiteNote(octave_, 2);
    case 'F': return whiteNote(octave_, 3);
    case 'T': return whiteNote(octave_, 3) + 1;
    case 'G': return whiteNote(octave_, 4);
    case 'Y': return whiteNote(octave_, 4) + 1;
    case 'H': return whiteNote(octave_, 5);
    case 'U': return whiteNote(octave_, 5) + 1;
    case 'J': return whiteNote(octave_, 6);
    case 'K': return whiteNote(octave_, 7);
    default: return -1;
    }
}

void InputMapper::pushNote(int note, bool down, int channel) {
    const int status = (down ? 0x90 : 0x80) | ((channel - 1) & 0x0F);
    injector_.push(juce::MidiMessage(status, note, down ? 100 : 0));
}

} // namespace dawn
