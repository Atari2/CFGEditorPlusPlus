#!/bin/bash
# build_qt_static.sh
#
# Downloads and builds a minimal static Qt installation for CFGEditorPlusPlus.
# The project only uses Qt::Widgets, so only qtbase is needed (Core + Gui + Widgets).
#
# Usage:
#   ./build_qt_static.sh [install_prefix]
#
# Default install prefix: <script_dir>/6.11.0/static
#
# After a successful build, compile the project with:
#   cmake -S . -B build \
#       -DCFGEDITOR_BUILD_STATIC=ON \
#       -DSTATIC_QT_DIR=<install_prefix> \
#       -DCMAKE_BUILD_TYPE=Release
#   cmake --build build --parallel

set -euo pipefail

QT_VERSION="6.11.0"
SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"

QT_SRC_PARENT="$SCRIPT_DIR/$QT_VERSION/Src"
QT_SRC_DIR="$QT_SRC_PARENT/qtbase"
BUILD_DIR="$QT_SRC_PARENT/qtbase-build"
QT_INSTALL_DIR="${1:-$SCRIPT_DIR/$QT_VERSION/static}"

VENV_AQT="$SCRIPT_DIR/.venv/bin/aqt"
JOBS="${JOBS:-$(nproc)}"

info()  { echo "[INFO]  $*"; }
error() { echo "[ERROR] $*" >&2; exit 1; }

check_cmd() {
    command -v "$1" >/dev/null 2>&1 || error "'$1' not found. Please install it before running this script."
}

info "Checking dependencies..."
check_cmd cmake
check_cmd ninja
check_cmd perl       # required by Qt's build machinery
check_cmd python3    # required by Qt's configure scripts

[ -x "$VENV_AQT" ] || error "aqtinstall not found at $VENV_AQT. Create the venv and run: pip install aqtinstall"

info "All dependencies found."
echo ""
info "Qt version : $QT_VERSION"
info "Source dir : $QT_SRC_DIR"
info "Build dir  : $BUILD_DIR"
info "Install dir: $QT_INSTALL_DIR"
info "Parallel   : $JOBS jobs"
echo ""

if [ ! -d "$QT_SRC_DIR" ]; then
    info "Downloading Qt $QT_VERSION qtbase sources via aqtinstall..."
    "$VENV_AQT" install-src linux "$QT_VERSION" \
        --outputdir "$SCRIPT_DIR" \
        --archives qtbase
    info "Download complete."
else
    info "Sources already present, skipping download."
fi
echo ""

info "Configuring..."

# Features intentionally kept:
#   gui        - required by Widgets
#   widgets    - the only Qt module the project uses
#   opengl     - used by QPainter/compositor internally; leave enabled
#   printsupport - small cost, avoids linker errors if Widgets references it
#   xml        - part of Core, negligible cost

cmake -S "$QT_SRC_DIR" -B "$BUILD_DIR" \
    -G Ninja \
    -DCMAKE_BUILD_TYPE=Release \
    -DCMAKE_INSTALL_PREFIX="$QT_INSTALL_DIR" \
    -DBUILD_SHARED_LIBS=OFF \
    -DQT_BUILD_EXAMPLES=OFF \
    -DQT_BUILD_TESTS=OFF \
    -DQT_BUILD_DOCS=OFF \
    -DQT_FEATURE_sql=OFF \
    -DQT_FEATURE_dbus=OFF \
    -DQT_FEATURE_concurrent=OFF \
    -DQT_FEATURE_testlib=OFF \
    -DQT_FEATURE_network=OFF \
    -DQT_FEATURE_fontconfig=ON

echo ""

info "Building (this will take a while)..."
cmake --build "$BUILD_DIR" --parallel "$JOBS"
echo ""

info "Installing to $QT_INSTALL_DIR..."
cmake --install "$BUILD_DIR"
echo ""

info "Qt $QT_VERSION static build complete!"
echo ""
echo "Build CFGEditorPlusPlus with:"
echo ""
echo "  cmake -S \"$SCRIPT_DIR\" -B \"$SCRIPT_DIR/build\" \\"
echo "      -DCFGEDITOR_BUILD_STATIC=ON \\"
echo "      -DSTATIC_QT_DIR=\"$QT_INSTALL_DIR\" \\"
echo "      -DCMAKE_BUILD_TYPE=Release"
echo "  cmake --build \"$SCRIPT_DIR/build\" --parallel"
