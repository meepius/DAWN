#include "model/DataLayout.h"

#include <vector>

namespace dawn {
namespace {

std::vector<std::string> parts(std::string_view path) {
    std::vector<std::string> out;
    std::string current;
    for (char character : path) {
        if (character == '/' || character == '\\') {
            if (!current.empty()) out.push_back(current);
            current.clear();
        } else {
            current.push_back(character);
        }
    }
    if (!current.empty()) out.push_back(current);
    return out;
}

std::string joinAbsolute(std::string_view original, const std::vector<std::string>& resolved) {
    std::string prefix;
    if (!original.empty() && (original.front() == '/' || original.front() == '\\')) prefix = "/";
    else if (original.size() > 2 && original[1] == ':') prefix = std::string(original.substr(0, 2)) + "/";
    std::string path = prefix;
    for (std::size_t i = 0; i < resolved.size(); ++i) {
        if (i > 0 && (path.empty() || path.back() != '/')) path.push_back('/');
        path += resolved[i];
    }
    return path;
}

std::string child(std::string root, std::string_view name) {
    if (!root.empty() && root.back() != '/') root.push_back('/');
    root.append(name);
    return root;
}

} // namespace

std::string portableAnchor(std::string_view executableDirectory) {
    const auto path = parts(executableDirectory);
    if (path.size() >= 3 && path.back() == "MacOS" && path[path.size() - 2] == "Contents") {
        const auto& bundle = path[path.size() - 3];
        if (bundle.size() >= 4 && bundle.substr(bundle.size() - 4) == ".app") {
            return joinAbsolute(executableDirectory, {path.begin(), path.end() - 3});
        }
    }
    return std::string(executableDirectory);
}

DataLayout makeDataLayout(std::string_view anchorDirectory,
                          bool markerFile,
                          bool dataDirectory,
                          std::string_view osConfigRoot,
                          std::string_view osVst3,
                          std::string_view osClap) {
    DataLayout layout;
    layout.portable = markerFile || dataDirectory;
    layout.root = layout.portable ? child(std::string(anchorDirectory), "DAWN_DATA") : std::string(osConfigRoot);
    layout.preferences = child(layout.root, "preferences.xml");
    layout.inputMappings = child(layout.root, "input-mappings.xml");
    layout.downloads = child(layout.root, "Downloads");
    if (layout.portable) {
        layout.vst3 = child(layout.root, "Plugins/VST3");
        layout.clap = child(layout.root, "Plugins/CLAP");
    } else {
        layout.vst3 = std::string(osVst3);
        layout.clap = std::string(osClap);
    }
    return layout;
}

} // namespace dawn
