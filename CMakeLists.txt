cmake_minimum_required(VERSION 3.22)

project(Enaga
    VERSION      0.1.0
    DESCRIPTION  "Relaxing noise generator"
    LANGUAGES    C CXX
)

# ---------------------------------------------------------------------------
# C++ standard
# Using C++23 now; migrate to C++26 by changing 23 → 26 below once toolchain
# support is widespread.  See the Source/ directory for per-file C++26 TODO notes.
# ---------------------------------------------------------------------------
set(CMAKE_CXX_STANDARD          23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS        OFF)

# ---------------------------------------------------------------------------
# Dependencies via FetchContent
# ---------------------------------------------------------------------------
include(FetchContent)

# Keep downloaded sources across CMake re-runs so builds stay offline-capable
set(FETCHCONTENT_UPDATES_DISCONNECTED ON)

# --- JUCE -----------------------------------------------------------------
# JUCE handles the platform lifecycle for iOS, Android, macOS, Windows, Linux.
# TODO:C++26  Update to the JUCE release that ships first-class C++26 support.
FetchContent_Declare(
    JUCE
    GIT_REPOSITORY https://github.com/juce-framework/JUCE.git
    GIT_TAG        8.0.4          # keep pinned; bump deliberately
    GIT_SHALLOW    TRUE
)
FetchContent_MakeAvailable(JUCE)

# ---------------------------------------------------------------------------
# Tests
# ---------------------------------------------------------------------------
include(CTest)
add_subdirectory(tests)

# ---------------------------------------------------------------------------
# Component 1: Enaga VST/AU plugin
# ---------------------------------------------------------------------------
# Source lives in Plugin/; builds VST3 and AU formats that can be loaded
# in any compatible DAW.
add_subdirectory(Plugin)

# ---------------------------------------------------------------------------
# Component 2: Generic standalone plugin host application
# ---------------------------------------------------------------------------
# Source lives in Host/; hosts EnagaProcessor (or any juce::AudioProcessor)
# via AudioProcessorPlayer and AudioDeviceManager on all target platforms.
add_subdirectory(Host)
