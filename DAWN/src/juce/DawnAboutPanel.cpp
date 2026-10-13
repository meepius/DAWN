#include "juce/DawnAboutPanel.h"

namespace dawn {

DawnAboutPanel::DawnAboutPanel() {
    setBufferedToImage(true);
    addAndMakeVisible(standard_);
    standard_.onClick = [] {
        juce::URL("https://github.com/bitwig/dawproject").launchInDefaultBrowser();
    };
}

void DawnAboutPanel::paint(juce::Graphics& graphics) {
    graphics.fillAll(juce::Colour(0xff050605));
    for (int y = 0; y < getHeight(); y += 4) {
        graphics.setColour(juce::Colours::black.withAlpha(0.18f));
        graphics.fillRect(0, y, getWidth(), 1);
    }
    graphics.setColour(juce::Colour(0xff39ff14).withAlpha(0.25f));
    graphics.setFont(juce::FontOptions(72.f));
    graphics.drawText("DAWN", getLocalBounds().removeFromTop(180).translated(2, 2), juce::Justification::centred);
    graphics.setColour(juce::Colour(0xff39ff14));
    graphics.drawText("DAWN", getLocalBounds().removeFromTop(180), juce::Justification::centred);
    graphics.setColour(juce::Colour(0xff00f0ff));
    graphics.setFont(juce::FontOptions(18.f));
    graphics.drawFittedText("DAWN is 100% DRM-Free. No portals. No online checks. Licensed under GPLv3 for everyone, forever.",
                            getLocalBounds().reduced(48, 24).withTrimmedTop(180).withTrimmedBottom(80), juce::Justification::centred, 4);
}

void DawnAboutPanel::resized() { standard_.setBounds(getWidth() / 2 - 140, getHeight() - 72, 280, 36); }

} // namespace dawn
