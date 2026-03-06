# Enaga – Organization and Refactoring Plan (pt 1)

This document records the plan agreed before any code changes are made.
It covers: (1) best-practice gaps found in the current code, and (2) concrete
organization/refactoring improvements that can be applied without changing
any observable behaviour.

---

## 1. Best-Practice Gaps

### 1.1 No CI/CD pipeline
`.github/` contains only `copilot-instructions.md`.  There is no
`workflows/` directory, so nothing automatically builds or validates the
project on every push or pull request.

**Expected:** At minimum a Linux Ninja build triggered on every push to `main`
and on every PR (using the `linux` preset).

---

### 1.2 No code-formatting standard
There is no `.clang-format` file.  Contributors must infer the style by eye
from existing sources, and an automated formatter cannot be added to CI without
one.

**Expected:** A `.clang-format` at the repository root that encodes the
existing style (column limit 100, braces/indent conventions, etc.).

---

### 1.3 Only Debug CMake presets — no Release preset
`CMakePresets.json` defines one build configuration per platform, hardcoded to
`CMAKE_BUILD_TYPE=Debug`.  Shipping a `Release` build currently requires
manual CMake invocation.

**Expected:** Paired `*-release` configure + build presets for each platform
so that `cmake --preset linux-release && cmake --build --preset linux-release`
produces an optimised binary.

---

### 1.4 README project-structure section is out of date
`README.md` still shows only `Main.cpp` under `Source/`, even though the
directory now contains 15 files across audio, UI, and platform concerns.

**Expected:** The project-structure section reflects the actual file tree and
describes each major file/group in one sentence.

---

### 1.5 Full class implementations inside header files
Three classes are defined entirely in `.h` files despite having non-trivial
bodies.  This is inconsistent with the convention established by
`MainComponent` and `WhiteNoiseAudioSource` (each split into `.h` + `.cpp`),
and it increases incremental compile times for every TU that includes those
headers.

| File | Non-trivial implementation |
|------|---------------------------|
| `LfoComponent.h` (242 lines) | Full `juce::Component` with 8 setup helpers, sync/apply logic, and all child-control callbacks |
| `EnagaLookAndFeel.h` (86 lines) | Constructor with 20+ colour assignments; `drawLinearSlider` override |
| `PlayButton.h` (51 lines) | `paintButton` override with path / fill logic |

**Expected:** Each class is split into a `.h` (interface only) + `.cpp`
(implementation), matching the rest of the project.

---

### 1.6 No test infrastructure
`copilot-instructions.md` specifies CTest for unit tests, but there is no
`tests/` directory, no `CMakeLists.txt` test block, and no registered test
targets.

**Expected:** At least a skeleton `tests/` directory with a `CMakeLists.txt`
that adds a single smoke test (e.g. constructing `WhiteNoiseAudioSource` and
calling `prepareToPlay` / `getNextAudioBlock` offline) so the testing path is
established and CI can run it.

---

## 2. Organization Improvements

### 2.1 Flat `Source/` directory — no subdirectory structure
All 15 source files live in one flat directory.  As the project grows this
becomes hard to navigate.  The files fall naturally into three concerns:

| Proposed subdirectory | Files |
|-----------------------|-------|
| `Source/audio/` | `LfoEngine.h`, `LfoMode.h`, `NoiseType.h`, `WhiteNoiseAudioSource.h`, `WhiteNoiseAudioSource.cpp` |
| `Source/audio/generators/` | `NoiseGenerator.h`, `WhiteNoiseGenerator.h`, `PinkNoiseGenerator.h`, `BrownNoiseGenerator.h`, `GreyNoiseGenerator.h` |
| `Source/ui/` | `EnagaLookAndFeel.h`, `PlayButton.h`, `LfoComponent.h`, `MainComponent.h`, `MainComponent.cpp` |
| `Source/platform/` | `iOSVolumeView.h`, `iOSVolumeView.mm` |

`Main.cpp` stays at `Source/` root as the application entry point.

Subdirectory names are lowercase.  `Source/` is kept as-is (it is existing
code), but new subdirectories follow lowercase convention which is standard
for C++ source trees on Linux/Unix and avoids case-sensitivity surprises
across platforms (e.g. macOS HFS+ vs Linux ext4).

The `generators/` subdirectory keeps the interface (`NoiseGenerator.h`) and all
four concrete implementations in one place, making it easy to add a fifth
noise type without cluttering `Source/audio/`.

**Impact on `CMakeLists.txt`:** `target_sources` paths and the iOS conditional
source must be updated to use the new subdirectory paths.  No other CMake
changes are needed.

---

