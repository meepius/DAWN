#include "juce/MasterVisualizers.h"

#include <cmath>

namespace dawn {
namespace {
const juce::Colour kGreen{0xff39ff14};
const juce::Colour kCyan{0xff00f0ff};

void strokeGlow(juce::Graphics& graphics, const juce::Path& path, juce::Colour colour) {
    graphics.setColour(colour.withAlpha(0.18f));
    graphics.strokePath(path, juce::PathStrokeType(6.f));
    graphics.setColour(colour.withAlpha(0.45f));
    graphics.strokePath(path, juce::PathStrokeType(2.5f));
    graphics.setColour(colour);
    graphics.strokePath(path, juce::PathStrokeType(1.2f));
}
} // namespace

void Oscilloscope::pushSamples(const float* mono, int count) {
    if (mono == nullptr || count <= 0) return;
    for (int i = 0; i < count; ++i) {
        samples_[static_cast<std::size_t>(filled_)] = mono[i];
        filled_ = (filled_ + 1) % 512;
    }
    repaint();
}

void Oscilloscope::paint(juce::Graphics& graphics) {
    graphics.fillAll(juce::Colour(0xff050605));
    juce::Path path;
    const float mid = static_cast<float>(getHeight()) * 0.5f;
    for (int i = 0; i < 512; ++i) {
        const int index = (filled_ + i) % 512;
        const float x = static_cast<float>(i) / 511.f * static_cast<float>(getWidth());
        const float y = mid - samples_[static_cast<std::size_t>(index)] * mid;
        if (i == 0) path.startNewSubPath(x, y);
        else path.lineTo(x, y);
    }
    strokeGlow(graphics, path, kGreen);
}

void FFTSpectrumAnalyzer::pushSamples(const float* mono, int count) {
    if (mono == nullptr || count <= 0) return;
    for (int i = 0; i < count; ++i) {
        fifo_[static_cast<std::size_t>(count_)] = mono[i];
        count_ = (count_ + 1) % 1024;
    }
    float windowed[2048]{};
    for (int i = 0; i < 1024; ++i) {
        const int index = (count_ + i) % 1024;
        const float window = 0.5f - 0.5f * std::cos(6.2831853f * static_cast<float>(i) / 1023.f);
        windowed[i] = fifo_[static_cast<std::size_t>(index)] * window;
    }
    fft_.performFrequencyOnlyForwardTransform(windowed);
    for (int i = 0; i < 512; ++i) magnitudes_[i] = windowed[i];
    repaint();
}

void FFTSpectrumAnalyzer::paint(juce::Graphics& graphics) {
    graphics.fillAll(juce::Colour(0xff050605));
    juce::Path path;
    path.startNewSubPath(0.f, static_cast<float>(getHeight()));
    for (int i = 1; i < 128; ++i) {
        const float x = static_cast<float>(i) / 127.f * static_cast<float>(getWidth());
        const float mag = std::min(1.f, magnitudes_[i] * 0.15f);
        path.lineTo(x, static_cast<float>(getHeight()) * (1.f - mag));
    }
    strokeGlow(graphics, path, kCyan);
}

MasterVisualizers::MasterVisualizers(VisualizerDataQueue& queue) : queue_(queue) {
    addAndMakeVisible(scope_);
    addAndMakeVisible(spectrum_);
    startTimerHz(60);
}

void MasterVisualizers::resized() {
    auto area = getLocalBounds();
    scope_.setBounds(area.removeFromLeft(area.getWidth() / 2).reduced(2));
    spectrum_.setBounds(area.reduced(2));
}

void MasterVisualizers::timerCallback() {
    float left[256]{};
    float right[256]{};
    const int got = queue_.pop(left, right, 256);
    if (got <= 0) return;
    float mono[256]{};
    for (int i = 0; i < got; ++i) mono[i] = 0.5f * (left[i] + right[i]);
    scope_.pushSamples(mono, got);
    spectrum_.pushSamples(mono, got);
}

} // namespace dawn
