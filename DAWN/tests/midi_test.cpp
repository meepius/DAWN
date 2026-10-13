#include "engine/AnalogSynth.h"
#include "engine/AudioEngine.h"
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

dawn::ScheduledEvent findStatus(const dawn::MidiBlock& block, int status) {
    for (int i = 0; i < block.count; ++i) {
        if (block.events[i].status == status) return block.events[i];
    }
    return {};
}

} // namespace

int runMidiTests() {
    using namespace dawn;
    AudioEngine engine;
    engine.prepare(48000.0, 1024, 1);
    Project project;
    const auto bass = project.addTrack(TrackKind::Midi, "Bass");
    engine.publish(project);

    MidiClip clip(bass, 4.0);
    clip.addNote(MidiNote{0.0, 1.0, 60, 100, 0});
    const ClipLaunch launch{&clip, 0.0};
    const auto playing = PlaybackSnapshot::fromLaunches(std::span<const ClipLaunch>(&launch, 1),
                                                        TransportCommand::Play,
                                                        1,
                                                        0.0,
                                                        120.0);
    engine.publishPlayback(playing);

    std::vector<ScheduledEvent> storage(64);
    MidiBlock block{storage.data(), static_cast<int>(storage.size()), 0};
    std::vector<float> audio(512, 0.f);
    float* channels[] = {audio.data()};
    engine.process(channels, 1, 512, nullptr, 0, &block);

    const auto on = findStatus(block, 0x90);
    expect(on.status == 0x90 && on.sampleOffset == 0 && on.data1 == 60 && on.data2 == 100, "note on at sample 0");

    block.count = 0;
    engine.process(channels, 1, 512, nullptr, 0, &block);
    expect(findStatus(block, 0x90).status == 0, "note does not retrigger inside the loop");

    AudioEngine timed;
    timed.prepare(48000.0, 1024, 1);
    timed.publish(project);
    const double beat = 240.0 * 120.0 / (48000.0 * 60.0);
    MidiClip offsetClip(bass, 4.0);
    offsetClip.addNote(MidiNote{beat, 128.0 / 24000.0, 64, 90, 0});
    const ClipLaunch offsetLaunch{&offsetClip, 0.0};
    timed.publishPlayback(PlaybackSnapshot::fromLaunches(std::span<const ClipLaunch>(&offsetLaunch, 1),
                                                         TransportCommand::Play,
                                                         1,
                                                         0.0,
                                                         120.0));
    block.count = 0;
    timed.process(channels, 1, 512, nullptr, 0, &block);
    const auto shifted = findStatus(block, 0x90);
    const auto off = findStatus(block, 0x80);
    expect(shifted.sampleOffset == 240 && shifted.data1 == 64, "note on is sample accurate");
    expect(off.sampleOffset == 368 && off.data1 == 64, "note off is sample accurate");

    AudioEngine looped;
    looped.prepare(48000.0, 1024, 1);
    looped.publish(project);
    MidiClip looping(bass, 1.0);
    looping.addNote(MidiNote{0.0, 0.25, 62, 80, 0});
    const ClipLaunch loopLaunch{&looping, 0.0};
    const double nearWrap = 1.0 - (256.0 / 24000.0);
    looped.publishPlayback(PlaybackSnapshot::fromLaunches(std::span<const ClipLaunch>(&loopLaunch, 1),
                                                          TransportCommand::Play,
                                                          1,
                                                          nearWrap,
                                                          120.0));
    block.count = 0;
    looped.process(channels, 1, 512, nullptr, 0, &block);
    const auto wrapped = findStatus(block, 0x90);
    expect(wrapped.sampleOffset == 256 && wrapped.data1 == 62, "looped note fires on the wrap");

    AnalogSynth synth;
    synth.prepare(48000.0, 4);
    std::vector<float> voice(128, 0.f);
    synth.process(voice.data(), 128);
    expect(std::fabs(voice[64]) < 0.0001f, "synth is silent until a note");
    synth.noteOn(69, 100);
    std::fill(voice.begin(), voice.end(), 0.f);
    synth.process(voice.data(), 128);
    float peak = 0.f;
    for (float sample : voice) peak = std::max(peak, std::fabs(sample));
    expect(peak > 0.01f, "analog voice produces audio");

    if (failures != 0) std::cerr << failures << " midi checks failed\n";
    return failures;
}
