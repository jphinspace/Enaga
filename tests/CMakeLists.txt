cmake_minimum_required(VERSION 3.22)

# ---------------------------------------------------------------------------
# Enaga offline unit tests
# ---------------------------------------------------------------------------
# These tests run without a real audio device and are intended to be used by
# CTest (cmake --build --preset linux && ctest --preset linux) and CI.
# ---------------------------------------------------------------------------

enable_testing()

# ---------------------------------------------------------------------------
# Smoke test: constructs NoiseAudioSource and exercises the audio path
# ---------------------------------------------------------------------------

add_executable(EnagaSmokeTest smoke_test.cpp

    # Audio sources under test (now in Plugin/Source/ after refactoring)
    ${CMAKE_SOURCE_DIR}/Plugin/Source/audio/noise_audio_source.cpp
    ${CMAKE_SOURCE_DIR}/Plugin/Source/audio/lfo_engine.cpp
    ${CMAKE_SOURCE_DIR}/Plugin/Source/audio/generators/white_noise_generator.cpp
    ${CMAKE_SOURCE_DIR}/Plugin/Source/audio/generators/pink_noise_generator.cpp
    ${CMAKE_SOURCE_DIR}/Plugin/Source/audio/generators/brown_noise_generator.cpp
    ${CMAKE_SOURCE_DIR}/Plugin/Source/audio/generators/grey_noise_generator.cpp
)

target_include_directories(EnagaSmokeTest PRIVATE
    ${CMAKE_SOURCE_DIR}/Plugin/Source
)

target_compile_features(EnagaSmokeTest PRIVATE cxx_std_23)

target_compile_definitions(EnagaSmokeTest PRIVATE
    JUCE_WEB_BROWSER=0
    JUCE_USE_CURL=0
    JUCE_GLOBAL_MODULE_SETTINGS_INCLUDED=1
    JUCE_STANDALONE_APPLICATION=0
)

target_link_libraries(EnagaSmokeTest PRIVATE
    juce::juce_audio_basics
    juce::juce_core
)

add_test(NAME SmokeTest COMMAND EnagaSmokeTest)
