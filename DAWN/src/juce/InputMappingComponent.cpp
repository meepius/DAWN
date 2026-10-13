#include "juce/InputMappingComponent.h"

namespace dawn {
namespace {
constexpr int kRow = 36;
const juce::Colour kBg{0xff101114};
const juce::Colour kRowBg{0xff1c1e24};
const juce::Colour kAccent{0xffd6f25c};
} // namespace

InputMappingComponent::InputMappingComponent(InputProfile& profile, InputMapper& mapper) : profile_(profile), mapper_(mapper) {
    addAndMakeVisible(learn_);
    learn_.onClick = [this] { toggleLearn(); };
    setWantsKeyboardFocus(true);
}

void InputMappingComponent::toggleLearn() {
    if (mapper_.isLearning()) {
        mapper_.cancelLearn();
        learn_.setButtonText("MIDI Learn");
        repaint();
        return;
    }
    learn_.setButtonText("Listening...");
    mapper_.armLearn([this](InputCapture capture) {
        profile_.bind(selected_, capture.source, capture.code, profile_.binding(selected_)[MapId::kind].toString(),
                      static_cast<int>(profile_.binding(selected_)[MapId::number]),
                      static_cast<int>(profile_.binding(selected_)[MapId::channel]));
        learn_.setButtonText("MIDI Learn");
        repaint();
    });
    repaint();
}

juce::String InputMappingComponent::rowText(int index) const {
    const auto row = profile_.binding(index);
    return row[MapId::source].toString() + " " + row[MapId::code].toString() + "  ->  " + row[MapId::kind].toString() + " "
           + row[MapId::number].toString();
}

void InputMappingComponent::paint(juce::Graphics& graphics) {
    graphics.fillAll(kBg);
    graphics.setColour(kAccent);
    graphics.setFont(juce::FontOptions(18.f));
    graphics.drawText("Input", 16, 8, getWidth() - 32, 28, juce::Justification::centredLeft);
    graphics.setColour(juce::Colours::white.withAlpha(0.65f));
    graphics.setFont(juce::FontOptions(13.f));
    graphics.drawText(mapper_.isLearning() ? "Press a key or a pad" : "Keys A-K play. Z and X move the octave.", 16, 36, getWidth() - 32, 20,
                      juce::Justification::centredLeft);
    for (int i = 0; i < profile_.size(); ++i) {
        const auto row = juce::Rectangle<int>(12, 96 + i * kRow, getWidth() - 24, kRow - 6);
        graphics.setColour(i == selected_ ? kAccent.withAlpha(0.18f) : kRowBg);
        graphics.fillRoundedRectangle(row.toFloat(), 6.f);
        graphics.setColour(i == selected_ ? kAccent : juce::Colours::white);
        graphics.drawText(rowText(i), row.reduced(12, 0), juce::Justification::centredLeft);
    }
}

void InputMappingComponent::resized() { learn_.setBounds(getWidth() - 148, 12, 132, 32); }

void InputMappingComponent::mouseDown(const juce::MouseEvent& event) {
    const int row = (event.y - 96) / kRow;
    if (row >= 0 && row < profile_.size()) {
        selected_ = row;
        repaint();
    }
    grabKeyboardFocus();
}

} // namespace dawn
