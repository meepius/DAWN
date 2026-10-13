#pragma once

#include <cstdint>

namespace dawn {

enum class PluginFormat : std::uint8_t { Internal, Vst3, Clap, AudioUnit };

struct DeviceProcessContext {
    float* const* audio = nullptr;
    const float* sidechain = nullptr;
    int channels = 0;
    int samples = 0;
    double sampleRate = 48000.0;
    double tempo = 120.0;
    double beat = 0.0;
    bool playing = false;
};

// Lock-free device callback used by the mixer. It does not reference JUCE.
// The desktop and Android hosts implement dawn::DeviceProcessor in
// juce/DeviceProcessor.h and adapt into this type before the snapshot is published.
class RealtimeProcessor {
public:
    virtual ~RealtimeProcessor() = default;
    virtual void prepare(double sampleRate, int maxBlock) = 0;
    virtual void process(DeviceProcessContext& context) noexcept = 0;
    virtual void releaseResources() = 0;
};

} // namespace dawn
