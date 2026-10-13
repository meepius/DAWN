#pragma once

#include "catalog/PluginCatalog.h"

#include <juce_core/juce_core.h>
#include <functional>

namespace dawn {

struct InstallNotice {
    bool ok = false;
    juce::String message;
    juce::File installed;
};

juce::File pluginInstallDirectory(PackageFormat format);

// Download and unzip on a background thread. Rejects zip entries that escape
// the destination directory. The callback runs on the message thread.
class PluginInstaller : private juce::URL::DownloadTask::Listener {
public:
    void install(const PluginPackage& package, std::function<void(InstallNotice)> finished);
    void progress(juce::URL::DownloadTask* task, juce::int64 downloaded, juce::int64 total) override;
    void finished(juce::URL::DownloadTask* task, bool success) override;

private:
    PluginPackage package_;
    juce::File zip_;
    juce::File destination_;
    std::unique_ptr<juce::URL::DownloadTask> task_;
    std::function<void(InstallNotice)> finished_;
};

} // namespace dawn
