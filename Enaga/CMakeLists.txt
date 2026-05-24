# ---------------------------------------------------------------------------
# EnagaPlugin — JUCE plugin target (VST3 / AU / AUv3)
#
# Builds the Enaga noise generator as a standard audio plugin that can be
# loaded in compatible DAWs/hosts on desktop and iOS.
#
# Standalone format is intentionally omitted; this repository builds plugin
# formats only.
# ---------------------------------------------------------------------------

if(CMAKE_SYSTEM_NAME STREQUAL "iOS")
    set(ENAGA_PLUGIN_FORMATS AUv3 Standalone)
    set(ENAGA_IOS_ASSET_ARGS
        CUSTOM_XCASSETS_FOLDER "${CMAKE_CURRENT_SOURCE_DIR}/Resources/iOS/Assets.xcassets")
else()
    set(ENAGA_PLUGIN_FORMATS VST3 AU)
    set(ENAGA_IOS_ASSET_ARGS)
endif()

juce_add_plugin(EnagaPlugin
    PRODUCT_NAME              "Enaga"
    BUNDLE_ID                 "com.jphinspace.enaga.plugin"
    COMPANY_NAME              "jphinspace"
    COMPANY_WEBSITE           "https://github.com/jphinspace"
    COMPANY_EMAIL             ""
    VERSION                   "${PROJECT_VERSION}"
    DESCRIPTION               "Relaxing noise generator"
    ${ENAGA_IOS_ASSET_ARGS}

    # Plugin type flags
    IS_SYNTH                  FALSE
    NEEDS_MIDI_INPUT          FALSE
    NEEDS_MIDI_OUTPUT         FALSE
    EDITOR_WANTS_KEYBOARD_FOCUS FALSE

    # Plugin format codes (4-char, unique per manufacturer+product)
    PLUGIN_MANUFACTURER_CODE  Jphs
    PLUGIN_CODE               Enag

    # iOS builds include a small standalone container app so the AUv3 can be
    # installed on a device; desktop builds keep the regular VST3/AU outputs.
    FORMATS                   ${ENAGA_PLUGIN_FORMATS}

    # Category metadata
    VST3_CATEGORIES           "Fx"
    AU_MAIN_TYPE              "kAudioUnitType_Effect"
)

target_include_directories(EnagaPlugin
    PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/Source
)

target_sources(EnagaPlugin PRIVATE
    # Plugin-binary entry point (createPluginFilter) — plugin targets only
    Source/entry.cpp

    # Plugin processor and editor
    Source/processor.cpp
    Source/processor_core.cpp
    Source/editor.cpp
    Source/editor_actions.cpp

    # Audio engine
    Source/audio/noise_audio_source.cpp
    Source/audio/lfo_engine.cpp
    Source/audio/generators/white_noise_generator.cpp
    Source/audio/generators/brown_noise_generator.cpp
    Source/audio/generators/pink_noise_generator.cpp
    Source/audio/generators/grey_noise_generator.cpp

    # UI
    Source/ui/enaga_look_and_feel.cpp
    Source/ui/play_button.cpp
    Source/ui/lfo_component.cpp
    Source/ui/main_component.cpp
)

# iOS-specific: native MPVolumeView wrapper (Objective-C++, .mm extension required).
if(CMAKE_SYSTEM_NAME STREQUAL "iOS")
    target_sources(EnagaPlugin PRIVATE Source/platform/ios_volume_view.mm)
endif()

target_compile_features(EnagaPlugin PRIVATE cxx_std_26)

target_compile_definitions(EnagaPlugin
    PUBLIC
        JUCE_WEB_BROWSER=0
        JUCE_USE_CURL=0
        # No VST2 version of this plugin has ever been released; disabling
        # VST3_CAN_REPLACE_VST2 avoids a JUCE 8.0.13+ compile-time error about
        # potential parameter-automation conflicts between VST2 and VST3.
        JUCE_VST3_CAN_REPLACE_VST2=0
)

target_link_libraries(EnagaPlugin
    PRIVATE
        juce::juce_audio_processors  # AudioProcessor, AudioProcessorEditor
        juce::juce_gui_basics        # JUCEApplication, GUI lifecycle
        juce::juce_core              # String, File, Logger, …
        juce::juce_events            # MessageManager, Timer, …
        juce::juce_audio_basics      # AudioSource, AudioBuffer, …
        juce::juce_audio_devices     # AudioDeviceManager, AudioSourcePlayer, …
        juce::juce_audio_utils       # Standalone plugin wrapper on iOS
)

# iOS-specific: UIViewComponent (hosts MPVolumeView) lives in juce_gui_extra;
# MediaPlayer.framework provides MPVolumeView itself.
if(CMAKE_SYSTEM_NAME STREQUAL "iOS")
    target_link_libraries(EnagaPlugin PRIVATE
        juce::juce_gui_extra
        "-framework CoreAudioKit"
        "-framework MediaPlayer"
    )
endif()
