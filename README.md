# Enaga

Relaxing white noise generator.

Built in C++23 (targeting C++26), using [JUCE](https://juce.com/) for platform
lifecycle, audio, and UI.

---

## Prerequisites

### All platforms
| Tool | Minimum version | Notes |
|------|-----------------|-------|
| [CMake](https://cmake.org/download/) | 3.22 | Provides `cmake --preset` |
| [Git](https://git-scm.com/) | 2.x | Required by CMake's `FetchContent` |
| C++23-capable compiler | see below | |

### macOS / iOS
| Tool | Notes |
|------|-------|
| Xcode 15+ | Install from the Mac App Store |
| Xcode Command Line Tools | `xcode-select --install` |

### Windows
| Tool | Notes |
|------|-------|
| Visual Studio 2022 | Include the **Desktop development with C++** workload |
| [Ninja](https://ninja-build.org/) | Optional; only needed if you use a Ninja preset |

### Linux
| Tool | Install |
|------|---------|
| GCC 13+ or Clang 17+ | `sudo apt install gcc g++` / `sudo apt install clang` |
| Ninja | `sudo apt install ninja-build` |
| JUCE system dependencies | `sudo apt install libasound2-dev libx11-dev libxext-dev libxinerama-dev libxrandr-dev libxcursor-dev libfreetype6-dev libcurl4-openssl-dev` |

### Android
| Tool | Notes |
|------|-------|
| Android NDK r25c+ | Install via Android Studio SDK Manager or standalone |
| Set `ANDROID_NDK` environment variable | `export ANDROID_NDK=/path/to/ndk` |
| Ninja | `sudo apt install ninja-build` (Linux host) |

---

## Building

Dependencies (JUCE) are fetched automatically by CMake the first time you
configure. An internet connection is required for the first build only.

### macOS

```sh
cmake --preset macos
cmake --build --preset macos
```

The built app bundle is at `build/macos/Enaga_artefacts/Debug/Enaga.app`.

### Linux

```sh
cmake --preset linux
cmake --build --preset linux
```

The built binary is at `build/linux/Enaga_artefacts/Debug/Enaga`.

### Windows

```sh
cmake --preset windows
cmake --build --preset windows
```

The built executable is at
`build\windows\Enaga_artefacts\Debug\Enaga.exe`.

### iOS (requires macOS host)

```sh
cmake --preset ios
```

Open the generated Xcode project to build and deploy to a device or simulator:

```sh
open build/ios/Enaga.xcodeproj
```

### Android (requires NDK)

```sh
export ANDROID_NDK=/path/to/your/ndk
cmake --preset android
cmake --build --preset android
```

---

## Development setup

### Cloning

```sh
git clone https://github.com/jphinspace/Enaga.git
cd Enaga
```

### IDE integration

**CLion / VS Code (CMake Tools)**  
Open the repository root — both IDEs detect `CMakePresets.json` automatically
and populate a preset selector in their UI.

**Xcode (macOS / iOS)**  
```sh
cmake --preset macos   # or ios
open build/macos/Enaga.xcodeproj
```

**Visual Studio 2022 (Windows)**  
Use *File → Open → CMake…* and select the repository root. Visual Studio reads
`CMakePresets.json` directly.

### Bumping the C++ standard to C++26

When compiler support is ready, change one line in `CMakeLists.txt`:

```cmake
set(CMAKE_CXX_STANDARD 26)   # was 23
```

All `TODO:C++26` comments in the source files mark the exact spots that benefit
from C++26 language features (modules, `std::println`, `std::expected`, etc.).

---

## Project structure

```
Enaga/
├── CMakeLists.txt       # Build definition; fetches JUCE via FetchContent
├── CMakePresets.json    # Per-platform configure & build presets
└── Source/
    └── Main.cpp         # Application entry point (JUCEApplication subclass)
```
