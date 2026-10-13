#pragma once

#include "host/DeviceProcessor.h"

#include <memory>
#include <string>
#include <vector>

namespace dawn {

struct PluginDescription {
    PluginFormat format = PluginFormat::Vst3;
    std::string identifier;
    std::string name;
    std::string path;
};

// VST3 and AudioUnit go through JUCE's plugin host.
// CLAP goes through the official CLAP host API behind the same wrapper.
// Both are constructed on the message thread and only then handed to a track.
class PluginHost {
public:
    virtual ~PluginHost() = default;
    virtual std::vector<PluginDescription> scan() = 0;
    virtual std::unique_ptr<RealtimeProcessor> instantiate(const PluginDescription& plugin) = 0;
};

} // namespace dawn
