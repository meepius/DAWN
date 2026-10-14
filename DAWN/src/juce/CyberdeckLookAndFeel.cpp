#include "juce/CyberdeckLookAndFeel.h"

namespace dawn {
namespace {

const juce::Colour kVoid{0xff07080c};
const juce::Colour kCharcoal{0xff10131a};
const juce::Colour kPanel{0xff171b24};
const juce::Colour kRaised{0xff222836};
const juce::Colour kHairline{0xff31384a};
const juce::Colour kPhosphor{0xff39ff14};
const juce::Colour kPhosphorDim{0xff1c7a12};
const juce::Colour kAmber{0xffffb000};
const juce::Colour kCyan{0xff3ee0ff};
const juce::Colour kInk{0xff071008};
const juce::Colour kText{0xffe7ffe4};
const juce::Colour kMuted{0xff8ea398};

juce::Font deckFont(float height, bool bold = false) {
    return juce::Font(juce::FontOptions(height).withStyle(bold ? "Bold" : "Plain"));
}

void fillVertical(juce::Graphics& graphics, juce::Rectangle<float> area, juce::Colour top, juce::Colour bottom) {
    juce::ColourGradient gradient(top, area.getX(), area.getY(), bottom, area.getX(), area.getBottom(), false);
    graphics.setGradientFill(gradient);
    graphics.fillRect(area);
}

void strokeInset(juce::Graphics& graphics, juce::Rectangle<float> area, juce::Colour edge) {
    graphics.setColour(juce::Colour(0xff000000).withAlpha(0.55f));
    graphics.drawRect(area, 1.0f);
    graphics.setColour(edge);
    graphics.drawRect(area.reduced(1.0f), 1.0f);
}

} // namespace

CyberdeckLookAndFeel::CyberdeckLookAndFeel() {
    setColour(juce::ResizableWindow::backgroundColourId, kVoid);
    setColour(juce::DocumentWindow::textColourId, kText);

    setColour(juce::TextButton::buttonColourId, kPanel);
    setColour(juce::TextButton::buttonOnColourId, kPhosphor);
    setColour(juce::TextButton::textColourOffId, kPhosphor);
    setColour(juce::TextButton::textColourOnId, kInk);

    setColour(juce::ToggleButton::textColourId, kText);
    setColour(juce::ToggleButton::tickColourId, kPhosphor);
    setColour(juce::ToggleButton::tickDisabledColourId, kMuted);

    setColour(juce::Label::textColourId, kCyan);
    setColour(juce::Label::backgroundColourId, juce::Colours::transparentBlack);
    setColour(juce::Label::outlineColourId, juce::Colours::transparentBlack);

    setColour(juce::Slider::backgroundColourId, kVoid);
    setColour(juce::Slider::trackColourId, kPhosphor);
    setColour(juce::Slider::thumbColourId, kAmber);
    setColour(juce::Slider::rotarySliderFillColourId, kPhosphor);
    setColour(juce::Slider::rotarySliderOutlineColourId, kHairline);
    setColour(juce::Slider::textBoxTextColourId, kPhosphor);
    setColour(juce::Slider::textBoxBackgroundColourId, kCharcoal);
    setColour(juce::Slider::textBoxOutlineColourId, kCyan.withAlpha(0.45f));
    setColour(juce::Slider::textBoxHighlightColourId, kAmber.withAlpha(0.35f));

    setColour(juce::TextEditor::backgroundColourId, kVoid);
    setColour(juce::TextEditor::textColourId, kPhosphor);
    setColour(juce::TextEditor::highlightColourId, kAmber.withAlpha(0.28f));
    setColour(juce::TextEditor::highlightedTextColourId, kText);
    setColour(juce::TextEditor::outlineColourId, kCyan.withAlpha(0.35f));
    setColour(juce::TextEditor::focusedOutlineColourId, kAmber);
    setColour(juce::CaretComponent::caretColourId, kAmber);

    setColour(juce::ComboBox::backgroundColourId, kPanel);
    setColour(juce::ComboBox::textColourId, kPhosphor);
    setColour(juce::ComboBox::outlineColourId, kCyan.withAlpha(0.45f));
    setColour(juce::ComboBox::arrowColourId, kAmber);
    setColour(juce::ComboBox::buttonColourId, kRaised);

    setColour(juce::PopupMenu::backgroundColourId, kCharcoal);
    setColour(juce::PopupMenu::textColourId, kText);
    setColour(juce::PopupMenu::highlightedBackgroundColourId, kPhosphor);
    setColour(juce::PopupMenu::highlightedTextColourId, kInk);
    setColour(juce::PopupMenu::headerTextColourId, kAmber);

    setColour(juce::ScrollBar::backgroundColourId, kVoid);
    setColour(juce::ScrollBar::thumbColourId, kPhosphorDim);
    setColour(juce::ScrollBar::trackColourId, kCharcoal);

    setColour(juce::GroupComponent::outlineColourId, kCyan.withAlpha(0.35f));
    setColour(juce::GroupComponent::textColourId, kAmber);

    setColour(juce::ListBox::backgroundColourId, kVoid);
    setColour(juce::ListBox::outlineColourId, kHairline);
    setColour(juce::ListBox::textColourId, kText);

    setColour(juce::ProgressBar::backgroundColourId, kVoid);
    setColour(juce::ProgressBar::foregroundColourId, kPhosphor);

    setColour(juce::AlertWindow::backgroundColourId, kCharcoal);
    setColour(juce::AlertWindow::textColourId, kText);
    setColour(juce::AlertWindow::outlineColourId, kAmber);

    setColour(juce::TooltipWindow::backgroundColourId, kCharcoal);
    setColour(juce::TooltipWindow::textColourId, kPhosphor);
    setColour(juce::TooltipWindow::outlineColourId, kAmber);
}

juce::Font CyberdeckLookAndFeel::getTextButtonFont(juce::TextButton&, int buttonHeight) {
    return deckFont(juce::jlimit(11.0f, 15.0f, static_cast<float>(buttonHeight) * 0.42f), true);
}

juce::Font CyberdeckLookAndFeel::getLabelFont(juce::Label&) {
    return deckFont(14.0f, true);
}

void CyberdeckLookAndFeel::drawButtonBackground(juce::Graphics& graphics, juce::Button& button, const juce::Colour& background, bool highlighted, bool down) {
    auto bounds = button.getLocalBounds().toFloat().reduced(1.0f);
    const bool on = down || button.getToggleState();
    const float alpha = button.isEnabled() ? 1.0f : 0.4f;

    juce::Colour fill = background.darker(0.35f);
    juce::Colour edge = kCyan.withAlpha(0.55f);
    if (on) {
        fill = kPhosphor;
        edge = kPhosphor;
    } else if (highlighted) {
        fill = kRaised;
        edge = kAmber;
    }

    fillVertical(graphics, bounds, fill.brighter(on ? 0.08f : 0.12f), fill.darker(0.25f));
    graphics.setColour(juce::Colours::white.withAlpha(on ? 0.18f : 0.05f));
    graphics.fillRect(bounds.withHeight(2.0f));
    graphics.setColour(edge.withMultipliedAlpha(alpha));
    graphics.drawRect(bounds, on ? 1.5f : 1.0f);
    if (highlighted && !on) {
        graphics.setColour(kAmber.withAlpha(0.12f));
        graphics.fillRect(bounds.reduced(1.0f));
    }
}

void CyberdeckLookAndFeel::drawButtonText(juce::Graphics& graphics, juce::TextButton& button, bool, bool down) {
    const bool on = down || button.getToggleState();
    graphics.setFont(getTextButtonFont(button, button.getHeight()));
    auto colour = button.findColour(on ? juce::TextButton::textColourOnId : juce::TextButton::textColourOffId);
    if (!button.isEnabled()) colour = kMuted;
    graphics.setColour(colour);
    graphics.drawFittedText(button.getButtonText(), button.getLocalBounds().reduced(6, 2), juce::Justification::centred, 1);
}

void CyberdeckLookAndFeel::drawTickBox(juce::Graphics& graphics, juce::Component&, float x, float y, float w, float h, bool ticked, bool isEnabled, bool highlighted, bool down) {
    auto box = juce::Rectangle<float>(x, y, w, h).reduced(1.0f);
    graphics.setColour(ticked || down ? kPhosphor : kVoid);
    graphics.fillRect(box);
    graphics.setColour((highlighted ? kAmber : kCyan).withAlpha(isEnabled ? 0.8f : 0.35f));
    graphics.drawRect(box, 1.0f);
    if (ticked || down) {
        graphics.setColour(kInk);
        auto mark = box.reduced(box.getWidth() * 0.28f, box.getHeight() * 0.28f);
        graphics.fillRect(mark);
    }
}

void CyberdeckLookAndFeel::drawToggleButton(juce::Graphics& graphics, juce::ToggleButton& button, bool highlighted, bool down) {
    const float size = juce::jmin(16.0f, static_cast<float>(button.getHeight()) - 4.0f);
    const float y = (static_cast<float>(button.getHeight()) - size) * 0.5f;
    drawTickBox(graphics, button, 4.0f, y, size, size, button.getToggleState(), button.isEnabled(), highlighted, down);
    graphics.setColour(button.isEnabled() ? kText : kMuted);
    graphics.setFont(deckFont(14.0f));
    graphics.drawFittedText(button.getButtonText(), juce::Rectangle<int>(static_cast<int>(size) + 12, 0, button.getWidth(), button.getHeight()),
                            juce::Justification::centredLeft, 1);
}

void CyberdeckLookAndFeel::drawLinearSlider(juce::Graphics& graphics, int x, int y, int width, int height, float position, float minPosition, float maxPosition,
                                           juce::Slider::SliderStyle style, juce::Slider& slider) {
    const bool vertical = style == juce::Slider::LinearVertical || style == juce::Slider::LinearBarVertical || style == juce::Slider::TwoValueVertical
                          || style == juce::Slider::ThreeValueVertical;
    const bool ranged = style == juce::Slider::TwoValueHorizontal || style == juce::Slider::TwoValueVertical || style == juce::Slider::ThreeValueHorizontal
                        || style == juce::Slider::ThreeValueVertical;
    auto bounds = juce::Rectangle<float>(static_cast<float>(x), static_cast<float>(y), static_cast<float>(width), static_cast<float>(height));

    if (vertical) {
        const float trackW = juce::jmin(10.0f, bounds.getWidth() * 0.45f);
        auto track = bounds.withSizeKeepingCentre(trackW, bounds.getHeight()).reduced(0.0f, 6.0f);
        graphics.setColour(kVoid);
        graphics.fillRect(track);
        strokeInset(graphics, track, kHairline);
        const float fillTop = ranged ? juce::jmin(minPosition, maxPosition) : position;
        const float fillBottom = ranged ? juce::jmax(minPosition, maxPosition) : track.getBottom();
        auto active = juce::Rectangle<float>(track.getX() + 2.0f, fillTop, track.getWidth() - 4.0f, juce::jmax(2.0f, fillBottom - fillTop));
        fillVertical(graphics, active, kPhosphor, kPhosphorDim);
        auto thumb = juce::Rectangle<float>(bounds.getCentreX() - 9.0f, position - 7.0f, 18.0f, 14.0f);
        fillVertical(graphics, thumb, kAmber.brighter(0.15f), kAmber.darker(0.35f));
        graphics.setColour(kInk);
        graphics.fillRect(thumb.reduced(4.0f, 6.0f).withHeight(2.0f).withY(thumb.getCentreY() - 1.0f));
        graphics.setColour(kAmber.brighter(0.4f));
        graphics.drawRect(thumb, 1.0f);
        juce::ignoreUnused(slider);
        return;
    }

    const float trackH = 8.0f;
    auto track = bounds.withSizeKeepingCentre(bounds.getWidth(), trackH).reduced(8.0f, 0.0f);
    graphics.setColour(slider.findColour(juce::Slider::backgroundColourId));
    graphics.fillRect(track);
    strokeInset(graphics, track, kHairline);
    const float fillLeft = ranged ? juce::jmin(minPosition, maxPosition) : track.getX();
    const float fillRight = ranged ? juce::jmax(minPosition, maxPosition) : position;
    auto active = juce::Rectangle<float>(fillLeft, track.getY() + 2.0f, juce::jmax(2.0f, fillRight - fillLeft), track.getHeight() - 4.0f);
    juce::ColourGradient activeFill(kPhosphorDim, active.getX(), active.getY(), kPhosphor, active.getRight(), active.getY(), false);
    graphics.setGradientFill(activeFill);
    graphics.fillRect(active);
    graphics.setColour(kAmber.withAlpha(0.9f));
    graphics.fillRect(active.getRight() - 2.0f, active.getY(), 2.0f, active.getHeight());

    auto thumb = juce::Rectangle<float>(position - 6.0f, bounds.getCentreY() - 11.0f, 12.0f, 22.0f);
    fillVertical(graphics, thumb, juce::Colour(0xffffd36a), kAmber.darker(0.45f));
    graphics.setColour(kInk.withAlpha(0.85f));
    graphics.fillRect(thumb.getCentreX() - 1.0f, thumb.getY() + 4.0f, 2.0f, thumb.getHeight() - 8.0f);
    graphics.setColour(kText.withAlpha(0.7f));
    graphics.drawRect(thumb, 1.0f);
}

void CyberdeckLookAndFeel::drawRotarySlider(juce::Graphics& graphics, int x, int y, int width, int height, float proportional, float startAngle, float endAngle,
                                           juce::Slider& slider) {
    auto bounds = juce::Rectangle<float>(static_cast<float>(x), static_cast<float>(y), static_cast<float>(width), static_cast<float>(height)).reduced(4.0f);
    const float radius = juce::jmin(bounds.getWidth(), bounds.getHeight()) * 0.5f;
    const auto centre = bounds.getCentre();
    const float angle = startAngle + proportional * (endAngle - startAngle);
    const float arcRadius = radius - 4.0f;

    graphics.setColour(kVoid);
    graphics.fillEllipse(centre.x - radius, centre.y - radius, radius * 2.0f, radius * 2.0f);
    graphics.setColour(kHairline);
    graphics.drawEllipse(centre.x - radius, centre.y - radius, radius * 2.0f, radius * 2.0f, 2.0f);

    juce::Path track;
    track.addCentredArc(centre.x, centre.y, arcRadius, arcRadius, 0.0f, startAngle, endAngle, true);
    graphics.setColour(slider.findColour(juce::Slider::rotarySliderOutlineColourId));
    graphics.strokePath(track, juce::PathStrokeType(4.0f, juce::PathStrokeType::curved, juce::PathStrokeType::rounded));

    juce::Path value;
    value.addCentredArc(centre.x, centre.y, arcRadius, arcRadius, 0.0f, startAngle, angle, true);
    graphics.setColour(slider.findColour(juce::Slider::rotarySliderFillColourId));
    graphics.strokePath(value, juce::PathStrokeType(4.0f, juce::PathStrokeType::curved, juce::PathStrokeType::rounded));

    const float cap = radius * 0.62f;
    graphics.setColour(kPanel);
    graphics.fillEllipse(centre.x - cap, centre.y - cap, cap * 2.0f, cap * 2.0f);
    graphics.setColour(kCyan.withAlpha(0.35f));
    graphics.drawEllipse(centre.x - cap, centre.y - cap, cap * 2.0f, cap * 2.0f, 1.0f);

    juce::Path pointer;
    pointer.addRectangle(-1.5f, -cap + 4.0f, 3.0f, cap * 0.55f);
    graphics.setColour(kAmber);
    graphics.fillPath(pointer, juce::AffineTransform::rotation(angle).translated(centre.x, centre.y));
}

int CyberdeckLookAndFeel::getSliderThumbRadius(juce::Slider&) {
    return 8;
}

void CyberdeckLookAndFeel::drawDocumentWindowTitleBar(juce::DocumentWindow& window, juce::Graphics& graphics, int width, int height, int titleSpaceX, int titleSpaceWidth,
                                                     const juce::Image* icon, bool drawTitleTextOnLeft) {
    auto bar = juce::Rectangle<float>(0.0f, 0.0f, static_cast<float>(width), static_cast<float>(height));
    fillVertical(graphics, bar, juce::Colour(0xff1c2230), kVoid);
    graphics.setColour(kAmber);
    graphics.fillRect(0.0f, 0.0f, 4.0f, bar.getHeight());
    graphics.setColour(kPhosphor.withAlpha(0.85f));
    graphics.fillRect(0.0f, bar.getHeight() - 1.0f, bar.getWidth(), 1.0f);

    auto title = window.getName();
    graphics.setColour(findColour(juce::DocumentWindow::textColourId));
    graphics.setFont(deckFont(15.0f, true));
    auto area = juce::Rectangle<int>(titleSpaceX, 0, titleSpaceWidth, height);
    if (icon != nullptr && icon->isValid()) {
        graphics.drawImageWithin(*icon, area.getX() + 6, (height - 16) / 2, 16, 16, juce::RectanglePlacement::centred);
        area.removeFromLeft(26);
    } else if (drawTitleTextOnLeft) {
        area.removeFromLeft(10);
    }
    graphics.drawFittedText(title, area, drawTitleTextOnLeft ? juce::Justification::centredLeft : juce::Justification::centred, 1);
}

void CyberdeckLookAndFeel::fillResizableWindowBackground(juce::Graphics& graphics, int width, int height, const juce::BorderSize<int>&, juce::ResizableWindow&) {
    auto area = juce::Rectangle<float>(0.0f, 0.0f, static_cast<float>(width), static_cast<float>(height));
    fillVertical(graphics, area, juce::Colour(0xff12151c), kVoid);
    graphics.setColour(juce::Colours::white.withAlpha(0.03f));
    graphics.fillRect(area.removeFromTop(1.0f));
}

void CyberdeckLookAndFeel::drawResizableFrame(juce::Graphics& graphics, int width, int height, const juce::BorderSize<int>& border) {
    if (border.getTop() < 1 && border.getLeft() < 1) return;
    auto frame = juce::Rectangle<int>(0, 0, width, height).toFloat();
    graphics.setColour(kVoid);
    graphics.drawRect(frame, 2.0f);
    graphics.setColour(kCyan.withAlpha(0.28f));
    graphics.drawRect(frame.reduced(1.0f), 1.0f);
}

void CyberdeckLookAndFeel::drawGroupComponentOutline(juce::Graphics& graphics, int width, int height, const juce::String& text, const juce::Justification&, juce::GroupComponent&) {
    auto area = juce::Rectangle<float>(0.0f, 8.0f, static_cast<float>(width), static_cast<float>(height) - 8.0f).reduced(1.0f);
    graphics.setColour(kPanel.withAlpha(0.65f));
    graphics.fillRect(area);
    graphics.setColour(kCyan.withAlpha(0.35f));
    graphics.drawRect(area, 1.0f);
    graphics.setColour(kAmber);
    graphics.fillRect(area.getX(), area.getY(), juce::jmin(36.0f, area.getWidth()), 2.0f);
    if (text.isNotEmpty()) {
        graphics.setColour(kAmber);
        graphics.setFont(deckFont(12.0f, true));
        graphics.drawFittedText(text, juce::Rectangle<int>(10, 0, width - 16, 16), juce::Justification::centredLeft, 1);
    }
}

void CyberdeckLookAndFeel::fillTextEditorBackground(juce::Graphics& graphics, int width, int height, juce::TextEditor& editor) {
    graphics.setColour(editor.findColour(juce::TextEditor::backgroundColourId));
    graphics.fillRect(0, 0, width, height);
    graphics.setColour(juce::Colours::white.withAlpha(0.04f));
    graphics.fillRect(0, 0, width, 1);
}

void CyberdeckLookAndFeel::drawTextEditorOutline(juce::Graphics& graphics, int width, int height, juce::TextEditor& editor) {
    const auto colour = editor.hasKeyboardFocus(true) ? editor.findColour(juce::TextEditor::focusedOutlineColourId)
                                                      : editor.findColour(juce::TextEditor::outlineColourId);
    graphics.setColour(colour);
    graphics.drawRect(0, 0, width, height, editor.hasKeyboardFocus(true) ? 2 : 1);
}

void CyberdeckLookAndFeel::drawComboBox(juce::Graphics& graphics, int width, int height, bool isButtonDown, int buttonX, int, int, int, juce::ComboBox& box) {
    auto bounds = juce::Rectangle<float>(0.0f, 0.0f, static_cast<float>(width), static_cast<float>(height)).reduced(0.5f);
    fillVertical(graphics, bounds, isButtonDown ? kRaised : kPanel, kCharcoal);
    graphics.setColour(box.findColour(juce::ComboBox::outlineColourId));
    graphics.drawRect(bounds, 1.0f);
    juce::Path arrow;
    const float cx = static_cast<float>(buttonX) + 10.0f;
    const float cy = bounds.getCentreY();
    arrow.addTriangle(cx, cy - 3.0f, cx + 8.0f, cy - 3.0f, cx + 4.0f, cy + 3.0f);
    graphics.setColour(box.findColour(juce::ComboBox::arrowColourId));
    graphics.fillPath(arrow);
}

void CyberdeckLookAndFeel::drawPopupMenuBackground(juce::Graphics& graphics, int width, int height) {
    auto area = juce::Rectangle<float>(0.0f, 0.0f, static_cast<float>(width), static_cast<float>(height));
    graphics.setColour(kCharcoal);
    graphics.fillRect(area);
    graphics.setColour(kAmber);
    graphics.fillRect(0.0f, 0.0f, 3.0f, area.getHeight());
    graphics.setColour(kCyan.withAlpha(0.4f));
    graphics.drawRect(area, 1.0f);
}

void CyberdeckLookAndFeel::drawLabel(juce::Graphics& graphics, juce::Label& label) {
    auto bounds = label.getLocalBounds().toFloat();
    const auto background = label.findColour(juce::Label::backgroundColourId);
    if (!background.isTransparent()) {
        graphics.setColour(background);
        graphics.fillRect(bounds);
    }
    if (!label.isBeingEdited()) {
        graphics.setColour(label.findColour(juce::Label::textColourId).withMultipliedAlpha(label.isEnabled() ? 1.0f : 0.45f));
        graphics.setFont(getLabelFont(label));
        graphics.drawFittedText(label.getText(), label.getLocalBounds().reduced(2, 0), label.getJustificationType(), juce::jmax(1, static_cast<int>(bounds.getHeight() / 14.0f)));
    }
    const auto outline = label.findColour(juce::Label::outlineColourId);
    if (!outline.isTransparent()) {
        graphics.setColour(outline);
        graphics.drawRect(bounds, 1.0f);
    }
}

void CyberdeckLookAndFeel::drawScrollbar(juce::Graphics& graphics, juce::ScrollBar&, int x, int y, int width, int height, bool, int thumbStart, int thumbSize, bool isMouseOver, bool isMouseDown) {
    graphics.setColour(kVoid);
    graphics.fillRect(x, y, width, height);
    if (thumbSize <= 0) return;
    const bool vertical = height > width;
    juce::Rectangle<float> thumb;
    if (vertical) thumb = juce::Rectangle<float>(static_cast<float>(x) + 2.0f, static_cast<float>(thumbStart), static_cast<float>(width) - 4.0f, static_cast<float>(thumbSize));
    else thumb = juce::Rectangle<float>(static_cast<float>(thumbStart), static_cast<float>(y) + 2.0f, static_cast<float>(thumbSize), static_cast<float>(height) - 4.0f);
    graphics.setColour(isMouseDown ? kPhosphor : isMouseOver ? kAmber : kPhosphorDim);
    graphics.fillRect(thumb);
}

} // namespace dawn
