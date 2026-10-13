#include "engine/GamepadMap.h"

#include <cmath>

namespace dawn {
namespace {

int axisToCc(float value) noexcept {
    const float shaped = value >= 0.f ? value : 0.f;
    const float unit = shaped > 1.f ? 1.f : shaped;
    return static_cast<int>(std::lround(unit * 127.f));
}

int stickToCc(float value) noexcept {
    const float clamped = value < -1.f ? -1.f : (value > 1.f ? 1.f : value);
    return static_cast<int>(std::lround((clamped * 0.5f + 0.5f) * 127.f));
}

} // namespace

int translateGamepad(const GamepadFrame& previous,
                     const GamepadFrame& current,
                     const GamepadBinding* bindings,
                     int bindingCount,
                     int* lastCc,
                     ScheduledEvent* out,
                     int outCapacity) noexcept {
    if (!current.connected || bindings == nullptr || out == nullptr || outCapacity <= 0) return 0;
    int count = 0;
    auto push = [&](std::uint8_t status, std::uint8_t data1, std::uint8_t data2) {
        if (count >= outCapacity) return;
        out[count++] = ScheduledEvent{0, status, data1, data2};
    };
    for (int i = 0; i < bindingCount; ++i) {
        const auto& binding = bindings[i];
        const auto channel = static_cast<std::uint8_t>(binding.channel & 0x0F);
        if (!binding.axis) {
            if (binding.control < 0 || binding.control >= 16 || binding.cc) continue;
            const bool was = previous.connected && previous.button[binding.control];
            const bool now = current.button[binding.control];
            if (was == now) continue;
            const auto note = static_cast<std::uint8_t>(binding.number & 0x7F);
            push(static_cast<std::uint8_t>((now ? 0x90 : 0x80) | channel), note, now ? 100 : 0);
            continue;
        }
        if (binding.control < 0 || binding.control >= 6 || lastCc == nullptr || !binding.cc) continue;
        const float delta = std::fabs(current.axis[binding.control] - (previous.connected ? previous.axis[binding.control] : 0.f));
        if (previous.connected && delta < 0.02f) continue;
        const bool trigger = binding.control >= 4;
        const int cc = trigger ? axisToCc(current.axis[binding.control]) : stickToCc(current.axis[binding.control]);
        if (previous.connected && cc == lastCc[binding.control]) continue;
        lastCc[binding.control] = cc;
        push(static_cast<std::uint8_t>(0xB0 | channel), static_cast<std::uint8_t>(binding.number & 0x7F), static_cast<std::uint8_t>(cc));
    }
    return count;
}

} // namespace dawn
