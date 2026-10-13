#include "juce/GamepadPoller.h"

#if defined(__linux__)
#include <fcntl.h>
#include <linux/joystick.h>
#include <unistd.h>
#endif

namespace dawn {

GamepadPoller::GamepadPoller(InputMapper& mapper) : mapper_(mapper) { startTimerHz(60); }

GamepadPoller::~GamepadPoller() {
    stopTimer();
#if defined(__linux__)
    if (device_ >= 0) ::close(device_);
#endif
}

void GamepadPoller::timerCallback() {
    held_.connected = read(held_);
    mapper_.pollGamepad(held_);
}

bool GamepadPoller::read(GamepadFrame& frame) {
#if defined(__linux__)
    if (device_ < 0) device_ = ::open("/dev/input/js0", O_RDONLY | O_NONBLOCK);
    if (device_ < 0) return false;
    js_event event{};
    while (::read(device_, &event, sizeof(event)) == static_cast<ssize_t>(sizeof(event))) {
        const auto type = event.type & ~JS_EVENT_INIT;
        if (type == JS_EVENT_BUTTON && event.number < 16) frame.button[event.number] = event.value != 0;
        if (type == JS_EVENT_AXIS && event.number < 6) frame.axis[event.number] = static_cast<float>(event.value) / 32767.f;
    }
    return true;
#else
    juce::ignoreUnused(frame);
    return false;
#endif
}

} // namespace dawn
