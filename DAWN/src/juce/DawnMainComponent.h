#pragma once

#include "juce/CompingComponent.h"
#include "juce/DawnAboutPanel.h"
#include "juce/InspectorPanel.h"
#include "juce/MasterVisualizers.h"

namespace dawn {

// One component for phone and desktop. Touch and mouse both arrive as mouse events.
class DawnMainComponent : public juce::Component {
public:
    DawnMainComponent(VisualizerDataQueue& visuals, juce::ValueTree project);

    void paint(juce::Graphics& graphics) override;
    void resized() override;
    void mouseDown(const juce::MouseEvent& event) override;

private:
    void showPage(int page);
    bool phone() const noexcept { return getWidth() < 720; }

    VisualizerDataQueue& queue_;
    juce::ValueTree project_;
    juce::TextButton nav_[5];
    juce::TextButton pads_[8];
    juce::TextButton keys_[8];
    juce::TextButton manifesto_{"MANIFESTO"};
    juce::Slider volume_;
    MasterVisualizers visuals_;
    InspectorPanel inspector_;
    CompingComponent comping_;
    DawnAboutPanel about_;
    int page_ = 0;
    int litPad_ = -1;
    int litKey_ = -1;
};

} // namespace dawn
