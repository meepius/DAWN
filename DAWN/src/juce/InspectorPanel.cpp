#include "juce/InspectorPanel.h"

namespace dawn {

InspectorPanel::InspectorPanel() {
    title_.setText("NOTES", juce::dontSendNotification);
    title_.setJustificationType(juce::Justification::centredLeft);
    notes_.setMultiLine(true);
    notes_.setReturnKeyStartsNewLine(true);
    notes_.setTextToShowWhenEmpty("Notes for this track, clip, or plugin", juce::Colours::white.withAlpha(0.35f));
    notes_.onTextChange = [this] {
        if (item_.isValid()) item_.setProperty(Id::userNotes, notes_.getText(), nullptr);
    };
    addAndMakeVisible(title_);
    addAndMakeVisible(notes_);
}

void InspectorPanel::setItem(juce::ValueTree item) {
    item_ = std::move(item);
    const auto name = item_.isValid() ? item_[Id::name].toString() : juce::String("Nothing selected");
    title_.setText(name.isEmpty() ? "NOTES" : name, juce::dontSendNotification);
    notes_.setText(item_.isValid() ? item_[Id::userNotes].toString() : juce::String(), juce::dontSendNotification);
}

void InspectorPanel::resized() {
    auto area = getLocalBounds().reduced(8);
    title_.setBounds(area.removeFromTop(24));
    notes_.setBounds(area);
}

} // namespace dawn
