#include "engine/AudioEngine.h"
#include "model/Project.h"

#include <cmath>
#include <cstdlib>
#include <iostream>
#include <string>
#include <vector>

namespace {

int failures = 0;

void expect(bool condition, const char* message) {
    if (condition) return;
    std::cerr << "FAIL " << message << '\n';
    ++failures;
}

void expectNear(float actual, float wanted, const char* message) {
    if (std::fabs(actual - wanted) < 0.0001f) return;
    std::cerr << "FAIL " << message << " got " << actual << " wanted " << wanted << '\n';
    ++failures;
}

float render(dawn::AudioEngine& engine, const dawn::RenderInput& input, int samples = 8) {
    std::vector<float> left(static_cast<std::size_t>(samples), 0.f);
    float* channels[] = {left.data()};
    engine.process(channels, 1, samples, &input, 1);
    return left[0];
}

} // namespace

int runMidiTests();
int runInputTests();
int runFinalTests();
int runWorkflowTests();
int runPortableTests();

int main() {
    using namespace dawn;

    Project project;
    const auto drums = project.addTrack(TrackKind::Midi, "Drums");
    const auto bass = project.addTrack(TrackKind::Midi, "Bass");
    const auto verb = project.addTrack(TrackKind::Return, "Reverb");
    expect(project.topLevel().size() == 3, "three top-level tracks");
    expect(project.reorder(bass, 0), "reorder bass");
    expect(project.topLevel().front() == bass, "bass is first");

    expect(project.setVolume(drums, 0.5f), "volume");
    AudioEngine engine;
    engine.prepare(48000.0, 64, 1);
    engine.publish(project);
    std::vector<float> hit(8, 1.f);
    expectNear(render(engine, RenderInput{drums, hit.data()}), 0.5f, "fader scales the track");

    expect(project.setMute(drums, true), "mute");
    engine.publish(project);
    engine.reclaim();
    expectNear(render(engine, RenderInput{drums, hit.data()}), 0.f, "mute silences the track");
    project.setMute(drums, false);
    project.setVolume(drums, 1.f);

    expect(project.setSolo(bass, true), "solo bass");
    engine.publish(project);
    expectNear(render(engine, RenderInput{drums, hit.data()}), 0.f, "unsoloed track is silent");
    expectNear(render(engine, RenderInput{bass, hit.data()}), 0.8f, "soloed track uses its fader");
    project.setSolo(bass, false);

    expect(project.addSend(drums, verb, 0.5f), "send to reverb");
    project.setVolume(drums, 1.f);
    project.setVolume(verb, 1.f);
    engine.publish(project);
    expectNear(render(engine, RenderInput{drums, hit.data()}), 1.5f, "dry plus post-fader send");

    expect(!project.addSend(verb, drums, 1.f), "cannot send into a music track");
    expect(!project.setOutput(drums, verb), "output target must be a group");
    expect(!project.setOutput(drums, drums), "no self output");

    Project grouped;
    const auto a = grouped.addTrack(TrackKind::Audio, "A");
    const auto b = grouped.addTrack(TrackKind::Audio, "B");
    grouped.setVolume(a, 1.f);
    grouped.setVolume(b, 1.f);
    const TrackId members[] = {a, b};
    const auto kit = grouped.groupTracks(members, "Kit");
    expect(kit.valid(), "group created");
    expect(grouped.topLevel().size() == 1, "children leave the top level");
    expect(grouped.find(a)->output == kit, "child outputs to the group");
    grouped.setVolume(kit, 0.5f);
    engine.publish(grouped);
    expectNear(render(engine, RenderInput{a, hit.data()}), 0.5f, "group fader after the child");

    const auto inside = grouped.find(kit)->children;
    expect(!grouped.setOutput(kit, a), "group cannot route back into its child");
    expect(grouped.removeTrack(kit), "ungroup on delete");
    expect(grouped.find(a) != nullptr && !grouped.find(a)->group.valid(), "child survives the group");
    expect(inside.size() == 2, "group had both children");

    const auto device = project.addDevice(drums, DeviceKind::Compressor);
    expect(device != 0, "device id");
    expect(project.setSidechain(device, bass), "sidechain source");

    failures += runMidiTests();
    failures += runInputTests();
    failures += runFinalTests();
    failures += runWorkflowTests();
    failures += runPortableTests();

    if (failures != 0) {
        std::cerr << failures << " failed\n";
        return EXIT_FAILURE;
    }
    std::cout << "dawn core ok\n";
    return EXIT_SUCCESS;
}
