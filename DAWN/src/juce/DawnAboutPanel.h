#pragma once

#include <juce_gui_basics/juce_gui_basics.h>

namespace dawn {

class DawnAboutPanel : public juce::Component {
public:
    DawnAboutPanel();
    void paint(juce::Graphics& graphics) override;
    void resized() override;

private:
    juce::TextButton standard_{"DAWPROJECT STANDARD"};
};

} // namespace dawn
