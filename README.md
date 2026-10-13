# DAWN

```text
DAWN/
├── DAWN.exe          (or DAWN.app, or the Linux binary)
├── DAWN_PORTABLE.txt
└── DAWN_DATA/
    └── demo.dawn    PUBLIC ENEMY, PRIVATE NETWORK
```

DAWN is a lock-free, zero-telemetry, cross-platform DAW built for live performance and rapid arrangement. It features sub-sample accurate MIDI scheduling, advanced comping, and dynamic routing, all powered by a completely lock-free RCU audio engine.

**DAWN is 100% DRM-free. No background portals. No online checks. Licensed under GPLv3 for everyone, forever.**

## The Manifesto

The modern music production ecosystem is broken. Developers force you to install bloated license managers that eat your RAM, phone home to check your DRM status, and crash your sessions if your Wi-Fi drops. They gatekeep essential workflow features to sell you the next upgrade cycle. They lock your projects into proprietary formats.

DAWN was built to bypass the gatekeepers.

- **No installers required.** Unzip it. Run it. You are done. Packages exist if you want them. Nothing in the program requires one.
- **No telemetry.** Web wrappers, usage reporting, and network tracking are compiled out. DAWN goes online only if you ask the built-in plugin hub to download an open-source instrument.
- **Open standards.** DAWN exports and imports the open [`.dawproject`](https://github.com/bitwig/dawproject) format natively. Your music belongs to you, not to us.
- **The folder is the studio.** Unzip DAWN onto a USB drive. A `DAWN_PORTABLE.txt` flag beside the executable keeps every preference, mapping, download, and project inside `DAWN_DATA/`. Eject the drive and the host computer has nothing new to remember.

If a tool cannot survive without calling home, it was never a tool. It was a lease.

## Features

- **Lock-free audio engine.** The audio callback never locks, never allocates, and never touches the disk. Plugin tails fade out on the audio thread without a spike and without a click.
- **Dual-workflow UI.** A 720 px responsive shell. Wider than that, the session stays up and the device rack stays along the bottom. Narrower, one page and a thumb bar.
- **Universal input matrix.** Play the built-in analog synth and drum rack from a MIDI controller, the computer keyboard, or a gamepad. Map a trigger to a filter.
- **Built-in visualizers.** A lock-free phosphor oscilloscope and an FFT on the master bus. If the meter queue is full, samples are dropped. The music is not.
- **Open-source plugin hub.** Browse, download, and install free VST3 and CLAP plugins from inside the app. In portable mode they install into `DAWN_DATA/Plugins`, not into the host's plug-in folder.
- **Comping that the callback never sees.** Lanes stay on the message thread. The snapshot receives the flattened take.

## Portable mode

Unzip, then you are done. There is no installer and no extra hardware.

1. Download the latest `.zip` for your OS from the Releases tab.
2. Unzip that folder onto a USB drive, an external SSD, or the desktop.
3. Run `DAWN.exe`, `DAWN.app`, or the Linux binary.

The folder already contains `DAWN_PORTABLE.txt`, so DAWN writes device setup, input maps, hub downloads, and projects into `DAWN_DATA/` next to the executable. Eject the drive, plug it into another machine, and the studio is the same folder.

`DAWN_DATA/demo.dawn` is the first-run session: **PUBLIC ENEMY, PRIVATE NETWORK**, 142 BPM, F minor. RIOT is already launched. IDLE and OVERLOAD are sitting on the same tracks, not launched. The chop clip is comped. The note on that clip says to use the rushed word.

Leave the flag out and DAWN falls back to `%APPDATA%\DAWN`, `~/Library/Application Support/DAWN`, or `~/.config/dawn`.

## Compiling from source

JUCE 8 and CMake 3.22 or newer. The first configure fetches JUCE.

```bash
git clone https://github.com/meepius/DAWN.git
cd DAWN
cmake -S DAWN -B build -DDAWN_WITH_JUCE=ON -DCMAKE_BUILD_TYPE=Release
cmake --build build --config Release --parallel
```

The engine tests do not need JUCE:

```bash
cmake -S DAWN -B build
cmake --build build --target dawn_tests
./build/dawn_tests
```

Packages:

```bash
cmake --build build --config Release --target package
cmake --build build --target dawn_appimage   # Linux, if appimagetool is installed
```

CPack produces a Windows NSIS `.exe` and a WiX `.msi`, a macOS `.dmg`, or a Linux `.tar.gz`. The install layout puts `DAWN_PORTABLE.txt` and `DAWN_DATA/demo.dawn` next to the application.

On Linux, configure needs `libasound2-dev`, `libjack-jackd2-dev`, `libx11-dev`, `libxext-dev`, `libxrandr-dev`, `libxcursor-dev`, `libxinerama-dev`, `libxi-dev`, `libfreetype6-dev`, `libglu1-mesa-dev`, and `libgtk-3-dev`.

[.github/workflows/build.yml](.github/workflows/build.yml) builds that on `ubuntu-latest`, `macos-latest`, and `windows-latest` for every push to `main`, every `v*` tag, and every GitHub release. macOS ad-hoc signs the app with `codesign --sign -`.

## License

GNU General Public License v3. See [DAWN/packaging/LICENSE.txt](DAWN/packaging/LICENSE.txt).

No analytics. No crash uploader. No account. If that ever changes, the license still gives you the last version that told the truth.
