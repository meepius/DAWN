#pragma once

#include <cstdint>

namespace dawn {

// Fixed voice pool. prepare() allocates. noteOn, noteOff, and process do not.
class AnalogSynth {
public:
    void prepare(double sampleRate, int voices = 8);
    void noteOn(int pitch, int velocity) noexcept;
    void noteOff(int pitch) noexcept;
    void process(float* mono, int samples) noexcept;

private:
    struct Voice {
        bool active = false;
        bool releasing = false;
        int pitch = 60;
        double phase = 0.0;
        double frequency = 440.0;
        float level = 0.f;
        float envelope = 0.f;
    };

    double sampleRate_ = 48000.0;
    int voiceCount_ = 0;
    Voice voices_[16]{};
};

} // namespace dawn
