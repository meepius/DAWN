#pragma once

#include <string>
#include <vector>

namespace dawn {

enum class PackageFormat { Vst3, Clap };

struct PluginPackage {
    std::string id;
    std::string name;
    std::string version;
    std::string url;
    std::string sha256;
    PackageFormat format = PackageFormat::Vst3;
    std::string os;
};

// Parses the catalog manifest. Unknown objects are skipped. No allocations
// beyond the returned vector.
std::vector<PluginPackage> parsePluginCatalog(std::string_view json);

} // namespace dawn
