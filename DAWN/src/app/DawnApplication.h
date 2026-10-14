#pragma once

// Desktop and Android share this class. JUCE selects ASIO, Core Audio,
// JACK, ALSA, or Oboe from the same AudioDeviceManager.

#include "engine/AudioEngine.h"
#include "juce/CyberdeckLookAndFeel.h"
#include "juce/GamepadPoller.h"
#include "juce/InputMappingComponent.h"
#include "juce/LiveMidiInjector.h"
#include "juce/PluginRack.h"
#include "juce/ProjectState.h"
#include "juce/VisualizerDataQueue.h"

#include <juce_audio_utils/juce_audio_utils.h>
#include <memory>

namespace dawn {

class DawnApplication : public juce::JUCEApplication {
public:
    DawnApplication();
    ~DawnApplication() override;

    const juce::String getApplicationName() override;
    const juce::String getApplicationVersion() override;
    bool moreThanOneInstanceAllowed() override;
    void initialise(const juce::String& commandLine) override;
    void shutdown() override;
    void systemRequestedQuit() override;

    AudioEngine& engine() noexcept { return engine_; }

private:
    class Callback;

    AudioEngine engine_;
    PluginRack rack_;
    VisualizerDataQueue visuals_;
    std::unique_ptr<CyberdeckLookAndFeel> look_;
    LiveMidiInjector injector_;
    InputProfile profile_;
    std::unique_ptr<InputMapper> mapper_;
    std::unique_ptr<GamepadPoller> pads_;
    std::unique_ptr<ProjectState> state_;
    juce::AudioDeviceManager devices_;
    std::unique_ptr<Callback> callback_;
    std::unique_ptr<juce::DocumentWindow> window_;
};

} // namespace dawn
