#pragma once

#include <juce_audio_processors/juce_audio_processors.h>
#include <functional>
#include <memory>

namespace dawn {

// Scans VST3 with KnownPluginList on a worker thread, then folds the result
// back onto the message thread. CLAP bundles are recorded by path because
// JUCE has no CLAP host format.
class PluginScanner : private juce::Thread {
public:
    PluginScanner();
    ~PluginScanner() override;

    juce::KnownPluginList& plugins() noexcept { return plugins_; }
    void rescan(std::function<void()> finished);

private:
    void run() override;

    juce::KnownPluginList plugins_;
    juce::AudioPluginFormatManager formats_;
    std::function<void()> finished_;
    juce::Array<juce::PluginDescription> found_;
    std::shared_ptr<bool> alive_ = std::make_shared<bool>(true);
};

} // namespace dawn
