# Enaga

Relaxing noise generator.

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

The built app bundle is at `build/macos/Host/EnagaHost_artefacts/Debug/Enaga.app`.

The VST3 plugin is at `build/macos/Plugin/EnagaPlugin_artefacts/Debug/VST3/Enaga.vst3`.

### Linux

```sh
cmake --preset linux
cmake --build --preset linux
```

The standalone host binary is at `build/linux/Host/EnagaHost_artefacts/Debug/Enaga`.

The VST3 plugin is at
`build/linux/Plugin/EnagaPlugin_artefacts/Debug/VST3/Enaga.vst3/`.

### Windows

```sh
cmake --preset windows
cmake --build --preset windows
```

The standalone host executable is at
`build\windows\Host\EnagaHost_artefacts\Debug\Enaga.exe`.

The VST3 plugin is at
`build\windows\Plugin\EnagaPlugin_artefacts\Debug\VST3\Enaga.vst3\`.

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

The codebase is split into two components that can live in this repo and be
split into separate repos later:

```
Enaga/
├── .clang-format              # Code style configuration (100-col, Allman)
├── .clang-tidy                # Static analysis checks (readability-*, modernize-*, cppcoreguidelines-*)
├── CMakeLists.txt             # Top-level build; fetches JUCE, includes Plugin/ and Host/
├── CMakePresets.json          # Per-platform Debug + Release configure/build presets
├── tests/                     # CTest-registered unit tests
│   ├── CMakeLists.txt
│   └── SmokeTest.cpp          # Smoke test: constructs NoiseAudioSource and exercises the audio path
│
├── Plugin/                    # Component 1 — Enaga VST/AU plugin (self-contained; can be split out)
│   ├── CMakeLists.txt         # juce_add_plugin: builds VST3 and AU formats
│   └── Source/
│       ├── PluginProcessor.h/.cpp  # AudioProcessor wrapping NoiseAudioSource
│       ├── PluginEditor.h/.cpp     # AudioProcessorEditor wrapping MainComponent
│       ├── PluginEntry.cpp         # createPluginFilter() factory (plugin binary only)
│       ├── audio/             # Audio-domain classes and enumerations
│       │   ├── LfoEngine.h/.cpp    # Sine-wave LFO oscillator with lock-free parameter updates
│       │   ├── LfoMode.h           # enum class LfoMode (Disabled / Volume / Filter / Both)
│       │   ├── NoiseAudioSource.h/.cpp  # JUCE AudioSource: LP filter, gain, fade, LFO
│       │   ├── NoiseType.h         # enum class NoiseType (White / Pink / Brown / Grey)
│       │   └── generators/         # Per-sample noise generator implementations
│       │       ├── NoiseGenerator.h            # Abstract base interface
│       │       ├── WhiteNoiseGenerator.h/.cpp  # Flat spectrum (PRNG)
│       │       ├── PinkNoiseGenerator.h/.cpp   # 1/f spectrum (Kellett parallel filter)
│       │       ├── BrownNoiseGenerator.h/.cpp  # 1/f² spectrum (leaky integrator)
│       │       └── GreyNoiseGenerator.h/.cpp   # Perceptually flat (inverse A-weighting)
│       ├── ui/                # UI-domain classes
│       │   ├── EnagaLookAndFeel.h/.cpp  # Dark theme; exposes kBackground/kAccent/etc. constants
│       │   ├── LfoComponent.h/.cpp      # LFO controls panel (mode button, rate/intensity sliders)
│       │   ├── MainComponent.h/.cpp     # Root component: layout, sliders, menus, preset I/O
│       │   └── PlayButton.h/.cpp        # Custom play/stop toggle button
│       └── platform/          # Platform-specific glue code
│           ├── IOSVolumeView.h    # iOS MPVolumeView wrapper interface
│           └── IOSVolumeView.mm   # iOS MPVolumeView wrapper implementation (Objective-C++)
│
└── Host/                      # Component 2 — Generic standalone plugin host (not Enaga-specific)
    ├── CMakeLists.txt         # juce_add_gui_app: builds the standalone application
    └── Source/
        └── Main.cpp           # PluginWindow + EnagaHostApplication; hosts any juce::AudioProcessor
```
