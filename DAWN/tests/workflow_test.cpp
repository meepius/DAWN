#include "engine/AudioEngine.h"
#include "engine/PlaybackSnapshot.h"
#include "model/MidiClip.h"

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

void tail(void*, float* const* output, int, int samples, float gain) noexcept {
    for (int i = 0; i < samples; ++i) output[0][i] += gain;
}

} // namespace

int runWorkflowTests() {
    using namespace dawn;
    MidiClip clip(TrackId{1}, 4.0);
    clip.addNote(MidiNote{0.0, 1.0, 60, 100, 0});
    clip.addTake();
    clip.addNoteToTake(1, MidiNote{0.0, 1.0, 67, 110, 0});
    clip.setComp({CompRegion{1, 0.0, 2.0}});
    const ClipLaunch launch{&clip, 0.0};
    const auto snapshot = PlaybackSnapshot::fromLaunches(std::span<const ClipLaunch>(&launch, 1), TransportCommand::None, 0, 0.0, 120.0);
    expect(snapshot.notes.size() == 1 && snapshot.notes[0].pitch == 67, "the snapshot hears only the comped take");
    expect(clip.notes().size() == 1 && clip.notes()[0].pitch == 60, "the muted lane stays on the message thread");

    AudioEngine engine;
    engine.prepare(48000.0, 64, 1);
    int marker = 0;
    expect(engine.adoptTail(&marker, &tail, 100), "a tail slot is claimed without allocating");
    std::vector<float> audio(50, 0.f);
    float* channels[] = {audio.data()};
    engine.process(channels, 1, 50, nullptr, 0);
    expect(std::fabs(audio[0] - 1.f) < 0.001f, "a new tail starts at full level");
    std::fill(audio.begin(), audio.end(), 0.f);
    engine.process(channels, 1, 50, nullptr, 0);
    expect(std::fabs(audio[0] - 0.5f) < 0.001f, "the tail is halfway down after half of the fade");
    expect(engine.reclaimTail() == &marker, "the message thread deletes the tail only after it is silent");

    if (failures != 0) std::cerr << failures << " workflow checks failed\n";
    return failures;
}
