#include "model/DataLayout.h"

#include <iostream>

namespace {

int failures = 0;

void expect(bool condition, const char* message) {
    if (condition) return;
    std::cerr << "FAIL " << message << '\n';
    ++failures;
}

} // namespace

int runPortableTests() {
    using namespace dawn;
    expect(portableAnchor("/Volumes/USB/DAWN.app/Contents/MacOS") == "/Volumes/USB", "a macOS bundle anchors beside the .app");
    expect(portableAnchor("/usb/DAWN") == "/usb/DAWN", "a loose executable anchors in its own folder");

    const auto usb = makeDataLayout("/usb/DAWN", true, false, "/home/me/.config/dawn", "/home/me/.vst3", "/home/me/.clap");
    expect(usb.portable, "DAWN_PORTABLE.txt selects portable mode");
    expect(usb.root == "/usb/DAWN/DAWN_DATA", "portable data stays next to the executable");
    expect(usb.preferences == "/usb/DAWN/DAWN_DATA/preferences.xml", "preferences leave the host config directory");
    expect(usb.inputMappings == "/usb/DAWN/DAWN_DATA/input-mappings.xml", "mappings stay on the USB stick");
    expect(usb.vst3 == "/usb/DAWN/DAWN_DATA/Plugins/VST3", "plugin downloads stay on the USB stick");

    const auto folder = makeDataLayout("/usb/DAWN", false, true, "/home/me/.config/dawn", "/home/me/.vst3", "/home/me/.clap");
    expect(folder.portable && folder.downloads == "/usb/DAWN/DAWN_DATA/Downloads", "DAWN_DATA alone is enough");

    const auto host = makeDataLayout("/usr/bin", false, false, "/home/me/.config/dawn", "/home/me/.vst3", "/home/me/.clap");
    expect(!host.portable, "without the flag, portable mode stays off");
    expect(host.root == "/home/me/.config/dawn", "preferences fall back to the OS config directory");
    expect(host.vst3 == "/home/me/.vst3" && host.clap == "/home/me/.clap", "plugins fall back to the OS plug-in folders");

    if (failures != 0) std::cerr << failures << " portable checks failed\n";
    return failures;
}
