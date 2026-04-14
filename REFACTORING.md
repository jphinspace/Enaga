# Enaga – Refactoring Plan (Round 3)

Round 2 addressed naming consistency (PascalCase filenames, named colour
constants) and include organisation.  This round applies the
**Google C++ Style Guide** across the entire codebase, as required by the
project issue.

> **Exception:** Google mandates C++20 conformance; this project targets
> C++23/26 and that requirement is unchanged.

---

## 1. Tool-configuration Changes

### 1.1 `.clang-format` – Switched to Google base style

The previous configuration was based on LLVM with Allman braces, 4-space
indentation, and a 100-column limit.  All of these diverge from Google style.

**Applied fix:** Replaced the full custom config with `BasedOnStyle: Google`
plus `Standard: Latest` (clang-format 18 does not yet recognise `c++23` as a
literal; `Latest` selects the highest standard the tool understands).

Google formatting gives:
- 2-space indentation
- Attached braces (same-line opening brace)
- 80-column line limit
- Google include ordering

### 1.2 `.clang-tidy` – Updated naming rules to Google convention

The previous rules enforced `camelBack` for functions, members, parameters,
and variables.  Google requires:

| Identifier kind | Google rule | Previous rule |
|---|---|---|
| Function / method (own code) | `CamelCase` | `camelBack` |
| Non-static data member (class) | `snake_case_` (trailing `_`) | `camelBack` |
| Local variable / parameter | `snake_case` | `camelBack` |
| Constant (`constexpr`) | `kCamelCase` | mixed |
| Enum value | `kCamelCase` | `CamelCase` (no prefix) |

**Applied fix:** Updated all `CheckOptions` entries to the Google convention
and added `MemberSuffix: _`.

---

## 2. Source File Renames (snake\_case)

Google style requires filenames to be all-lowercase with underscores.

**Applied fix:** All source files renamed; all `#include` directives and
`@file` doc-comments updated; `CMakeLists.txt` source lists updated.

| Old name | New name |
|---|---|
| `PluginProcessor.{h,cpp}` | `plugin_processor.{h,cpp}` |
| `PluginEditor.{h,cpp}` | `plugin_editor.{h,cpp}` |
| `PluginEntry.cpp` | `plugin_entry.cpp` |
| `NoiseAudioSource.{h,cpp}` | `noise_audio_source.{h,cpp}` |
| `LfoEngine.{h,cpp}` | `lfo_engine.{h,cpp}` |
| `LfoMode.h` | `lfo_mode.h` |
| `NoiseType.h` | `noise_type.h` |
| `NoiseGenerator.h` | `noise_generator.h` |
| `{White,Pink,Brown,Grey}NoiseGenerator.{h,cpp}` | `{white,pink,brown,grey}_noise_generator.{h,cpp}` |
| `EnagaLookAndFeel.{h,cpp}` | `enaga_look_and_feel.{h,cpp}` |
| `MainComponent.{h,cpp}` | `main_component.{h,cpp}` |
| `PlayButton.{h,cpp}` | `play_button.{h,cpp}` |
| `LfoComponent.{h,cpp}` | `lfo_component.{h,cpp}` |
| `IOSVolumeView.{h,mm}` | `ios_volume_view.{h,mm}` |
| `Host/Source/Main.cpp` | `Host/Source/main.cpp` |
| `tests/SmokeTest.cpp` | `tests/smoke_test.cpp` |

---

## 3. Enum Value Renames

Google style requires enum values to use the `kCamelCase` prefix.

| Enum | Old value | New value |
|---|---|---|
| `LfoMode` | `Disabled` | `kDisabled` |
| `LfoMode` | `Volume` | `kVolume` |
| `LfoMode` | `Filter` | `kFilter` |
| `LfoMode` | `Both` | `kBoth` |
| `NoiseType` | `White` | `kWhite` |
| `NoiseType` | `Pink` | `kPink` |
| `NoiseType` | `Brown` | `kBrown` |
| `NoiseType` | `Grey` | `kGrey` |

---

## 4. Function / Method Renames

All project-owned methods (not JUCE virtual overrides) renamed to
`UpperCamelCase`.  JUCE virtual overrides retain their original naming
because the declaration is owned by the JUCE interface:

| Class | Old name | New name | Notes |
|---|---|---|---|
| `NoiseGenerator` | `prepare` | `Prepare` | |
| `NoiseGenerator` | `reset` | `Reset` | |
| `NoiseGenerator` | `nextSample` | `NextSample` | |
| `LfoEngine` | `tick` | `Tick` | |
| `LfoEngine` | `applyToMax` | `ApplyToMax` | |
| `LfoEngine` | `setRate/Intensity/Mode` | `SetRate/Intensity/Mode` | |
| `LfoEngine` | `getMode/Intensity` | `GetMode/Intensity` | |
| `LfoEngine` | `reset` | `Reset` | |
| `NoiseAudioSource` | `setCutoff/Gain/…` | `SetCutoff/Gain/…` | |
| `NoiseAudioSource` | `startFadeIn/Out` | `StartFadeIn/Out` | |
| `NoiseAudioSource` | `updateLpFilters` | `UpdateLpFilters` | private |
| `NoiseAudioSource` | `activeGenerator` | `ActiveGenerator` | private |
| `EnagaProcessor` | `setCutoff/Gain/…` | `SetCutoff/Gain/…` | |
| `MainComponent` | `setup*/sync*/apply*` | `Setup*/Sync*/Apply*` | private |
| `LfoComponent` | `setup*/sync*/apply*` | `Setup*/Sync*/Apply*` | private |

JUCE-owned overrides kept unchanged: `prepareToPlay`, `releaseResources`,
`getNextAudioBlock`, `processBlock`, `createEditor`, `getName`, `resized`,
`paint`, `paintButton`, `closeButtonPressed`, `initialise`, `shutdown`, etc.

---

## 5. Member Variable Renames

All non-static class data members renamed to `snake_case_` (trailing `_`).
Struct data members (e.g., `AudioCallbacks`) renamed to `snake_case`
(no trailing underscore, per Google rule for struct fields).

Static `constexpr` members that were already `kCamelCase` were left
unchanged.  New constants added:

| Old constant | New constant | Location |
|---|---|---|
| `fadeDurationSeconds` | `kFadeDurationSeconds` | `NoiseAudioSource` |
| `defaultDiscreteValue` | `kDefaultDiscreteValue` | `MainComponent` |
| `defaultContinuousValue` | `kDefaultContinuousValue` | `MainComponent` |
| `defaultVolumeValue` | `kDefaultVolumeValue` | `MainComponent` |
| `menuBarHeight` | `kMenuBarHeight` | `MainComponent` |

---

## 6. Formatting

`clang-format -i --style=file` applied to every `.h` and `.cpp` file under
`Plugin/Source/`, `Host/Source/`, and `tests/`.  The new Google-based
`.clang-format` produces: 2-space indent, same-line braces, 80-column limit.

---

## Round 2 Notes (preserved for history)

Round 1 established CI, test infrastructure, subdirectory layout, header/source
splits, and the `AudioCallbacks` struct.  Round 2 addressed:

- `IOSVolumeView` PascalCase filename rename (from `iOSVolumeView`)
- `kButtonBackground` constant added to `EnagaLookAndFeel`
- `kPlayColour`/`kStopColour` constants added to `PlayButton`
- `LfoComponent.h` include changed from `LfoEngine.h` to `LfoMode.h`
- `IOSVolumeView.mm` include path corrected to `platform/IOSVolumeView.h`

