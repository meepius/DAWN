#pragma once

#include <juce_gui_basics/juce_gui_basics.h>

namespace dawn {

class CyberdeckLookAndFeel : public juce::LookAndFeel_V4 {
public:
    CyberdeckLookAndFeel();

    void drawButtonBackground(juce::Graphics& graphics, juce::Button& button, const juce::Colour& background, bool highlighted, bool down) override;
    void drawLinearSlider(juce::Graphics& graphics, int x, int y, int width, int height, float position, float min, float max,
                          juce::Slider::SliderStyle style, juce::Slider& slider) override;
    void fillTextEditorBackground(juce::Graphics& graphics, int width, int height, juce::TextEditor& editor) override;
    void drawTextEditorOutline(juce::Graphics& graphics, int width, int height, juce::TextEditor& editor) override;
};

} // namespace dawn
