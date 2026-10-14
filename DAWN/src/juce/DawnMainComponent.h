#pragma once

#include "juce/LiveMidiInjector.h"
#include "juce/MasterVisualizers.h"
#include "juce/ProjectState.h"

#include <memory>

namespace dawn {

// Session view. Layout is Grid and FlexBox. Transport, clip launches, levels,
// and pads all write the project tree or a lock-free engine field. The audio
// callback only reads the published snapshot and those atomics.
class DawnMainComponent : public juce::Component, private juce::Timer {
public:
    DawnMainComponent(ProjectState& state, AudioEngine& engine, VisualizerDataQueue& visuals, LiveMidiInjector& live);
    ~DawnMainComponent() override;

    void paint(juce::Graphics& graphics) override;
    void resized() override;

private:
    static constexpr int kColumns = 6;
    static constexpr int kScenes = 4;

    class TransportBar;
    class SessionView;
    class DevicePanel;

    void timerCallback() override;
    void launchClip(juce::ValueTree clip);
    void selectTrack(int index);
    double quantizeBeats() const;
    double quantized(double beat) const;
    void setSend(juce::ValueTree track, juce::ValueTree destination, float level);
    juce::ValueTree trackNamed(const juce::String& fragment) const;

    ProjectState& state_;
    AudioEngine& engine_;
    LiveMidiInjector& live_;
    juce::ValueTree project_;
    int selected_ = 0;
    int trackCount_ = 0;
    juce::ValueTree tracks_[kColumns];
    juce::ValueTree clips_[kColumns][kScenes];

    TransportBar* transport_ = nullptr;
    SessionView* session_ = nullptr;
    DevicePanel* devices_ = nullptr;

    std::unique_ptr<TransportBar> transportHolder_;
    std::unique_ptr<SessionView> sessionHolder_;
    std::unique_ptr<DevicePanel> deviceHolder_;
};

} // namespace dawn
