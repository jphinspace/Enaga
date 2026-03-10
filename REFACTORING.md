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
directory now contains 16 files across audio, UI, and platform concerns.

**Expected:** The project-structure section reflects the actual file tree and
describes each major file/group in one sentence.

---

### 1.5 Full class implementations inside header files
Eight classes are defined entirely in `.h` files despite having non-trivial
bodies.  This is inconsistent with the convention established by
`MainComponent` and `WhiteNoiseAudioSource` (each split into `.h` + `.cpp`),
and it increases incremental compile times for every TU that includes those
headers.

| File | Non-trivial implementation |
|------|---------------------------|
| `LfoComponent.h` (242 lines) | Full `juce::Component` with 6 setup helpers, sync/apply logic, and all child-control callbacks |
| `LfoEngine.h` (130 lines) | Full audio-thread `tick()` algorithm and all setters/getters inline |
| `EnagaLookAndFeel.h` (86 lines) | Constructor with 20+ colour assignments; `drawLinearSlider` override |
| `GreyNoiseGenerator.h` (68 lines) | Three-stage IIR filter chain with `prepare`, `reset`, and `nextSample` |
| `PinkNoiseGenerator.h` (55 lines) | Seven-tap parallel filter (Kellett algorithm) with `prepare`, `reset`, and `nextSample` |
| `PlayButton.h` (51 lines) | `paintButton` override with path / fill logic |
| `BrownNoiseGenerator.h` (39 lines) | Leaky integrator with `prepare`, `reset`, and `nextSample` |
| `WhiteNoiseGenerator.h` (30 lines) | `nextSample` inline |

**Expected:** Each class is split into a `.h` (interface only) + `.cpp`
(implementation), matching the rest of the project.  `LfoEngine` is the
highest-priority case because its `tick()` implementation is an audio-thread
algorithm that should not be inlined into every TU that pulls in the header.

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

### 1.7 Version number has multiple sources of truth
The project version `0.1.0` appears in three separate places and must be kept
in sync manually:

1. `CMakeLists.txt` → `project(Enaga VERSION 0.1.0 …)`
2. `CMakeLists.txt` → `juce_add_gui_app(Enaga … VERSION "0.1.0")`
3. `Main.cpp` → `getApplicationVersion()` returns the string literal `"0.1.0"`

`CMakeLists.txt` already defines `JUCE_APPLICATION_VERSION_STRING` as a
compile-time macro, but `Main.cpp` ignores it.

**Expected:**
- `juce_add_gui_app` uses `"${PROJECT_VERSION}"` instead of a literal so it
  automatically tracks the `project()` declaration.
- `getApplicationVersion()` returns
  `juce::String(JUCE_APPLICATION_VERSION_STRING)` so the runtime value is
  always driven by the build system.
- Similarly, `getApplicationName()` should return
  `juce::String(JUCE_APPLICATION_NAME_STRING)` instead of the literal
  `"Enaga"`.

---

### 1.8 `CMAKE_EXPORT_COMPILE_COMMANDS` not enabled
`CMakeLists.txt` does not set `CMAKE_EXPORT_COMPILE_COMMANDS ON`.  Without a
`compile_commands.json` file, the following tooling cannot locate include paths
or resolve headers automatically:

- **clangd** / **clang-tidy** (IDE language server and static analysis)
- **VS Code CMake Tools** and **CLion** header resolution
- **include-what-you-use** (IWYU)

On Ninja generators (Linux, Android) this is a one-line fix.  Xcode and VS
generators produce a `compile_commands.json` equivalent through their own
mechanisms but benefit from having it explicit.

**Expected:** Add `set(CMAKE_EXPORT_COMPILE_COMMANDS ON)` near the top of
`CMakeLists.txt`, or set `"CMAKE_EXPORT_COMPILE_COMMANDS": "ON"` in the `base`
configure preset in `CMakePresets.json`.

---

### 1.9 No `.clang-tidy` configuration
`.clang-format` (§1.2) can only enforce whitespace and layout.  `.clang-tidy`
enforces semantic code conventions and catches bugs that the compiler does not.
The project's own stated conventions (e.g. `[[nodiscard]]` on value-returning
functions, no raw `new`, `final` on leaf classes) cannot currently be
automatically verified.

**Expected:** A `.clang-tidy` at the repository root enabling at least the
`readability-*`, `modernize-*`, and `cppcoreguidelines-*` check families, with
suppressions scoped to JUCE macro-generated code (via `HeaderFilterRegex`).

---

### 1.10 Raw `new` in `MainWindow` constructor
`MainWindow`'s constructor calls `setContentOwned(new MainComponent(…), true)`.
This is a raw `new` on the heap with no RAII guard between allocation and the
call to `setContentOwned` — violating the project convention "No raw `new` /
`delete`".  If `setContentOwned` or any intermediate call were to throw, the
allocated `MainComponent` would leak.

