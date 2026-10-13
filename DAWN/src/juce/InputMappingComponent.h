#pragma once

#include "juce/InputMapper.h"

namespace dawn {

class InputMappingComponent : public juce::Component {
public:
    InputMappingComponent(InputProfile& profile, InputMapper& mapper);

    void paint(juce::Graphics& graphics) override;
    void resized() override;
    void mouseDown(const juce::MouseEvent& event) override;

private:
    void toggleLearn();
    juce::String rowText(int index) const;

    InputProfile& profile_;
    InputMapper& mapper_;
    juce::TextButton learn_{"MIDI Learn"};
    int selected_ = 0;
};

} // namespace dawn
