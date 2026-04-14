# Enaga – Refactoring Plan (Round 2)

Round 1 established CI, test infrastructure, subdirectory layout, header/source
splits, and the `AudioCallbacks` struct.  This document records the next pass,
focusing exclusively on **naming consistency** and **include organisation**.

---

## 1. Naming Gaps

### 1.1 `iOSVolumeView` breaks the PascalCase filename convention

Every other source file in the project uses PascalCase:
`NoiseAudioSource`, `LfoEngine`, `MainComponent`, `PlayButton`, …

`iOSVolumeView.h` / `iOSVolumeView.mm` start with a lowercase `i`, which is
inconsistent with the rest of the project and can cause confusion on
case-sensitive filesystems (Linux).

**Applied fix:** Renamed to `IOSVolumeView.h` / `IOSVolumeView.mm`; class
renamed to `IOSVolumeView`.  All references in `MainComponent.h`,
`CMakeLists.txt`, and `README.md` updated accordingly.

---

### 1.2 Raw hex literal for TextButton background in `EnagaLookAndFeel`

`EnagaLookAndFeel.cpp` sets the TextButton background colour using a raw hex
literal (`0xff212121`) that had no corresponding named constant in
`EnagaLookAndFeel.h`.  Five other theme colours were already promoted to
`static constexpr` members (`kBackground`, `kPanel`, `kAccent`, `kText`,
`kImageArea`); this sixth colour was missed.

**Applied fix:** Added `static constexpr juce::uint32 kButtonBackground =
0xff212121` to `EnagaLookAndFeel` and replaced the raw literal in the
constructor.

---

### 1.3 Unnamed semantic colours in `PlayButton`

`PlayButton::paintButton` contained two unnamed hex literals:
- `0xffe53935` — stop / playing state (red)
- `0xff43a047` — play / stopped state (green)

These are not theme colours (they convey play/stop state), but unnamed
literals are still harder to understand and maintain than named constants.

**Applied fix:** Added `static constexpr juce::uint32 kStopColour` and
`kPlayColour` as `public` members of `PlayButton` and replaced both raw
literals in `PlayButton.cpp`.

---

## 2. Include Organisation

### 2.1 `LfoComponent.h` includes more than it uses

`LfoComponent.h` included `audio/LfoEngine.h` to obtain the `LfoMode` type.
`LfoComponent` never instantiates or uses `LfoEngine` directly — it only needs
the enum.  Pulling in the full engine header adds an unnecessary transitive
dependency on `<atomic>` and `audio/LfoMode.h` indirectly.

**Applied fix:** Changed the include in `LfoComponent.h` from
`"audio/LfoEngine.h"` to `"audio/LfoMode.h"`.

---

### 2.2 `IOSVolumeView.mm` used a bare (directory-relative) include

Every other `.cpp` / `.mm` file in the project includes its own header using
the `Source/`-relative path (e.g. `"audio/LfoEngine.h"`,
`"ui/MainComponent.h"`).  The old `iOSVolumeView.mm` used a bare `#include
"iOSVolumeView.h"` (relative to the same directory), which is inconsistent
and fragile if the include path is ever narrowed.

**Applied fix:** Updated to `#include "platform/IOSVolumeView.h"` (applied
as part of the rename in §1.1 above).

---

## 3. Summary of Applied Changes

| # | File(s) changed | What changed |
|---|-----------------|--------------|
| 1.1 | `Source/platform/iOSVolumeView.{h,mm}` → `IOSVolumeView.{h,mm}` | Renamed files and class to PascalCase |
| 1.1 | `Source/ui/MainComponent.h` | Updated `#include` and member type |
| 1.1 | `CMakeLists.txt` | Updated iOS source path |
| 1.1 | `README.md` | Updated project-structure listing |
| 1.2 | `Source/ui/EnagaLookAndFeel.h` | Added `kButtonBackground` constant |
| 1.2 | `Source/ui/EnagaLookAndFeel.cpp` | Replaced `0xff212121` literal |
| 1.3 | `Source/ui/PlayButton.h` | Added `kPlayColour` / `kStopColour` constants |
| 1.3 | `Source/ui/PlayButton.cpp` | Replaced two raw hex literals |
| 2.1 | `Source/ui/LfoComponent.h` | `#include "audio/LfoMode.h"` (was `LfoEngine.h`) |
| 2.2 | `Source/platform/IOSVolumeView.mm` | Full-path include (part of rename) |
