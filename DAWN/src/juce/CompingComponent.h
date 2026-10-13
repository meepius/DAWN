#pragma once

#include "juce/ProjectState.h"

namespace dawn {

// Drag across a lane to keep that take. The snapshot only receives the comp.
class CompingComponent : public juce::Component {
public:
    void setClip(juce::ValueTree clip);
    void paint(juce::Graphics& graphics) override;
    void mouseDown(const juce::MouseEvent& event) override;
    void mouseDrag(const juce::MouseEvent& event) override;
    void mouseUp(const juce::MouseEvent& event) override;

private:
    double beatFor(int x) const;
    juce::ValueTree clip_;
    double anchor_ = 0.0;
    int lane_ = 0;
    bool dragging_ = false;
};

} // namespace dawn
