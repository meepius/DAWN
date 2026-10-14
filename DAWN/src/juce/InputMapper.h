#pragma once

#include "engine/GamepadMap.h"
#include "juce/InputProfile.h"
#include "juce/LiveMidiInjector.h"

#include <juce_gui_basics/juce_gui_basics.h>
#include <functional>

namespace dawn {

struct InputCapture {
    juce::String source;
    int code = 0;
};

class InputMapper : public juce::KeyListener {
public:
    InputMapper(LiveMidiInjector& injector, InputProfile& profile);

    void armLearn(std::function<void(InputCapture)> learned);
    void cancelLearn();
    bool isLearning() const noexcept { return static_cast<bool>(learned_); }
    void pollGamepad(const GamepadFrame& frame);

    bool keyPressed(const juce::KeyPress& key, juce::Component* origin) override;
    bool keyStateChanged(bool isKeyDown, juce::Component* originatingComponent) override;

private:
    int noteFor(const juce::KeyPress& key) const;
    void pushNote(int note, bool down, int channel);

    LiveMidiInjector& injector_;
    InputProfile& profile_;
    std::function<void(InputCapture)> learned_;
    GamepadFrame previous_{};
    int lastCc_[6]{-1, -1, -1, -1, -1, -1};
    int octave_ = 4;
    bool keyDown_[128]{};
    int keyNote_[128]{};
};

} // namespace dawn
