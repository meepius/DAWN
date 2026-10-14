#include "juce/CatalogManager.h"

#include <juce_events/juce_events.h>

// The catalog request is the only manifest download in DAWN.
// It runs when the user asks the plugin hub to refresh. There is no telemetry.

namespace dawn {

CatalogManager::CatalogManager() : juce::Thread("dawn-catalog") {}

CatalogManager::~CatalogManager() { stopThread(4000); }

void CatalogManager::fetch(const juce::URL& manifest, std::function<void(std::vector<PluginPackage>, juce::String)> finished) {
    if (isThreadRunning()) return;
    url_ = manifest;
    finished_ = std::move(finished);
    startThread();
}

void CatalogManager::run() {
    const auto stream = url_.createInputStream(juce::URL::InputStreamOptions(juce::URL::ParameterHandling::inAddress).withConnectionTimeoutMs(15000));
    juce::String error;
    std::vector<PluginPackage> packages;
    if (stream == nullptr) error = "The catalog could not be downloaded.";
    else packages = packagesForThisOs(parsePluginCatalog(stream->readEntireStreamAsString().toStdString()));
    auto callback = finished_;
    juce::MessageManager::callAsync([callback, packages, error] {
        if (callback) callback(packages, error);
    });
}

std::vector<PluginPackage> packagesForThisOs(const std::vector<PluginPackage>& packages) {
    juce::String os = "linux";
#if JUCE_WINDOWS
    os = "windows";
#elif JUCE_MAC
    os = "macos";
#endif
    std::vector<PluginPackage> matched;
    for (const auto& package : packages) {
        if (package.os.empty() || package.os == "any" || package.os == os.toStdString()) matched.push_back(package);
    }
    return matched;
}

} // namespace dawn
