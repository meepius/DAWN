#include "juce/PluginScanner.h"

#include "juce/PluginInstaller.h"

namespace dawn {

PluginScanner::PluginScanner() : juce::Thread("dawn-plugin-scan") {
    formats_.addDefaultFormats();
}

PluginScanner::~PluginScanner() {
    *alive_ = false;
    stopThread(8000);
}

void PluginScanner::rescan(std::function<void()> finished) {
    if (isThreadRunning()) return;
    finished_ = std::move(finished);
    startThread();
}

void PluginScanner::run() {
    juce::KnownPluginList local;
    const auto vst3 = pluginInstallDirectory(PackageFormat::Vst3);
    const auto clap = pluginInstallDirectory(PackageFormat::Clap);
    vst3.createDirectory();
    clap.createDirectory();

    for (int i = 0; i < formats_.getNumFormats(); ++i) {
        auto* format = formats_.getFormat(i);
        if (format == nullptr || !format->getName().containsIgnoreCase("VST3")) continue;
        juce::FileSearchPath paths;
        paths.add(vst3);
        const auto deadMan = juce::File::getSpecialLocation(juce::File::tempDirectory).getChildFile("dawn-plugin-scan.txt");
        juce::PluginDirectoryScanner scanner(local, *format, paths, true, deadMan, false);
        juce::String current;
        while (!threadShouldExit() && scanner.scanNextFile(true, current)) {}
    }

    for (const auto& entry : juce::RangedDirectoryIterator(clap, true, "*.clap", juce::File::findDirectories)) {
        juce::PluginDescription description;
        description.name = entry.getFile().getFileNameWithoutExtension();
        description.pluginFormatName = "CLAP";
        description.fileOrIdentifier = entry.getFile().getFullPathName();
        description.manufacturerName = "CLAP";
        description.uniqueId = entry.getFile().getFileName().hashCode();
        local.addType(description);
    }

    juce::Array<juce::PluginDescription> found;
    for (const auto& type : local.getTypes()) found.add(type);
    auto callback = finished_;
    auto alive = alive_;
    juce::MessageManager::callAsync([this, alive, found, callback] {
        if (!*alive) return;
        plugins_.clear();
        for (const auto& type : found) plugins_.addType(type);
        if (callback) callback();
    });
}

} // namespace dawn
