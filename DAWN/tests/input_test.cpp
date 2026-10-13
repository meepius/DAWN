#include "catalog/PluginCatalog.h"
#include "engine/AudioEngine.h"
#include "engine/GamepadMap.h"
#include "engine/LiveMidiQueue.h"

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

void popQueue(void* user, dawn::MidiBlock& block) noexcept {
    static_cast<dawn::LiveMidiQueue*>(user)->popInto(block);
}

} // namespace

int runInputTests() {
    using namespace dawn;

    LiveMidiQueue queue;
    int accepted = 0;
    for (int i = 0; i < 400; ++i) {
        if (queue.push(ScheduledEvent{0, 0x90, 60, 100})) ++accepted;
    }
    expect(accepted == LiveMidiQueue::capacity - 1, "a full live queue drops new events");

    AudioEngine engine;
    engine.prepare(48000.0, 128, 1);
    engine.setLiveMidi(&queue, &popQueue);
    std::vector<ScheduledEvent> storage(16);
    MidiBlock block{storage.data(), 16, 0};
    std::vector<float> audio(32, 0.f);
    float* channels[] = {audio.data()};
    engine.process(channels, 1, 32, nullptr, 0, &block);
    expect(block.count == 16, "process merges live notes until the block is full");
    expect(block.events[0].status == 0x90 && block.events[0].data1 == 60, "merged note is a note on");

    GamepadFrame idle;
    idle.connected = true;
    GamepadFrame pressed = idle;
    pressed.button[0] = true;
    pressed.axis[5] = 0.8f;
    GamepadBinding bindings[] = {
        {0, false, 0, 48, false},
        {5, true, 0, 1, true},
    };
    int lastCc[6]{-1, -1, -1, -1, -1, -1};
    ScheduledEvent events[8]{};
    const int first = translateGamepad(idle, pressed, bindings, 2, lastCc, events, 8);
    expect(first == 2, "press and trigger each emit once");
    const int repeat = translateGamepad(pressed, pressed, bindings, 2, lastCc, events, 8);
    expect(repeat == 0, "a resting pad does not refill the queue");
    GamepadFrame released = pressed;
    released.button[0] = false;
    const int up = translateGamepad(pressed, released, bindings, 2, lastCc, events, 8);
    expect(up == 1 && events[0].status == 0x80 && events[0].data1 == 48, "button release sends note off");

    const char* manifest = R"({
      "packages": [
        {"id":"surge-xt","name":"Surge XT","format":"vst3","version":"1.3.4","url":"https://example.com/surge.zip","os":"linux","sha256":"abc"},
        {"id":"chow","name":"ChowDSP","format":"clap","version":"1.0.0","url":"https://example.com/chow.zip","os":"windows","sha256":""}
      ]
    })";
    const auto packages = parsePluginCatalog(manifest);
    expect(packages.size() == 2, "catalog parses both packages");
    expect(packages[0].id == "surge-xt" && packages[0].format == PackageFormat::Vst3, "first package is Surge XT");
    expect(packages[1].format == PackageFormat::Clap && packages[1].os == "windows", "second package is CLAP");

    if (failures != 0) std::cerr << failures << " input checks failed\n";
    return failures;
}
