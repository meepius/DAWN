#pragma once

#include <string>
#include <string_view>

namespace dawn {

// Stored audio paths are relative to the project file whenever they can be.
std::string toRelativePath(std::string_view projectDirectory, std::string_view absoluteFile);
std::string fromRelativePath(std::string_view projectDirectory, std::string_view stored);

} // namespace dawn
