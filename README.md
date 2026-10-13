# LATTICE

A session and arrangement DAW. This repository builds **portable desktop apps** — no installer, projects stored beside the program so the whole thing can move on a USB stick.

## Download

Open the latest release on GitHub and grab the build for your computer:

- Windows: `LATTICE-1.0.0-windows-portable.exe` — double-click. No setup.
- Linux: `LATTICE-1.0.0-linux.AppImage` — mark executable, then run.
- macOS: unzip `LATTICE-1.0.0-mac-*.zip` and open LATTICE. Unsigned, so the first open is Control-click → Open.

Your songs are kept in a `LATTICE-data` folder next to the Windows exe or the Linux AppImage. Copy that folder with the app if you move machines.

The Windows file is unsigned, so SmartScreen may say it is unrecognized. More info → Run anyway.

## Build it yourself

Node 22.

```bash
npm install
npm install --no-save electron@44.7.1 electron-builder@26.15.3
npm run desktop:bundle
npx electron .
```

Packaging (from the matching operating system):

```bash
npx electron-builder --config electron-builder.yml --win portable
npx electron-builder --config electron-builder.yml --linux AppImage
npx electron-builder --config electron-builder.yml --mac zip
```
