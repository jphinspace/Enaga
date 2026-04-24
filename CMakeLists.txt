cmake_minimum_required(VERSION 3.25)  # 3.25+ required for cxx_std_26

project(Enaga
    VERSION      0.1.0
    DESCRIPTION  "Relaxing noise generator"
    LANGUAGES    C CXX
)

# ---------------------------------------------------------------------------
# C++ standard
# ---------------------------------------------------------------------------
set(CMAKE_CXX_STANDARD          26)
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
# NOTE: Update to a JUCE release with first-class C++26 support when available;
#       currently pinned to 8.0.4 (targets C++17 internally).
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
