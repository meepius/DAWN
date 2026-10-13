#include "catalog/PluginCatalog.h"

namespace dawn {
namespace {

std::string jsonString(std::string_view object, std::string_view key) {
    const auto token = std::string("\"") + std::string(key) + "\"";
    const auto at = object.find(token);
    if (at == std::string_view::npos) return {};
    auto cursor = object.find(':', at + token.size());
    if (cursor == std::string_view::npos) return {};
    cursor = object.find('"', cursor + 1);
    if (cursor == std::string_view::npos) return {};
    std::string value;
    for (std::size_t i = cursor + 1; i < object.size(); ++i) {
        if (object[i] == '\\' && i + 1 < object.size()) {
            value.push_back(object[++i]);
            continue;
        }
        if (object[i] == '"') break;
        value.push_back(object[i]);
    }
    return value;
}

} // namespace

std::vector<PluginPackage> parsePluginCatalog(std::string_view json) {
    std::vector<PluginPackage> packages;
    const auto list = json.find("\"packages\"");
    if (list == std::string_view::npos) return packages;
    int depth = 0;
    std::size_t start = 0;
    for (std::size_t i = list; i < json.size(); ++i) {
        if (json[i] == '{') {
            if (depth == 1) start = i;
            ++depth;
        } else if (json[i] == '}') {
            --depth;
            if (depth == 1) {
                const auto object = json.substr(start, i - start + 1);
                PluginPackage package;
                package.id = jsonString(object, "id");
                package.name = jsonString(object, "name");
                package.version = jsonString(object, "version");
                package.url = jsonString(object, "url");
                package.sha256 = jsonString(object, "sha256");
                package.os = jsonString(object, "os");
                const auto format = jsonString(object, "format");
                if (package.id.empty() || package.url.empty()) continue;
                package.format = format == "clap" ? PackageFormat::Clap : PackageFormat::Vst3;
                packages.push_back(std::move(package));
            }
            if (depth == 0) break;
        } else if (json[i] == '[' && depth == 0) {
            depth = 1;
        }
    }
    return packages;
}

} // namespace dawn
