cmake_minimum_required(VERSION 3.22)

project(Enaga
    VERSION      0.1.0
    DESCRIPTION  "Relaxing white noise generator"
    LANGUAGES    CXX
)

# ---------------------------------------------------------------------------
# C++ standard
# Using C++23 now; migrate to C++26 by changing 23 → 26 below once toolchain
# support is widespread.  See Source/Main.cpp for per-file C++26 TODO notes.
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

# --- SDL3 -----------------------------------------------------------------
# SDL3 provides low-level cross-platform audio / video / input access.
# TODO:C++26  Re-evaluate SDL3 C++ binding situation when upgrading standard.
FetchContent_Declare(
    SDL3
    GIT_REPOSITORY https://github.com/libsdl-org/SDL.git
    GIT_TAG        release-3.2.0  # keep pinned; bump deliberately
    GIT_SHALLOW    TRUE
)
# Build SDL3 as a shared library; set to ON/OFF to taste.
set(SDL_SHARED ON  CACHE BOOL "" FORCE)
set(SDL_STATIC OFF CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(SDL3)

# ---------------------------------------------------------------------------
# Enaga application target
# ---------------------------------------------------------------------------
# juce_add_gui_app is required to support iOS and Android.
# For desktop-only builds juce_add_console_app is a lighter alternative.
juce_add_gui_app(Enaga
    PRODUCT_NAME     "Enaga"
    BUNDLE_ID        "com.jphinspace.enaga"
    COMPANY_NAME     "jphinspace"
    COMPANY_WEBSITE  "https://github.com/jphinspace"
    COMPANY_EMAIL    ""
    VERSION          "0.1.0"
)

target_sources(Enaga PRIVATE
    Source/Main.cpp
)

# Require C++23 for this target explicitly (inherits project default, but
# explicit is better than implicit — a C++26 TODO marker in its own right).
target_compile_features(Enaga PRIVATE cxx_std_23)

# JUCE compile-time configuration flags
target_compile_definitions(Enaga
    PRIVATE
        JUCE_WEB_BROWSER=0       # no embedded browser needed
        JUCE_USE_CURL=0          # no libcurl dependency
        JUCE_APPLICATION_NAME_STRING="$<TARGET_PROPERTY:Enaga,JUCE_PRODUCT_NAME>"
        JUCE_APPLICATION_VERSION_STRING="$<TARGET_PROPERTY:Enaga,JUCE_VERSION>"
)

target_link_libraries(Enaga
    PRIVATE
        juce::juce_gui_basics    # JUCEApplication, GUI lifecycle
        juce::juce_core          # String, File, Logger, …
        juce::juce_events        # MessageManager, Timer, …
        SDL3::SDL3               # SDL3 shared library
)
