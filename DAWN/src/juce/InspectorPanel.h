#pragma once

#include "juce/ProjectState.h"

namespace dawn {

class InspectorPanel : public juce::Component {
public:
    InspectorPanel();
    void setItem(juce::ValueTree item);
    void resized() override;

private:
    juce::Label title_;
    juce::TextEditor notes_;
    juce::ValueTree item_;
};

} // namespace dawn
