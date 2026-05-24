cmake_minimum_required(VERSION 3.25)  # 3.25+ required for cxx_std_26

project(Enaga
    VERSION      0.1.0
    DESCRIPTION  "Relaxing noise generator"
    LANGUAGES    C CXX
)

if(CMAKE_SYSTEM_NAME STREQUAL "iOS")
    enable_language(OBJCXX)
endif()

# ---------------------------------------------------------------------------
# Dependencies via FetchContent
# ---------------------------------------------------------------------------
include(FetchContent)

# Keep downloaded sources across CMake re-runs so builds stay offline-capable
set(FETCHCONTENT_UPDATES_DISCONNECTED ON)

# --- JUCE -----------------------------------------------------------------
# JUCE handles the platform lifecycle for iOS, Android, macOS, Windows, Linux.
# NOTE: Update to a JUCE release with first-class C++26 support when available;
#       currently pinned to 8.0.13 (targets C++17 internally).
FetchContent_Declare(
    JUCE
    GIT_REPOSITORY https://github.com/juce-framework/JUCE.git
    GIT_TAG        8.0.13         # keep pinned; bump deliberately
    GIT_SHALLOW    TRUE
)
FetchContent_MakeAvailable(JUCE)

# ---------------------------------------------------------------------------
# Tests
# ---------------------------------------------------------------------------
include(CTest)
add_subdirectory(tests)

# ---------------------------------------------------------------------------
# Component 1: Enaga plugin
# ---------------------------------------------------------------------------
# Source lives in Enaga/; builds VST3/AU on desktop and AUv3 on iOS.
add_subdirectory(Enaga)
