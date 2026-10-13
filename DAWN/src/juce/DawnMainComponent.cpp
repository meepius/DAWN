#include "juce/DawnMainComponent.h"

namespace dawn {
namespace {
const juce::Colour kBg{0xff101114};
const juce::Colour kPanel{0xff1c1e24};
const juce::Colour kAccent{0xffd6f25c};
const char* kPages[] = {"Session", "Arrange", "Clip", "Devices", "Keys"};
} // namespace

DawnMainComponent::DawnMainComponent(VisualizerDataQueue& queue, juce::ValueTree project)
    : queue_(queue), project_(std::move(project)), visuals_(queue_) {
    for (int i = 0; i < 5; ++i) {
        nav_[i].setButtonText(kPages[i]);
        nav_[i].onClick = [this, i] { showPage(i); };
        addAndMakeVisible(nav_[i]);
    }
    for (int i = 0; i < 8; ++i) {
        pads_[i].setButtonText("Clip " + juce::String(i + 1));
        pads_[i].onClick = [this, i] {
            litPad_ = i;
            repaint();
        };
        keys_[i].setButtonText(juce::MidiMessage::getMidiNoteName(60 + i, true, true, 4));
        keys_[i].onStateChange = [this, i] {
            litKey_ = keys_[i].isDown() ? i : -1;
            repaint();
        };
        addAndMakeVisible(pads_[i]);
        addAndMakeVisible(keys_[i]);
    }
    volume_.setRange(0.0, 1.0, 0.001);
    volume_.setValue(0.8, juce::dontSendNotification);
    volume_.setSliderStyle(juce::Slider::LinearHorizontal);
    volume_.setTextBoxStyle(juce::Slider::NoTextBox, false, 0, 0);
    volume_.setColour(juce::Slider::thumbColourId, kAccent);
    volume_.setColour(juce::Slider::trackColourId, kAccent.withAlpha(0.45f));
    addAndMakeVisible(volume_);
    addAndMakeVisible(visuals_);
    addAndMakeVisible(inspector_);
    addAndMakeVisible(comping_);
    addChildComponent(about_);
    addAndMakeVisible(manifesto_);
    manifesto_.onClick = [this] {
        about_.setVisible(!about_.isVisible());
        if (about_.isVisible()) about_.toFront(false);
    };
    if (project_.getNumChildren() > 0) inspector_.setItem(project_.getChild(0));
    for (auto track : project_) {
        for (auto child : track) {
            if (child.hasType(Id::clip)) {
                comping_.setClip(child);
                break;
            }
        }
    }
    setWantsKeyboardFocus(true);
}

void DawnMainComponent::showPage(int page) {
    page_ = page;
    resized();
    repaint();
}

void DawnMainComponent::paint(juce::Graphics& graphics) {
    graphics.fillAll(kBg);
    graphics.setColour(juce::Colours::white);
    graphics.setFont(juce::FontOptions(18.f));
    const auto title = phone() ? juce::String(kPages[page_]) : juce::String("Session");
    graphics.drawText(title, 16, 8, 200, 28, juce::Justification::centredLeft);
    if (phone() && page_ == 1) {
        graphics.setColour(kPanel);
        for (int i = 0; i < 4; ++i) {
            graphics.fillRoundedRectangle(16.f, 80.f + static_cast<float>(i) * 48.f, static_cast<float>(getWidth() - 32), 36.f, 6.f);
        }
    }
    if (!phone()) {
        graphics.setColour(kPanel);
        graphics.fillRect(0, getHeight() - 168, getWidth(), 168);
        graphics.setColour(juce::Colours::white.withAlpha(0.7f));
        graphics.setFont(juce::FontOptions(13.f));
        graphics.drawText("Devices", 16, getHeight() - 160, 120, 20, juce::Justification::centredLeft);
    }
    for (int y = 0; y < getHeight(); y += 4) {
        graphics.setColour(juce::Colours::black.withAlpha(0.12f));
        graphics.fillRect(0, y, getWidth(), 1);
    }
}

void DawnMainComponent::resized() {
    const bool narrow = phone();
    auto area = getLocalBounds();
    auto bar = narrow ? area.removeFromBottom(64) : juce::Rectangle<int>();
    if (narrow) {
        auto slot = bar;
        const int width = slot.getWidth() / 5;
        for (int i = 0; i < 5; ++i) nav_[i].setBounds(slot.removeFromLeft(width).reduced(4, 8));
    }
    for (int i = 0; i < 5; ++i) nav_[i].setVisible(narrow);

    const bool session = !narrow || page_ == 0;
    const bool arrange = !narrow || page_ == 1;
    const bool clip = narrow && page_ == 2;
    const bool devices = narrow ? page_ == 3 : true;
    const bool keys = narrow ? page_ == 4 : false;
    comping_.setVisible(narrow ? page_ == 2 : false);
    inspector_.setVisible(!narrow);
    visuals_.setVisible(!narrow || page_ == 3);
    about_.setBounds(getLocalBounds());
    manifesto_.setBounds(getWidth() - 150, 8, 136, 28);
    auto content = area.reduced(12, 40);
    if (!narrow) {
        inspector_.setBounds(area.removeFromRight(240).reduced(8, 40));
        content.removeFromRight(0);
        content.removeFromBottom(150);
        visuals_.setBounds(16, getHeight() - 78, getWidth() - inspector_.getWidth() - 40, 70);
    } else if (page_ == 3) {
        visuals_.setBounds(content.removeFromBottom(120).reduced(4));
    }
    if (comping_.isVisible()) comping_.setBounds(content);

    auto padArea = content;
    const int padW = juce::jmax(1, padArea.getWidth() / 4);
    const int padH = juce::jmax(1, padArea.getHeight() / 2);
    for (int i = 0; i < 8; ++i) {
        const int column = i % 4;
        const int row = i / 4;
        pads_[i].setBounds(padArea.getX() + column * padW, padArea.getY() + row * padH, padW, padH);
        pads_[i].setVisible(session);
    }
    for (int i = 0; i < 8; ++i) {
        keys_[i].setBounds(content.getX() + i * (content.getWidth() / 8), content.getY(), content.getWidth() / 8, content.getHeight());
        keys_[i].setVisible(keys);
    }
    volume_.setVisible(devices);
    if (devices) {
        if (narrow) volume_.setBounds(content.reduced(8).removeFromTop(48));
        else volume_.setBounds(16, getHeight() - 120, getWidth() - 32, 48);
    }
    juce::ignoreUnused(arrange, clip);
}

void DawnMainComponent::mouseDown(const juce::MouseEvent&) { grabKeyboardFocus(); }

} // namespace dawn
