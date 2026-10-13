#include "juce/Vst3Wrapper.h"

namespace dawn {

Vst3Wrapper::Vst3Wrapper(std::unique_ptr<juce::AudioPluginInstance> plugin) : plugin_(std::move(plugin)) {}

std::unique_ptr<Vst3Wrapper> Vst3Wrapper::create(juce::AudioPluginFormatManager& formats,
                                                 const juce::PluginDescription& description,
                                                 double sampleRate,
                                                 int maxBlock,
                                                 juce::String& error) {
    auto plugin = formats.createPluginInstance(description, sampleRate, maxBlock, error);
    if (plugin == nullptr) return nullptr;
    return std::make_unique<Vst3Wrapper>(std::move(plugin));
}

void Vst3Wrapper::prepareToPlay(double sampleRate, int maxBlock) {
    if (plugin_ != nullptr) plugin_->prepareToPlay(sampleRate, maxBlock);
}

void Vst3Wrapper::releaseResources() {
    if (plugin_ != nullptr) plugin_->releaseResources();
}

void Vst3Wrapper::process(juce::AudioBuffer<float>& audio, juce::MidiBuffer& midi) noexcept {
    if (plugin_ != nullptr) plugin_->processBlock(audio, midi);
}

bool Vst3Wrapper::getStateInformation(juce::MemoryBlock& dest) {
    if (plugin_ == nullptr) return false;
    plugin_->getStateInformation(dest);
    return true;
}

void Vst3Wrapper::setStateInformation(const void* data, int size) {
    if (plugin_ != nullptr && data != nullptr && size > 0) plugin_->setStateInformation(data, size);
}

} // namespace dawn
