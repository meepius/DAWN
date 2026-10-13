#pragma once

#include <string>
#include <string_view>

namespace dawn {

struct DataLayout {
    bool portable = false;
    std::string root;
    std::string preferences;
    std::string inputMappings;
    std::string vst3;
    std::string clap;
    std::string downloads;
};

// If the executable sits inside App.app/Contents/MacOS, the anchor is the
// folder that contains the .app. Otherwise it is the executable's folder.
std::string portableAnchor(std::string_view executableDirectory);

// marker or dataDirectory means every DAWN file lives under <anchor>/DAWN_DATA.
// Otherwise preferences use osConfigRoot and plugins use the OS plug-in folders.
DataLayout makeDataLayout(std::string_view anchorDirectory,
                          bool markerFile,
                          bool dataDirectory,
                          std::string_view osConfigRoot,
                          std::string_view osVst3,
                          std::string_view osClap);

} // namespace dawn
