# ---------------------------------------------------------------------------
# EnagaPlugin — JUCE plugin target (VST3 / AU)
#
# Builds the Enaga noise generator as a standard audio plugin that can be
# loaded in any VST3- or AU-compatible DAW on macOS, Windows, and Linux.
#
# The standalone application is provided by the Host/ target instead of the
# built-in JUCE Standalone wrapper, so the Standalone format is intentionally
# omitted here.
# ---------------------------------------------------------------------------

juce_add_plugin(EnagaPlugin
    PRODUCT_NAME              "Enaga"
    BUNDLE_ID                 "com.jphinspace.enaga.plugin"
    COMPANY_NAME              "jphinspace"
    COMPANY_WEBSITE           "https://github.com/jphinspace"
    COMPANY_EMAIL             ""
    VERSION                   "${PROJECT_VERSION}"
    DESCRIPTION               "Relaxing noise generator"

    # Plugin type flags
    IS_SYNTH                  FALSE
    NEEDS_MIDI_INPUT          FALSE
    NEEDS_MIDI_OUTPUT         FALSE
    EDITOR_WANTS_KEYBOARD_FOCUS FALSE

    # Plugin format codes (4-char, unique per manufacturer+product)
    PLUGIN_MANUFACTURER_CODE  Jphs
    PLUGIN_CODE               Enag

    # Build VST3 and AU plugin binaries.
    # The standalone application is the separate Host/ target.
    FORMATS                   VST3 AU

    # Category metadata
    VST3_CATEGORIES           "Fx"
    AU_MAIN_TYPE              "kAudioUnitType_Effect"
)

target_include_directories(EnagaPlugin
    PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/Source
)

target_sources(EnagaPlugin PRIVATE
    # Plugin-binary entry point (createPluginFilter) — plugin targets only
    Source/PluginEntry.cpp

    # Plugin processor and editor
    Source/PluginProcessor.cpp
    Source/PluginEditor.cpp

    # Audio engine
    Source/audio/NoiseAudioSource.cpp
    Source/audio/LfoEngine.cpp
    Source/audio/generators/WhiteNoiseGenerator.cpp
    Source/audio/generators/BrownNoiseGenerator.cpp
    Source/audio/generators/PinkNoiseGenerator.cpp
    Source/audio/generators/GreyNoiseGenerator.cpp

    # UI
    Source/ui/EnagaLookAndFeel.cpp
    Source/ui/PlayButton.cpp
    Source/ui/LfoComponent.cpp
    Source/ui/MainComponent.cpp
)

# iOS-specific: native MPVolumeView wrapper (Objective-C++, .mm extension required).
if(CMAKE_SYSTEM_NAME STREQUAL "iOS")
    target_sources(EnagaPlugin PRIVATE Source/platform/IOSVolumeView.mm)
endif()

target_compile_features(EnagaPlugin PRIVATE cxx_std_23)

target_compile_definitions(EnagaPlugin
    PUBLIC
        JUCE_WEB_BROWSER=0
        JUCE_USE_CURL=0
)

target_link_libraries(EnagaPlugin
    PRIVATE
        juce::juce_audio_processors  # AudioProcessor, AudioProcessorEditor
        juce::juce_gui_basics        # JUCEApplication, GUI lifecycle
        juce::juce_core              # String, File, Logger, …
        juce::juce_events            # MessageManager, Timer, …
        juce::juce_audio_basics      # AudioSource, AudioBuffer, …
        juce::juce_audio_devices     # AudioDeviceManager, AudioSourcePlayer, …
)

# iOS-specific: UIViewComponent (hosts MPVolumeView) lives in juce_gui_extra;
# MediaPlayer.framework provides MPVolumeView itself.
if(CMAKE_SYSTEM_NAME STREQUAL "iOS")
    target_link_libraries(EnagaPlugin PRIVATE
        juce::juce_gui_extra
        "-framework MediaPlayer"
    )
endif()