**Expected:** Use `std::make_unique<MainComponent>(…).release()` to satisfy
both the JUCE API (which requires a raw pointer) and the no-raw-`new`
convention:

```cpp
setContentOwned(std::make_unique<MainComponent>(…).release(), true);
```

---

## 2. Organization Improvements

### 2.1 Flat `Source/` directory — no subdirectory structure
All 16 source files live in one flat directory.  As the project grows this
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
There are five distinct theme colours used across the codebase, all written as
raw hex literals with no shared source of truth:

| Hex value    | Role in `EnagaLookAndFeel` constructor | Duplicated in |
|--------------|----------------------------------------|---------------|
| `0xff1a1a1a` | `bg` — main background                 | `Main.cpp` (window background), `MainComponent.cpp` (`paint()` fill-all) |
| `0xff4fc3f7` | `accent` — highlight / thumb           | `EnagaLookAndFeel.h::drawLinearSlider` (intra-class: out of constructor scope), `MainComponent.cpp::paintImageArea()` |
| `0xffe0e0e0` | `textCol` — primary text               | `MainComponent.cpp::paintImageArea()` |
| `0xff2d2d2d` | `panel` — control backgrounds          | *(not duplicated elsewhere)* |
| `0xff222222` | *(unnamed)* — image area fill          | *(only in `MainComponent.cpp::paintImageArea()`)* |

Note that `0xff2d2d2d` (the `panel` local in `EnagaLookAndFeel`'s constructor)
and `0xff222222` (the image area background in `MainComponent.cpp`) are two
distinct shades of dark grey — they are not the same colour.

The `accent` colour has an additional intra-class duplication: `drawLinearSlider`
inside `EnagaLookAndFeel.h` re-types `0xff4fc3f7` as a raw literal because the
`accent` local variable defined in the constructor is not in scope there.

**Proposed fix:** Promote all five colour values to `public static constexpr`
members of `EnagaLookAndFeel` so that `Main.cpp`, `MainComponent.cpp`, and
`drawLinearSlider` itself can reference them by name
(`EnagaLookAndFeel::kBackground`, `EnagaLookAndFeel::kAccent`, etc.) rather than
repeating hex literals.

---

## 3. Refactoring Improvements

### 3.1 `WhiteNoiseAudioSource` is a misleading name
The class now generates White, Pink, Brown, and Grey noise.  "White" in the
class name implies single-type output, but that has not been true for some
time.

**Proposed rename:** `WhiteNoiseAudioSource` → `NoiseAudioSource`
(class, `.h`, `.cpp`, and all references in `Main.cpp`).

The `CMakeLists.txt` `DESCRIPTION` field also still reads `"Relaxing white
noise generator"`; this should be updated to `"Relaxing noise generator"` in
the same step.

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
- [ ] **4.2** Add `.clang-tidy` at the repository root.
- [ ] **4.3** Enable `CMAKE_EXPORT_COMPILE_COMMANDS` in `CMakeLists.txt` (or the base preset).
- [ ] **4.4** Fix version single source of truth: `juce_add_gui_app` uses `${PROJECT_VERSION}`; `getApplicationName()` and `getApplicationVersion()` use `JUCE_APPLICATION_NAME_STRING` / `JUCE_APPLICATION_VERSION_STRING`.
- [ ] **4.5** Add `*-release` CMake presets (configure + build) for each platform.
- [ ] **4.6** Create `Source/audio/NoiseType.h` and `Source/audio/LfoMode.h`; remove the enums from their current headers and update all includers.
- [ ] **4.7** Introduce `MainComponent::AudioCallbacks` struct; collapse constructor arities; replace raw `new MainComponent(…)` with `std::make_unique<MainComponent>(…).release()`.
- [ ] **4.8** Rename `WhiteNoiseAudioSource` → `NoiseAudioSource` (class + files + references in `Main.cpp`); update `CMakeLists.txt` DESCRIPTION.
- [ ] **4.9** Promote theme colours to `static constexpr` in `EnagaLookAndFeel`; remove duplicate literals.
- [ ] **4.10** Split all header-only implementations into `.h`/`.cpp` pairs: `LfoComponent`, `LfoEngine`, `EnagaLookAndFeel`, `PlayButton`, and the four noise generator classes.
- [ ] **4.11** Reorganize `Source/` into `audio/generators/`, `audio/`, `ui/`, and `platform/` subdirectories; update `CMakeLists.txt`.
- [ ] **4.12** Update `README.md` project-structure section to reflect the new layout.
- [ ] **4.13** Add skeleton `tests/` directory with a CTest-registered smoke test.
- [ ] **4.14** Add a GitHub Actions CI workflow (Linux build + test on push/PR).
