# Copilot Instructions for Enaga

## Project Overview

**Enaga** is a cross-platform, relaxing white noise generator built in **C++23**
(migrating to C++26 over time) using the [JUCE](https://juce.com/) framework for
platform lifecycle, audio device management, and UI.

Supported platforms: **macOS**, **Windows**, **Linux**, **iOS**, **Android**.

---

## Repository Layout

```
Enaga/
├── .github/
│   └── copilot-instructions.md   # This file
├── CMakeLists.txt                # Build definition; fetches JUCE via FetchContent
├── CMakePresets.json             # Per-platform configure & build presets
└── Source/
    └── Main.cpp                  # Application entry point (JUCEApplication subclass)
```

Key classes in `Source/Main.cpp`:

| Class | Role |
|---|---|
| `WhiteNoiseAudioSource` | Produces flat-spectrum (white) noise via `juce::AudioSource` |
| `MainWindow` | Single `juce::DocumentWindow`; closing it exits the app |
| `EnagaApplication` | `juce::JUCEApplication` subclass; owns the audio pipeline |

---

## Build System

CMake 3.22+ with `CMakePresets.json`. JUCE is fetched automatically on first configure.

| Preset | Platform | Generator |
|---|---|---|
| `linux` | Linux | Ninja |
| `macos` | macOS | Xcode |
| `windows` | Windows | Visual Studio 2022 |
| `ios` | iOS (macOS host) | Xcode |
| `android` | Android (NDK required) | Ninja |

```sh
# Example – Linux
cmake --preset linux
cmake --build --preset linux
```

---

## C++ Conventions

- **Standard**: C++23 (`CMAKE_CXX_STANDARD 23`). Extensions are **off**.
- **Migration marker**: Every place that will benefit from a C++26 language
  feature is annotated with a `// TODO:C++26` comment (e.g. modules, `std::println`,
  `std::expected`, `std::span`). When adding new code, follow the same pattern.
- **`[[nodiscard]]`** on all functions that return values the caller must not
  silently discard.
- **`final`** on leaf classes; virtual destructors are not needed when `final`
  is used with `juce::JUCEApplication` or `juce::AudioSource` overrides.
- **No raw `new`/`delete`**: use `std::make_unique` / `std::make_shared`.
- **No application-side gain stage**: output amplitude is always full-scale;
  loudness is governed by the OS/hardware volume control.
- Member declaration order mirrors the dependency chain so that destruction
  (LIFO) is safe without explicit teardown ordering.

---

## Audio Architecture

```
noiseSource (WhiteNoiseAudioSource)
    └─▶ sourcePlayer (juce::AudioSourcePlayer)
            └─▶ deviceManager (juce::AudioDeviceManager)
                    └─▶ hardware output
```

- `deviceManager.initialiseWithDefaultDevices(0, 2)` — stereo output, no input.
- Audio lifetime equals application lifetime (`initialise()` → `shutdown()`).
- Teardown is reverse-order: remove callback → clear source → close device.

---

## JUCE Guidelines

- Use `juce_gui_basics`, `juce_core`, `juce_events`, `juce_audio_basics`, and
  `juce_audio_devices` modules (already linked in `CMakeLists.txt`). Add new
  JUCE modules only when strictly necessary and update `CMakeLists.txt` accordingly.
- Compile-time flags: `JUCE_WEB_BROWSER=0`, `JUCE_USE_CURL=0` — do not enable
  these unless a specific feature requires them.
- JUCE version is pinned to **8.0.4** in `CMakeLists.txt`. Bump deliberately
  and update the comment when changing it.
- Prefer `juce::Logger::writeToLog(...)` over `std::cout` for diagnostic output.

---

## Testing

There is currently no automated test suite. When adding tests:
- Use **CTest** (integrated with CMake) for test registration.
- Prefer unit-testing individual classes (e.g. `WhiteNoiseAudioSource`) in
  isolation from the JUCE event loop where possible.

---

## Style

- Follow the existing file-level Doxygen doc-comment style (`@file`, `@brief`).
- Class-level and member-level comments use `/** … */` blocks.
- Inline explanatory comments use `//` with a space after the slashes.
- Keep lines ≤ 100 characters.
