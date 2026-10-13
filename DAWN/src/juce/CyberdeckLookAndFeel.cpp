#include "juce/CyberdeckLookAndFeel.h"

namespace dawn {
namespace {
const juce::Colour kBlack{0xff07080a};
const juce::Colour kGreen{0xff39ff14};
const juce::Colour kYellow{0xfffff200};
const juce::Colour kCyan{0xff00f0ff};
} // namespace

CyberdeckLookAndFeel::CyberdeckLookAndFeel() {
    setColour(juce::ResizableWindow::backgroundColourId, kBlack);
    setColour(juce::TextButton::buttonColourId, juce::Colour(0xff10140f));
    setColour(juce::TextButton::textColourOffId, kGreen);
    setColour(juce::TextButton::textColourOnId, kBlack);
    setColour(juce::Label::textColourId, kCyan);
    setColour(juce::TextEditor::textColourId, kGreen);
    setColour(juce::TextEditor::backgroundColourId, kBlack);
    setColour(juce::ScrollBar::thumbColourId, kGreen);
}

void CyberdeckLookAndFeel::drawButtonBackground(juce::Graphics& graphics, juce::Button& button, const juce::Colour&, bool highlighted, bool down) {
    auto bounds = button.getLocalBounds().toFloat().reduced(0.5f);
    graphics.setColour(down ? kGreen : juce::Colour(0xff10140f));
    graphics.fillRect(bounds);
    graphics.setColour(highlighted ? kYellow : kCyan);
    graphics.drawRect(bounds, 1.f);
}

void CyberdeckLookAndFeel::drawLinearSlider(juce::Graphics& graphics, int x, int y, int width, int height, float position, float, float,
                                           juce::Slider::SliderStyle, juce::Slider&) {
    const float mid = static_cast<float>(y) + static_cast<float>(height) * 0.5f;
    graphics.setColour(juce::Colour(0xff163018));
    graphics.fillRect(static_cast<float>(x), mid - 2.f, static_cast<float>(width), 4.f);
    graphics.setColour(kGreen);
    graphics.fillRect(static_cast<float>(x), mid - 2.f, position - static_cast<float>(x), 4.f);
    graphics.setColour(kYellow);
    graphics.fillRect(position - 3.f, mid - 8.f, 6.f, 16.f);
}

void CyberdeckLookAndFeel::fillTextEditorBackground(juce::Graphics& graphics, int width, int height, juce::TextEditor&) {
    graphics.setColour(kBlack);
    graphics.fillRect(0, 0, width, height);
}

void CyberdeckLookAndFeel::drawTextEditorOutline(juce::Graphics& graphics, int width, int height, juce::TextEditor&) {
    graphics.setColour(kCyan);
    graphics.drawRect(0, 0, width, height, 1);
}

} // namespace dawn
