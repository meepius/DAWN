#include "model/Paths.h"

#include <vector>

namespace dawn {
namespace {

bool isAbsolute(std::string_view path) {
    if (path.empty()) return false;
    if (path.front() == '/' || path.front() == '\\') return true;
    return path.size() > 2 && path[1] == ':';
}

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
    std::vector<std::string> resolved;
    for (const auto& part : out) {
        if (part == ".") continue;
        if (part == "..") {
            if (!resolved.empty() && resolved.back() != "..") resolved.pop_back();
            else if (!isAbsolute(path)) resolved.push_back(part);
            continue;
        }
        resolved.push_back(part);
    }
    return resolved;
}

std::string join(std::string_view original, const std::vector<std::string>& resolved) {
    std::string prefix;
    if (!original.empty() && (original.front() == '/' || original.front() == '\\')) prefix = "/";
    else if (original.size() > 2 && original[1] == ':') prefix = std::string(original.substr(0, 2)) + "/";
    std::string path = prefix;
    for (std::size_t i = 0; i < resolved.size(); ++i) {
        if (i > 0) path.push_back('/');
        path += resolved[i];
    }
    return path;
}

} // namespace

std::string toRelativePath(std::string_view projectDirectory, std::string_view absoluteFile) {
    if (absoluteFile.empty()) return {};
    const auto root = parts(projectDirectory);
    const auto file = parts(absoluteFile);
    if (!isAbsolute(absoluteFile) || root.size() > file.size()) return std::string(absoluteFile);
    for (std::size_t i = 0; i < root.size(); ++i) {
        if (root[i] != file[i]) return std::string(absoluteFile);
    }
    std::string relative;
    for (std::size_t i = root.size(); i < file.size(); ++i) {
        if (!relative.empty()) relative.push_back('/');
        relative += file[i];
    }
    return relative.empty() ? "." : relative;
}

std::string fromRelativePath(std::string_view projectDirectory, std::string_view stored) {
    if (stored.empty()) return {};
    if (isAbsolute(stored)) return join(stored, parts(stored));
    const auto combined = std::string(projectDirectory) + "/" + std::string(stored);
    return join(combined, parts(combined));
}

} // namespace dawn
