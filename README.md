# CFGEditorPlusPlus

A cross-platform rewrite using C++/Qt graphical editor of the classic CFGEditor for the Pixi/SpriteToolSuperDelux sprite inserter tool for SMW. All info and documentation can be found [here](https://github.com/JackTheSpades/SpriteToolSuperDelux?tab=readme-ov-file#cfg-files-and-the-new-cfg-editor).

---

## Table of Contents

- [Prerequisites](#prerequisites)
  - [Linux](#linux-prerequisites)
  - [Windows](#windows-prerequisites)
- [Building](#building)
  - [Linux - dynamic build (development)](#linux--dynamic-build-development)
  - [Linux - static build](#linux--static-build)
  - [Windows - dynamic build (for installer)](#windows--dynamic-build-for-installer)
  - [Windows - static build (standalone .exe)](#windows--static-build-standalone-exe)
- [Packaging](#packaging)
  - [Linux - AppImage](#linux--appimage)
  - [Windows - installer (ICFGEditor.exe)](#windows--installer-icfgeditorexe)
- [CI/CD](#cicd)

---

## Prerequisites

### Linux prerequisites

Install the following with your package manager. On Ubuntu/Debian:

```bash
sudo apt install \
  build-essential cmake ninja-build perl python3 pkg-config \
  libx11-dev libx11-xcb-dev \
  libxcb1-dev libxcb-cursor-dev libxcb-icccm4-dev libxcb-image0-dev \
  libxcb-keysyms1-dev libxcb-randr0-dev libxcb-render0-dev \
  libxcb-render-util0-dev libxcb-shape0-dev libxcb-shm0-dev \
  libxcb-sync-dev libxcb-util-dev libxcb-xfixes0-dev \
  libxcb-xinerama0-dev libxcb-xkb-dev \
  libxkbcommon-dev libxkbcommon-x11-dev \
  libfontconfig-dev libfreetype-dev \
  libgl-dev libegl-dev
```

You also need a Qt 6 installation. The recommended way is via [aqtinstall](https://github.com/miurahr/aqtinstall):

```bash
python3 -m venv .venv
source .venv/bin/activate
pip install aqtinstall

# Install the dynamic Qt (needed for development builds and AppImage packaging)
.venv/bin/aqt install-qt linux desktop 6.11.0 -O .
```

This places the Qt installation at `6.11.0/gcc_64/`.

### Windows prerequisites

- **Visual Studio 2022** with the "Desktop development with C++" workload (MSVC v143 toolchain)
- **CMake** ≥ 3.5 (bundled with Visual Studio or from cmake.org)
- **Ninja** (bundled with Visual Studio)
- **vcpkg** (available at `C:/vcpkg` - pre-installed on GitHub Actions runners)
- **Qt 6** - either:
  - A standard Qt Online Installer installation to `C:\Qt\6.x\msvc2022_64\` (for dynamic builds)
  - Or the pre-built static Qt archive (for static builds - see below)

---

## Building

### Linux - dynamic build (development)

A standard dynamic build links against the Qt shared libraries from `6.11.0/gcc_64/`:

```bash
cmake -S . -B build \
    -G Ninja \
    -DCMAKE_BUILD_TYPE=Release \
    -DCMAKE_PREFIX_PATH="$(pwd)/6.11.0/gcc_64"

cmake --build build --parallel
```

The binary is at `build/CFGEditorPlusPlus`. It requires the Qt `.so` files to be present
(either via the `6.11.0/gcc_64/` tree or a system Qt installation).

### Linux - static build

A static build produces a single fully self-contained binary. It requires building Qt
from source first.

**Step 1 - build static Qt** (one-time, takes ~30–60 minutes):

```bash
# Make sure all prerequisites above are installed, then:
./build_qt_static.sh
# Qt will be installed to 6.11.0/static/
```

The script downloads the Qt 6.11.0 sources automatically via aqtinstall if they are not
already present.

**Step 2 - build the application:**

```bash
cmake -S . -B build \
    -DCFGEDITOR_BUILD_STATIC=ON \
    -DSTATIC_QT_DIR="$(pwd)/6.11.0/static" \
    -DCMAKE_BUILD_TYPE=Release

cmake --build build --parallel
```

> **Note:** `CFGEDITOR_BUILD_STATIC=ON` requires `CMAKE_BUILD_TYPE=Release`
> (or `RelWithDebInfo` / `MinSizeRel`). A debug static build is intentionally rejected.

The binary is at `build/CFGEditorPlusPlus` and has no Qt runtime dependencies.

### Windows - dynamic build (for installer)

This produces `CFGEditorPlusPlus.exe` plus the required Qt DLLs, which are then bundled
into the installer package. Requires a standard Qt Online Installer installation under
`C:\Qt\`.

```powershell
.\build_script.ps1
```

The script automatically locates the newest Qt 6 installation at `C:\Qt\6.*\msvc2022_64\`,
sets up the MSVC environment via `vswhere`, and runs CMake + Ninja.

Output: `build\CFGEditorPlusPlus.exe` and the Qt DLLs copied alongside it by windeployqt.

### Windows - static build (standalone .exe)

This produces a single `.exe` with no external DLL dependencies. It requires the
pre-built static Qt archive.

**Step 1 - obtain the static Qt build:**

Download and extract `Qt6Static.7z` from `www.atarismwc.com/Qt6Static.7z` into a
directory named `Qt6Static` at the repository root (or anywhere - pass its path to
CMake). This is the same archive used by the CI workflow.

**Step 2 - install vcpkg dependencies:**

```powershell
vcpkg install zlib:x64-windows-static
```

**Step 3 - configure and build:**

```powershell
cmake -S . -B build `
    -DCMAKE_BUILD_TYPE=Release `
    -DCMAKE_TOOLCHAIN_FILE=C:/vcpkg/scripts/buildsystems/vcpkg.cmake `
    -DSTATIC_QT_DIR="$PWD/Qt6Static" `
    -DCFGEDITOR_BUILD_STATIC=ON

cmake --build build --config Release
```

Output: `build\Release\CFGEditorPlusPlus.exe` - a fully standalone executable.

---

## Packaging

### Linux - AppImage

The `build_appimage.sh` script produces a portable `CFGEditorPlusPlus-x86_64.AppImage`
using [linuxdeploy](https://github.com/linuxdeploy/linuxdeploy) and its Qt plugin.
It uses the **dynamic** Qt build and bundles all Qt libraries inside the AppImage.

```bash
./build_appimage.sh
```

The script will:
1. Download `linuxdeploy-x86_64.AppImage` and `linuxdeploy-plugin-qt-x86_64.AppImage`
   automatically if they are not present in the repository root.
2. Build the application against `6.11.0/gcc_64/` (dynamic Qt).
3. Package everything into `CFGEditorPlusPlus-x86_64.AppImage`.

**AppImage portability:**

| Requirement | Minimum version |
|---|---|
| Architecture | x86_64 |
| Linux kernel | 2.6.32 |
| glibc | 2.34 (Ubuntu 22.04 / Fedora 35 / Debian 12 or newer) |
| X11 or XWayland | required (no native Wayland support) |
| Mesa / GPU driver | required (OpenGL) |
| fontconfig + freetype | required (font rendering) |

The AppImage bundles Qt 6 and all xcb extension libraries. Everything else (glibc,
libstdc++, OpenGL/EGL, libX11, fontconfig, freetype) must be present on the host.

### Windows - installer (ICFGEditor.exe)

The Windows installer is produced by the C# project in `packaging/windows/`.

**How it works:**

1. `build_script.ps1` (in this repo) builds the dynamic Qt release.
2. `zip_script.py` packages the output into `CFGEditor.zip`:
   - `CFGEditorPlusPlus.exe`
   - `Qt6*.dll` (core Qt libraries)
   - `imageformats\*.dll`, `platforms\*.dll`, `styles\*.dll` (Qt plugins)
3. `CFGEditor.zip` is copied to `packaging/windows/Properties/CFGEditor.zip`.
4. MSBuild compiles `packaging/windows/CFGEditorInstaller.csproj`, embedding the zip as a
   resource into `ICFGEditor.exe`.
5. When a user runs `ICFGEditor.exe`, it extracts the ZIP next to itself and creates a
   shortcut `CFGEditor.lnk` pointing to the extracted `CFGEditorPlusPlus.exe`.

To build the installer locally:

```powershell
.\build_script.ps1
python zip_script.py build/Release
Copy-Item CFGEditor.zip packaging\windows\Properties\CFGEditor.zip
msbuild packaging\windows\CFGEditorInstaller.csproj /p:Configuration=Release
```

---

## CI/CD

### GitHub Actions (`.github/workflows/cmake.yml`)

Triggered on every push to `master`, pull requests targeting `master`, and on version
tags (`v*`). All three artifacts are produced on every run; only tag pushes create a
GitHub Release.

**Jobs:**

| Job | Runner | What it produces |
|---|---|---|
| `build-windows` | `windows-latest` | `CFGEditorPlusPlus.exe` (static, no DLL deps) + `ICFGEditor.exe` (installer) |
| `build-linux` | `ubuntu-22.04` | `CFGEditorPlusPlus-x86_64.AppImage` |
| `release` | `ubuntu-latest` | GitHub Release with all three files (tag pushes only) |

**Stable download URLs** (updated automatically on each tagged release):

| File | URL |
|---|---|
| Windows static exe | `https://github.com/Atari2/CFGEditorPlusPlus/releases/latest/download/CFGEditorPlusPlus.exe` |
| Windows installer | `https://github.com/Atari2/CFGEditorPlusPlus/releases/latest/download/ICFGEditor.exe` |
| Linux AppImage | `https://github.com/Atari2/CFGEditorPlusPlus/releases/latest/download/CFGEditorPlusPlus-x86_64.AppImage` |
