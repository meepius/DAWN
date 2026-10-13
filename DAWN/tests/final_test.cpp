#include "engine/OfflineRender.h"
#include "model/Paths.h"
#include "model/Project.h"

#include <atomic>
#include <cmath>
#include <iostream>
#include <vector>

namespace {

int failures = 0;

void expect(bool condition, const char* message) {
    if (condition) return;
    std::cerr << "FAIL " << message << '\n';
    ++failures;
}

struct Capture {
    std::vector<float> samples;
    static void write(void* user, const float* const* channels, int, int count) {
        auto* self = static_cast<Capture*>(user);
        self->samples.insert(self->samples.end(), channels[0], channels[0] + count);
    }
};

} // namespace

int runFinalTests() {
    using namespace dawn;
    expect(toRelativePath("/songs/demo", "/songs/demo/audio/kick.wav") == "audio/kick.wav", "audio inside the project is stored relative");
    expect(fromRelativePath("/songs/demo", "audio/kick.wav") == "/songs/demo/audio/kick.wav", "relative audio resolves beside the project");
    expect(toRelativePath("/songs/demo", "/other/kick.wav") == "/other/kick.wav", "audio outside the project stays absolute");

    Project project;
    const auto bass = project.addTrack(TrackKind::Audio, "Bass");
    project.setVolume(bass, 0.5f);
    AudioEngine engine;
    engine.prepare(48000.0, 64, 1);
    engine.publish(project);
    std::vector<float> input(64, 1.f);
    Capture captured;
    std::atomic<float> progress{0.f};
    const bool ok = renderOffline(engine, 32, 1, 0, 64, bass, input.data(), 64, OfflineWriter{&captured, &Capture::write}, &progress, nullptr);
    expect(ok, "offline render finishes");
    expect(captured.samples.size() == 64, "bounce length matches the range");
    expect(!captured.samples.empty() && std::fabs(captured.samples[0] - 0.5f) < 0.0001f, "bounce uses the live mixer fader");
    expect(progress.load() == 1.f, "progress reaches the end");

    std::atomic<bool> cancel{true};
    Capture skipped;
    const bool stopped = renderOffline(engine, 32, 1, 0, 64, bass, input.data(), 64, OfflineWriter{&skipped, &Capture::write}, nullptr, &cancel);
    expect(!stopped && skipped.samples.empty(), "cancel stops the bounce before writing");

    if (failures != 0) std::cerr << failures << " final checks failed\n";
    return failures;
}