### 2.2 Scattered colour literals
The four theme colours (`0xff1a1a1a` background, `0xff4fc3f7` accent,
`0xffe0e0e0` text, `0xff222222` panel) are defined as local variables inside
`EnagaLookAndFeel`'s constructor but then re-typed as magic literals in
`Main.cpp` (window background) and `MainComponent.cpp` (fill/paint calls).

**Proposed fix:** Promote the four colour values to `public static constexpr`
members of `EnagaLookAndFeel` so that `Main.cpp` and `MainComponent.cpp` can
reference them by name (`EnagaLookAndFeel::kBackground`, etc.) rather than
repeating the hex literals.

---

## 3. Refactoring Improvements

### 3.1 `WhiteNoiseAudioSource` is a misleading name
The class now generates White, Pink, Brown, and Grey noise.  "White" in the
class name implies single-type output, but that has not been true for some
time.

**Proposed rename:** `WhiteNoiseAudioSource` → `NoiseAudioSource`
(class, `.h`, `.cpp`, and all references in `Main.cpp`).

---

### 3.2 Audio-domain enums live in the wrong headers
`NoiseType` is defined in `WhiteNoiseAudioSource.h`, but it is a property of
the *noise generator* abstraction, not of the audio-source layer.  `LfoMode`
(defined in `LfoEngine.h`) follows the same anti-pattern: it describes a
parameter of the audio engine but is also consumed directly by `LfoComponent`
in the UI layer.

**Proposed fix:** Give each enum its own minimal header in `Source/audio/`:

| New header | Contains |
|------------|----------|
| `Source/audio/NoiseType.h` | `enum class NoiseType { White, Pink, Brown, Grey };` |
| `Source/audio/LfoMode.h`   | `enum class LfoMode { Disabled, Volume, Filter, Both };` |

Keeping the enums in separate files means a consumer (e.g. `LfoComponent.h`)
can include only the enum it needs without pulling in the full engine header,
and adding a new enum type in the future requires only a new file rather than
touching a shared one.

---

### 3.3 Excessive constructor arity on `MainWindow` and `MainComponent`
`MainWindow` takes 8 parameters (name + 7 callbacks); `MainComponent` takes 7
callbacks individually.  Every time a new audio parameter is added, both
constructors must grow by one parameter and every call site must be updated.

**Why so many callbacks?**  The callbacks (`AudioToggleCallback`,
`AudioFilterCallback`, `AudioGainCallback`, etc.) are the current mechanism for
decoupling the UI layer from the audio layer.  `MainComponent` has no direct
reference to `WhiteNoiseAudioSource` or `juce::AudioDeviceManager`; instead,
`EnagaApplication` captures `this` in lambdas and passes them in.  This keeps
the UI component independently testable and avoids a circular dependency between
UI and audio headers.  The *mechanism* is sound; the *interface* (one parameter
per callback) is the problem.

**Proposed fix:** Introduce a `MainComponent::AudioCallbacks` plain struct
holding all seven `std::function` members.  `MainComponent` takes one
`AudioCallbacks` by value; `MainWindow` takes the struct and forwards it.
This reduces arity from 7/8 to 1/2, and new callbacks only require changing
the struct definition rather than every call site.

---

## 4. Summary Checklist (execution order)

These are ordered so that each step leaves a compilable repository.  They are
**not yet applied**; this document records the plan only.

- [ ] **4.1** Add `.clang-format` at the repository root.
- [ ] **4.2** Add `*-release` CMake presets (configure + build) for each platform.
- [ ] **4.3** Create `Source/audio/NoiseType.h` and `Source/audio/LfoMode.h`; remove the enums from their current headers and update all includers.
- [ ] **4.4** Introduce `MainComponent::AudioCallbacks` struct; collapse constructor arities (before the rename so call-sites need only one update pass).
- [ ] **4.5** Rename `WhiteNoiseAudioSource` → `NoiseAudioSource` (class + files + references in `Main.cpp`).
- [ ] **4.6** Promote theme colours to `static constexpr` in `EnagaLookAndFeel`; remove duplicate literals.
- [ ] **4.7** Split `LfoComponent.h`, `EnagaLookAndFeel.h`, and `PlayButton.h` into `.h`/`.cpp` pairs.
- [ ] **4.8** Reorganize `Source/` into `audio/generators/`, `audio/`, `ui/`, and `platform/` subdirectories; update `CMakeLists.txt`.
- [ ] **4.9** Update `README.md` project-structure section to reflect the new layout.
- [ ] **4.10** Add skeleton `tests/` directory with a CTest-registered smoke test.
- [ ] **4.11** Add a GitHub Actions CI workflow (Linux build + test on push/PR).
