#pragma once

#include "catalog/PluginCatalog.h"
#include "model/DataLayout.h"

#include <juce_core/juce_core.h>

namespace dawn {

// Resolved once at startup. Portable mode is DAWN_PORTABLE.txt or DAWN_DATA/
// beside the executable (or beside the .app bundle on macOS).
class AppPaths {
public:
    static AppPaths resolve();
    static void setCurrent(AppPaths paths);
    static const AppPaths& current();

    bool portable() const noexcept { return layout_.portable; }
    const DataLayout& layout() const noexcept { return layout_; }
    juce::File root() const { return juce::File(layout_.root); }
    juce::File preferences() const { return juce::File(layout_.preferences); }
    juce::File inputMappings() const { return juce::File(layout_.inputMappings); }
    juce::File downloads() const { return juce::File(layout_.downloads); }
    juce::File plugins(PackageFormat format) const;

private:
    explicit AppPaths(DataLayout layout);
    DataLayout layout_;
};

} // namespace dawn
