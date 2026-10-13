#pragma once

#include "juce/InputMapper.h"

namespace dawn {

// Polls at 60 Hz on the message thread. The translation step drops unchanged axes.
class GamepadPoller : private juce::Timer {
public:
    explicit GamepadPoller(InputMapper& mapper);
    ~GamepadPoller() override;

private:
    void timerCallback() override;
    bool read(GamepadFrame& frame);

    InputMapper& mapper_;
    GamepadFrame held_{};
    int device_ = -1;
};

} // namespace dawn
