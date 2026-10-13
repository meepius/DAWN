#include "juce/CompingComponent.h"

#include <cmath>

namespace dawn {
namespace {
const juce::Colour kLanes[] = {juce::Colour(0xff39ff14), juce::Colour(0xff00f0ff), juce::Colour(0xfffff200), juce::Colour(0xffff4fd8)};
constexpr int kLaneCount = 4;
} // namespace

void CompingComponent::setClip(juce::ValueTree clip) {
    clip_ = std::move(clip);
    repaint();
}

double CompingComponent::beatFor(int x) const {
    const double loop = clip_.isValid() ? static_cast<double>(clip_[Id::loopBeats]) : 4.0;
    const double span = loop > 0.0 ? loop : 4.0;
    const int width = juce::jmax(1, getWidth());
    const float ratio = juce::jlimit(0.0f, 1.0f, static_cast<float>(x) / static_cast<float>(width));
    return static_cast<double>(ratio) * span;
}

void CompingComponent::paint(juce::Graphics& graphics) {
    graphics.fillAll(juce::Colour(0xff07080a));
    const double loop = clip_.isValid() ? static_cast<double>(clip_[Id::loopBeats]) : 4.0;
    const int height = juce::jmax(1, getHeight() / kLaneCount);
    for (int lane = 0; lane < kLaneCount; ++lane) {
        auto row = juce::Rectangle<int>(0, lane * height, getWidth(), height - 4);
        graphics.setColour(juce::Colour(0xff10140f));
        graphics.fillRect(row);
        graphics.setColour(kLanes[lane].withAlpha(0.85f));
        graphics.drawText("TAKE " + juce::String(lane + 1), row.reduced(8, 0), juce::Justification::centredLeft);
    }
    if (!clip_.isValid() || loop <= 0.0) return;
    for (const auto& child : clip_) {
        if (!child.hasType(Id::comp)) continue;
        const int take = juce::jlimit(0, kLaneCount - 1, static_cast<int>(child[Id::take]));
        const double start = static_cast<double>(child[Id::start]);
        const double length = static_cast<double>(child[Id::length]);
        const int x = static_cast<int>(start / loop * getWidth());
        const int w = juce::jmax(2, static_cast<int>(length / loop * getWidth()));
        graphics.setColour(kLanes[take].withAlpha(0.35f));
        graphics.fillRect(x, take * height, w, height - 4);
    }
}

void CompingComponent::mouseDown(const juce::MouseEvent& event) {
    if (event.mods.isRightButtonDown()) {
        if (clip_.isValid()) {
            for (int i = clip_.getNumChildren() - 1; i >= 0; --i) {
                if (clip_.getChild(i).hasType(Id::comp)) clip_.removeChild(i, nullptr);
            }
        }
        repaint();
        return;
    }
    const int rowHeight = juce::jmax(1, getHeight() / kLaneCount);
    lane_ = juce::jlimit(0, kLaneCount - 1, static_cast<int>(event.y) / rowHeight);
    anchor_ = beatFor(static_cast<int>(event.x));
    dragging_ = true;
}

void CompingComponent::mouseDrag(const juce::MouseEvent& event) {
    juce::ignoreUnused(event);
    repaint();
}

void CompingComponent::mouseUp(const juce::MouseEvent& event) {
    if (!dragging_ || !clip_.isValid()) return;
    dragging_ = false;
    const double end = beatFor(static_cast<int>(event.x));
    const double start = juce::jmin(anchor_, end);
    const double length = juce::jmax(0.25, std::abs(end - anchor_));
    auto region = juce::ValueTree{Id::comp};
    region.setProperty(Id::take, lane_, nullptr);
    region.setProperty(Id::start, start, nullptr);
    region.setProperty(Id::length, length, nullptr);
    clip_.appendChild(region, nullptr);
    repaint();
}

} // namespace dawn
