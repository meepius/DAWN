#include "juce/PluginInstaller.h"

#include "juce/AppPaths.h"

// User-started plugin install only. DAWN does not open a connection for anything else.

namespace dawn {
namespace {

bool safeRelativePath(const juce::String& name) {
    if (name.isEmpty() || name.startsWithChar('/') || name.startsWithChar('\\')) return false;
    if (name.contains("..")) return false;
    return !juce::File::isAbsolutePath(name);
}

} // namespace

juce::File pluginInstallDirectory(PackageFormat format) { return AppPaths::current().plugins(format); }

void PluginInstaller::install(const PluginPackage& package, std::function<void(InstallNotice)> finished) {
    if (task_ != nullptr) return;
    package_ = package;
    finished_ = std::move(finished);
    destination_ = pluginInstallDirectory(package.format);
    destination_.createDirectory();
    zip_ = AppPaths::current().downloads().getChildFile(package.id + ".zip");
    juce::URL::DownloadTaskOptions options;
    options.listener = this;
    task_ = juce::URL(package.url).downloadToFile(zip_, options);
    if (task_ == nullptr) {
        InstallNotice notice;
        notice.message = "The download did not start.";
        if (finished_) finished_(notice);
    }
}

void PluginInstaller::progress(juce::URL::DownloadTask*, juce::int64, juce::int64) {}

void PluginInstaller::finished(juce::URL::DownloadTask*, bool success) {
    InstallNotice notice;
    notice.installed = destination_;
    if (!success) {
        notice.message = "The download failed.";
    } else {
        juce::ZipFile zip(zip_);
        if (zip.getNumEntries() <= 0) {
            notice.message = "The archive was empty.";
        } else {
            bool wrote = false;
            for (int i = 0; i < zip.getNumEntries(); ++i) {
                const auto* entry = zip.getEntry(i);
                if (entry == nullptr || !safeRelativePath(entry->filename)) continue;
                if (zip.uncompressEntry(i, destination_, true)) wrote = true;
            }
            notice.ok = wrote;
            notice.message = wrote ? "Installed " + juce::String(package_.name) : "Nothing safe to extract.";
        }
    }
    zip_.deleteFile();
    task_.reset();
    auto callback = finished_;
    juce::MessageManager::callAsync([callback, notice] {
        if (callback) callback(notice);
    });
}

} // namespace dawn
