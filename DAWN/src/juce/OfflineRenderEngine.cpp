#include "juce/OfflineRenderEngine.h"

#include "engine/OfflineRender.h"

namespace dawn {
namespace {

int writerBits(int bitDepth) { return bitDepth >= 32 ? 32 : 24; }

} // namespace

OfflineRenderEngine::OfflineRenderEngine() : juce::Thread("dawn-bounce") {}

OfflineRenderEngine::~OfflineRenderEngine() { stopThread(8000); }

void OfflineRenderEngine::start(Project project, PlaybackSnapshot playback, BounceSettings settings) {
    if (isThreadRunning()) return;
    project_ = std::move(project);
    playback_ = std::move(playback);
    settings_ = std::move(settings);
    progress_.store(0.f);
    cancel_.store(false);
    finished_.store(false);
    ok_.store(false);
    startThread();
}

void OfflineRenderEngine::run() {
    auto stream = std::make_unique<juce::FileOutputStream>(settings_.file);
    if (stream == nullptr || stream->failedToOpen()) {
        finished_.store(true);
        return;
    }
    stream->setPosition(0);
    stream->truncate();
    juce::WavAudioFormat format;
    juce::StringPairArray meta;
    if (settings_.bitDepth >= 32) meta.set("floating-point", "1");
    auto* created = format.createWriterFor(stream.get(), settings_.sampleRate, 2, writerBits(settings_.bitDepth), meta, 0);
    if (created == nullptr) {
        finished_.store(true);
        return;
    }
    stream.release();
    std::unique_ptr<juce::AudioFormatWriter> writer(created);

    AudioEngine engine;
    engine.prepare(settings_.sampleRate, 512, 2);
    engine.publish(project_);
    engine.publishPlayback(playback_);
    const auto write = [](void* user, const float* const* channels, int numChannels, int samples) {
        static_cast<juce::AudioFormatWriter*>(user)->writeFromFloatArrays(channels, numChannels, samples);
    };
    const bool wrote = renderOffline(engine, 512, 2, settings_.startSample, settings_.endSample, {}, nullptr, 0,
                                     OfflineWriter{writer.get(), write}, &progress_, &cancel_);
    writer.reset();
    if (!wrote || cancel_.load()) settings_.file.deleteFile();
    ok_.store(wrote);
    finished_.store(true);
}

} // namespace dawn
