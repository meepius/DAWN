#pragma once

#include "engine/AudioEngine.h"
#include "juce/DeviceProcessor.h"

#include <cstdint>
#include <memory>
#include <vector>

namespace dawn {

// Message-thread owner. Removal does not free a plugin in the callback.
// The instance fades for three seconds, then drain() deletes it here.
class PluginRack {
public:
    void setEngine(AudioEngine* engine) noexcept { engine_ = engine; }
    void clear();
    void retire(std::uint64_t id);
    void add(std::uint64_t id, std::unique_ptr<DeviceProcessor> device);
    DeviceProcessor* find(std::uint64_t id) noexcept;
    void prepare(double sampleRate, int maxBlock);
    void drain();

private:
    struct Slot {
        std::uint64_t id = 0;
        std::unique_ptr<DeviceProcessor> device;
    };
    struct Tail;
    void handOff(std::unique_ptr<DeviceProcessor> device);

    std::vector<Slot> slots_;
    std::vector<Tail*> waiting_;
    AudioEngine* engine_ = nullptr;
    double sampleRate_ = 48000.0;
    int maxBlock_ = 512;
};

} // namespace dawn
