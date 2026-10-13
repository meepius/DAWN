#!/bin/sh
# Builds DAWN-*.AppImage from a CMake build tree. Requires appimagetool on PATH.
set -eu
BUILD="${1:-.}"
BIN="$(find "$BUILD" -path '*DAWN_artefacts*' -type f -name DAWN ! -name '*.a' | head -n 1)"
if [ -z "$BIN" ]; then
    echo "DAWN binary not found under $BUILD" >&2
    exit 1
fi
APPDIR="$BUILD/DAWN.AppDir"
rm -rf "$APPDIR"
mkdir -p "$APPDIR/usr/bin"
cp "$BIN" "$APPDIR/usr/bin/DAWN"
chmod +x "$APPDIR/usr/bin/DAWN"
cp "$(dirname "$0")/dawn.desktop" "$APPDIR/DAWN.desktop"
# 1x1 png so appimagetool has an icon.
printf '%s' 'iVBORw0KGgoAAAANSUhEUgAAAAEAAAABCAYAAAAfFcSJAAAADUlEQVR42mNk+M9QDwADhgGAWjR9awAAAABJRU5ErkJggg==' | base64 -d > "$APPDIR/dawn.png" 2>/dev/null || \
    printf '%s' 'iVBORw0KGgoAAAANSUhEUgAAAAEAAAABCAYAAAAfFcSJAAAADUlEQVR42mNk+M9QDwADhgGAWjR9awAAAABJRU5ErkJggg==' | openssl base64 -d -A > "$APPDIR/dawn.png"
cat > "$APPDIR/AppRun" << 'EOF'
#!/bin/sh
HERE="$(dirname "$(readlink -f "$0")")"
exec "$HERE/usr/bin/DAWN" "$@"
EOF
chmod +x "$APPDIR/AppRun"
if ! command -v appimagetool >/dev/null 2>&1; then
    echo "appimagetool is not installed" >&2
    exit 1
fi
cd "$BUILD"
ARCH="$(uname -m)" appimagetool "$APPDIR" "DAWN-1.0.0-Linux.AppImage"
