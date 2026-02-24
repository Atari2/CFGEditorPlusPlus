#!/bin/bash
# build_appimage.sh
#
# Builds CFGEditorPlusPlus and packages it into an AppImage using
# linuxdeploy + linuxdeploy-plugin-qt (dynamic Qt build, Qt bundled inside).
#
# Usage:
#   ./build_appimage.sh
#
# Output: CFGEditorPlusPlus-x86_64.AppImage in the project root.

set -euo pipefail

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
QT_DIR="$SCRIPT_DIR/6.11.0/gcc_64"
BUILD_DIR="$SCRIPT_DIR/build-appimage"
APPDIR="$SCRIPT_DIR/AppDir"

LINUXDEPLOY="$SCRIPT_DIR/linuxdeploy-x86_64.AppImage"
LINUXDEPLOY_QT="$SCRIPT_DIR/linuxdeploy-plugin-qt-x86_64.AppImage"

JOBS="${JOBS:-$(nproc)}"

info()  { echo "[INFO]  $*"; }
error() { echo "[ERROR] $*" >&2; exit 1; }

[ -d "$QT_DIR" ] || error "Dynamic Qt not found at $QT_DIR. Install it with: .venv/bin/aqt install-qt linux desktop 6.11.0"
command -v cmake  >/dev/null 2>&1 || error "cmake not found"
command -v ninja  >/dev/null 2>&1 || error "ninja not found"

if [ ! -f "$LINUXDEPLOY" ]; then
    info "Downloading linuxdeploy..."
    wget -q --show-progress \
        "https://github.com/linuxdeploy/linuxdeploy/releases/download/continuous/linuxdeploy-x86_64.AppImage" \
        -O "$LINUXDEPLOY"
    chmod +x "$LINUXDEPLOY"
fi

if [ ! -f "$LINUXDEPLOY_QT" ]; then
    info "Downloading linuxdeploy-plugin-qt..."
    wget -q --show-progress \
        "https://github.com/linuxdeploy/linuxdeploy-plugin-qt/releases/download/continuous/linuxdeploy-plugin-qt-x86_64.AppImage" \
        -O "$LINUXDEPLOY_QT"
    chmod +x "$LINUXDEPLOY_QT"
fi

info "Configuring (dynamic Qt $QT_DIR)..."
cmake -S "$SCRIPT_DIR" -B "$BUILD_DIR" \
    -G Ninja \
    -DCMAKE_BUILD_TYPE=Release \
    -DCMAKE_PREFIX_PATH="$QT_DIR"

info "Building with $JOBS parallel jobs..."
cmake --build "$BUILD_DIR" --parallel "$JOBS"

info "Packaging AppImage..."
rm -rf "$APPDIR"

export QMAKE="$QT_DIR/bin/qmake"
export OUTPUT="$SCRIPT_DIR/CFGEditorPlusPlus-x86_64.AppImage"

export PATH="$SCRIPT_DIR:$PATH"

"$LINUXDEPLOY" \
    --appdir "$APPDIR" \
    --executable "$BUILD_DIR/CFGEditorPlusPlus" \
    --desktop-file "$SCRIPT_DIR/CFGEditorPlusPlus.desktop" \
    --icon-file "$SCRIPT_DIR/VioletEgg.png" \
    --plugin qt \
    --output appimage

info "Done: $OUTPUT"
