#include "juce/AppPaths.h"

namespace dawn {
namespace {

AppPaths& slot() {
    static AppPaths current = AppPaths::resolve();
    return current;
}

juce::File osConfigRoot() {
#if JUCE_MAC
    return juce::File::getSpecialLocation(juce::File::userApplicationDataDirectory).getChildFile("DAWN");
#elif JUCE_WINDOWS
    return juce::File::getSpecialLocation(juce::File::userApplicationDataDirectory).getChildFile("DAWN");
#else
    return juce::File::getSpecialLocation(juce::File::userApplicationDataDirectory).getChildFile("dawn");
#endif
}

juce::File osPlugins(PackageFormat format) {
    const auto folder = format == PackageFormat::Clap ? "CLAP" : "VST3";
#if JUCE_WINDOWS
    return juce::File::getSpecialLocation(juce::File::userApplicationDataDirectory)
        .getParentDirectory()
        .getChildFile("Local/Programs/Common")
        .getChildFile(folder);
#elif JUCE_MAC
    return juce::File::getSpecialLocation(juce::File::userHomeDirectory)
        .getChildFile("Library/Audio/Plug-Ins")
        .getChildFile(folder);
#else
    return juce::File::getSpecialLocation(juce::File::userHomeDirectory)
        .getChildFile(format == PackageFormat::Clap ? ".clap" : ".vst3");
#endif
}

bool flagged(const juce::File& directory) {
    return directory.getChildFile("DAWN_PORTABLE.txt").existsAsFile() || directory.getChildFile("DAWN_DATA").isDirectory();
}

} // namespace

AppPaths::AppPaths(DataLayout layout) : layout_(std::move(layout)) {}

AppPaths AppPaths::resolve() {
    const auto executable = juce::File::getSpecialLocation(juce::File::currentExecutableFile).getParentDirectory();
    juce::File anchor = executable;
    if (!flagged(executable)) {
        const auto besideBundle = juce::File(portableAnchor(executable.getFullPathName().toStdString()));
        if (besideBundle != executable && flagged(besideBundle)) anchor = besideBundle;
    }
    const bool portable = flagged(anchor);
    auto layout = makeDataLayout(anchor.getFullPathName().toStdString(), portable, false, osConfigRoot().getFullPathName().toStdString(),
                                 osPlugins(PackageFormat::Vst3).getFullPathName().toStdString(),
                                 osPlugins(PackageFormat::Clap).getFullPathName().toStdString());
    return AppPaths(std::move(layout));
}

void AppPaths::setCurrent(AppPaths paths) {
    if (paths.portable()) {
        paths.root().createDirectory();
        paths.downloads().createDirectory();
        paths.plugins(PackageFormat::Vst3).createDirectory();
        paths.plugins(PackageFormat::Clap).createDirectory();
    } else {
        paths.root().createDirectory();
    }
    slot() = std::move(paths);
}

const AppPaths& AppPaths::current() { return slot(); }

juce::File AppPaths::plugins(PackageFormat format) const {
    return juce::File(format == PackageFormat::Clap ? layout_.clap : layout_.vst3);
}

} // namespace dawn
