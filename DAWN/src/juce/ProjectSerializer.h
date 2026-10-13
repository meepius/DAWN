#pragma once

#include "juce/PluginRack.h"
#include "juce/ProjectState.h"

#include <juce_audio_processors/juce_audio_processors.h>

namespace dawn {

class ProjectSerializer {
public:
    bool save(const ProjectState& state, PluginRack& rack, const juce::File& file) const;
    bool load(ProjectState& state, PluginRack& rack, juce::AudioPluginFormatManager& formats, const juce::File& file, juce::String& error) const;
    bool exportDawproject(const ProjectState& state, PluginRack& rack, const juce::File& file) const;
    bool importDawproject(ProjectState& state, PluginRack& rack, juce::AudioPluginFormatManager& formats, const juce::File& file, juce::String& error) const;
};

} // namespace dawn
