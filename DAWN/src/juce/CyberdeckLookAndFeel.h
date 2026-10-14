#pragma once

#include <juce_gui_basics/juce_gui_basics.h>

namespace dawn {

// Flat terminal panels, not skeuomorphic hardware. Phosphor, amber, and cyan
// sit on charcoal. Pressed controls invert. Nothing here is a picture of a knob.
class CyberdeckLookAndFeel : public juce::LookAndFeel_V4 {
public:
    CyberdeckLookAndFeel();

    void drawButtonBackground(juce::Graphics& graphics, juce::Button& button, const juce::Colour& background, bool highlighted, bool down) override;
    void drawButtonText(juce::Graphics& graphics, juce::TextButton& button, bool highlighted, bool down) override;
    void drawToggleButton(juce::Graphics& graphics, juce::ToggleButton& button, bool highlighted, bool down) override;
    void drawTickBox(juce::Graphics& graphics, juce::Component& component, float x, float y, float w, float h, bool ticked, bool isEnabled, bool highlighted, bool down) override;

    void drawLinearSlider(juce::Graphics& graphics, int x, int y, int width, int height, float position, float minPosition, float maxPosition,
                          juce::Slider::SliderStyle style, juce::Slider& slider) override;
    void drawRotarySlider(juce::Graphics& graphics, int x, int y, int width, int height, float proportional, float startAngle, float endAngle,
                          juce::Slider& slider) override;
    int getSliderThumbRadius(juce::Slider& slider) override;

    void drawDocumentWindowTitleBar(juce::DocumentWindow& window, juce::Graphics& graphics, int width, int height, int titleSpaceX, int titleSpaceWidth,
                                    const juce::Image* icon, bool drawTitleTextOnLeft) override;
    void fillResizableWindowBackground(juce::Graphics& graphics, int width, int height, const juce::BorderSize<int>& border, juce::ResizableWindow& window) override;
    void drawResizableFrame(juce::Graphics& graphics, int width, int height, const juce::BorderSize<int>& border) override;
    void drawGroupComponentOutline(juce::Graphics& graphics, int width, int height, const juce::String& text, const juce::Justification& justification,
                                   juce::GroupComponent& group) override;

    void fillTextEditorBackground(juce::Graphics& graphics, int width, int height, juce::TextEditor& editor) override;
    void drawTextEditorOutline(juce::Graphics& graphics, int width, int height, juce::TextEditor& editor) override;
    void drawComboBox(juce::Graphics& graphics, int width, int height, bool isButtonDown, int buttonX, int buttonY, int buttonWidth, int buttonHeight,
                      juce::ComboBox& box) override;
    void drawPopupMenuBackground(juce::Graphics& graphics, int width, int height) override;
    void drawLabel(juce::Graphics& graphics, juce::Label& label) override;
    void drawScrollbar(juce::Graphics& graphics, juce::ScrollBar& bar, int x, int y, int width, int height, bool vertical, int thumbStart, int thumbSize,
                       bool isMouseOver, bool isMouseDown) override;

    juce::Font getTextButtonFont(juce::TextButton& button, int buttonHeight) override;
    juce::Font getLabelFont(juce::Label& label) override;
};

} // namespace dawn
