#include "app/DawnApplication.h"

#include "juce/AppPaths.h"
#include "juce/DawnMainComponent.h"
#include "juce/MidiBridge.h"
#include "juce/ProjectSerializer.h"

#include <vector>

namespace dawn {

struct DawnWindow : juce::DocumentWindow {
    DawnWindow() : juce::DocumentWindow("DAWN", juce::Colour(0xff1a1a1a), juce::DocumentWindow::allButtons) {
        setUsingNativeTitleBar(true);
    }

    void closeButtonPressed() override {
        if (auto* app = juce::JUCEApplication::getInstance()) app->systemRequestedQuit();
    }
};

class DawnApplication::Callback : public juce::AudioIODeviceCallback {
public:
    Callback(AudioEngine& engine, VisualizerDataQueue& visuals) : engine_(engine), visuals_(visuals) {}

    void audioDeviceAboutToStart(juce::AudioIODevice* device) override {
        const int channels = std::max(1, device->getActiveOutputChannels().countNumberOfSetBits());
        events_.resize(4096);
        midi_.ensureSize(48 * 1024);
        engine_.prepare(device->getCurrentSampleRate(), device->getCurrentBufferSizeSamples(), channels);
        engine_.setAudioRunning(true);
    }

    void audioDeviceStopped() override {
        engine_.setAudioRunning(false);
    }

    void audioDeviceIOCallbackWithContext(const float* const*,
                                          int,
                                          float* const* output,
                                          int numOutputChannels,
                                          int numSamples,
                                          const juce::AudioIODeviceCallbackContext&) override {
        MidiBlock block{events_.data(), static_cast<int>(events_.size()), 0};
        engine_.process(output, numOutputChannels, numSamples, nullptr, 0, &block);
        if (output != nullptr && numOutputChannels > 0 && output[0] != nullptr) {
            visuals_.push(output[0], numOutputChannels > 1 ? output[1] : output[0], numSamples);
        }
        copyToMidiBuffer(midi_, block);
    }

private:
    AudioEngine& engine_;
    VisualizerDataQueue& visuals_;
    std::vector<ScheduledEvent> events_;
    juce::MidiBuffer midi_;
};

DawnApplication::DawnApplication() = default;
DawnApplication::~DawnApplication() = default;

const juce::String DawnApplication::getApplicationName() { return "DAWN"; }
const juce::String DawnApplication::getApplicationVersion() { return "0.1.0"; }
bool DawnApplication::moreThanOneInstanceAllowed() { return false; }

void DawnApplication::initialise(const juce::String&) {
    AppPaths::setCurrent(AppPaths::resolve());
    profile_.loadFrom(AppPaths::current().inputMappings());
    look_ = std::make_unique<CyberdeckLookAndFeel>();
    juce::LookAndFeel::setDefaultLookAndFeel(look_.get());
    rack_.setEngine(&engine_);
    engine_.setLiveMidi(&injector_, &LiveMidiInjector::pop);
    mapper_ = std::make_unique<InputMapper>(injector_, profile_);
    pads_ = std::make_unique<GamepadPoller>(*mapper_);
    state_ = std::make_unique<ProjectState>(engine_);
    auto demo = AppPaths::current().root().getChildFile("demo.dawn");
    if (!demo.existsAsFile()) {
        demo = juce::File::getSpecialLocation(juce::File::currentExecutableFile)
                   .getParentDirectory()
                   .getChildFile("DAWN_DATA/demo.dawn");
    }
    juce::AudioPluginFormatManager formats;
    juce::String projectError;
    if (!demo.existsAsFile() || !ProjectSerializer{}.load(*state_, rack_, formats, demo, projectError)) {
        auto drums = state_->addTrack(TrackKind::Midi, "Drums");
        state_->addTrack(TrackKind::Midi, "Bass");
        state_->addTrack(TrackKind::Return, "Reverb");
        auto clip = state_->addClip(drums, 4.0);
        state_->addNote(clip, 36, 0.0, 0.25, 110);
        state_->setLaunched(clip, true, 0.0);
    }

    callback_ = std::make_unique<Callback>(engine_, visuals_);
    const auto preferences = AppPaths::current().preferences();
    std::unique_ptr<juce::XmlElement> savedAudio(juce::XmlDocument::parse(preferences));
    const auto audioError = devices_.initialise(0, 2, savedAudio.get(), true);
    juce::ignoreUnused(audioError);
    devices_.addAudioCallback(callback_.get());

    window_ = std::make_unique<DawnWindow>();
    auto* shell = new DawnMainComponent(*state_, engine_, visuals_, injector_);
    if (mapper_ != nullptr) shell->addKeyListener(mapper_.get());
    window_->setContentOwned(shell, true);
    window_->centreWithSize(1280, 800);
    window_->setVisible(true);
}

void DawnApplication::shutdown() {
    if (auto xml = devices_.createStateXml()) xml->writeTo(AppPaths::current().preferences());
    profile_.saveTo(AppPaths::current().inputMappings());
    pads_.reset();
    if (callback_ != nullptr) devices_.removeAudioCallback(callback_.get());
    engine_.setAudioRunning(false);
    rack_.clear();
    rack_.drain();
    engine_.reclaim();
    window_ = nullptr;
    juce::LookAndFeel::setDefaultLookAndFeel(nullptr);
}

void DawnApplication::systemRequestedQuit() { quit(); }

} // namespace dawn
