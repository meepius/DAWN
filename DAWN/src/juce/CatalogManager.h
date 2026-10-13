#pragma once

#include "catalog/PluginCatalog.h"

#include <juce_core/juce_core.h>
#include <functional>
#include <vector>

namespace dawn {

// Fetches the manifest on a worker thread and parses it there.
class CatalogManager : private juce::Thread {
public:
    CatalogManager();
    ~CatalogManager() override;

    void fetch(const juce::URL& manifest, std::function<void(std::vector<PluginPackage>, juce::String)> finished);

private:
    void run() override;

    juce::URL url_;
    std::function<void(std::vector<PluginPackage>, juce::String)> finished_;
};

std::vector<PluginPackage> packagesForThisOs(const std::vector<PluginPackage>& packages);

} // namespace dawn
