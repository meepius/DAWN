#pragma once

#include "juce/LiveMidiInjector.h"
#include "juce/ProjectState.h"
#include "juce/VisualizerDataQueue.h"

#include <juce_gui_extra/juce_gui_extra.h>
#include <memory>
#include <optional>

namespace dawn {

// The native widget grid is gone. This window is one JUCE 8 web view.
// HTML sends events and slider relays. Those callbacks write ProjectState or
// the lock-free live MIDI injector. The audio thread never sees the browser.
class DawnMainComponent : public juce::Component,
                          private juce::Timer,
                          private juce::WebSliderRelay::Listener {
public:
    DawnMainComponent(ProjectState& state, AudioEngine& engine, VisualizerDataQueue& visuals, LiveMidiInjector& live);
    ~DawnMainComponent() override;

    void resized() override;

private:
    static constexpr int kColumns = 6;
    static constexpr int kScenes = 4;

    void timerCallback() override;
    void sliderValueChanged(juce::WebSliderRelay* relay) override;
    void initialUpdateRequested(juce::WebSliderRelay* relay) override;

    juce::WebBrowserComponent::Options browserOptions();
    std::optional<juce::WebBrowserComponent::Resource> resourceFor(const juce::String& path) const;
    juce::File webRoot() const;
    juce::String sessionJson() const;
    void launchClip(int track, int scene);
    void setTrackFlag(int track, const juce::String& flag, bool value);
    double quantized(double beat) const;

    ProjectState& state_;
    AudioEngine& engine_;
    LiveMidiInjector& live_;
    juce::ValueTree project_;
    int trackCount_ = 0;
    juce::ValueTree tracks_[kColumns];
    juce::ValueTree clips_[kColumns][kScenes];

    juce::WebSliderRelay masterRelay_{"master"};
    juce::WebSliderRelay bpmRelay_{"bpm"};
    std::unique_ptr<juce::WebBrowserComponent> browser_;
};

} // namespace dawn
