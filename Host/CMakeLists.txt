# ---------------------------------------------------------------------------
# EnagaHost — generic standalone plugin host application
#
# Hosts any juce::AudioProcessor via juce::AudioProcessorPlayer and
# juce::AudioDeviceManager.  The only Enaga-specific code is the
# instantiation of EnagaProcessor in Main.cpp; everything else is generic
# JUCE plugin-hosting infrastructure.
#
# Supported platforms: macOS, Windows, Linux, iOS, Android.
# ---------------------------------------------------------------------------

juce_add_gui_app(EnagaHost
    PRODUCT_NAME     "Enaga"
    BUNDLE_ID        "com.jphinspace.enaga"
    COMPANY_NAME     "jphinspace"
    COMPANY_WEBSITE  "https://github.com/jphinspace"
    COMPANY_EMAIL    ""
    VERSION          "${PROJECT_VERSION}"
)

target_include_directories(EnagaHost PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/Source
    ${CMAKE_SOURCE_DIR}/Plugin/Source   # EnagaProcessor and its transitive headers
)

target_sources(EnagaHost PRIVATE
    Source/main.cpp

    # Plugin sources compiled directly into the host so that EnagaProcessor
    # can be instantiated without loading a plugin binary at runtime.
    # plugin_entry.cpp is intentionally omitted — createPluginFilter() is not
    # needed in the host context.
    ${CMAKE_SOURCE_DIR}/Plugin/Source/plugin_processor.cpp
    ${CMAKE_SOURCE_DIR}/Plugin/Source/plugin_editor.cpp

    # Audio engine
    ${CMAKE_SOURCE_DIR}/Plugin/Source/audio/noise_audio_source.cpp
    ${CMAKE_SOURCE_DIR}/Plugin/Source/audio/lfo_engine.cpp
    ${CMAKE_SOURCE_DIR}/Plugin/Source/audio/generators/white_noise_generator.cpp
    ${CMAKE_SOURCE_DIR}/Plugin/Source/audio/generators/brown_noise_generator.cpp
    ${CMAKE_SOURCE_DIR}/Plugin/Source/audio/generators/pink_noise_generator.cpp
    ${CMAKE_SOURCE_DIR}/Plugin/Source/audio/generators/grey_noise_generator.cpp

    # UI
    ${CMAKE_SOURCE_DIR}/Plugin/Source/ui/enaga_look_and_feel.cpp
    ${CMAKE_SOURCE_DIR}/Plugin/Source/ui/play_button.cpp
    ${CMAKE_SOURCE_DIR}/Plugin/Source/ui/lfo_component.cpp
    ${CMAKE_SOURCE_DIR}/Plugin/Source/ui/main_component.cpp
)

# iOS-specific: native MPVolumeView wrapper (Objective-C++, .mm extension required).
if(CMAKE_SYSTEM_NAME STREQUAL "iOS")
    target_sources(EnagaHost PRIVATE
        ${CMAKE_SOURCE_DIR}/Plugin/Source/platform/ios_volume_view.mm
    )
endif()

target_compile_features(EnagaHost PRIVATE cxx_std_23)

target_compile_definitions(EnagaHost
    PRIVATE
        JUCE_WEB_BROWSER=0
        JUCE_USE_CURL=0
        JUCE_APPLICATION_NAME_STRING="$<TARGET_PROPERTY:EnagaHost,JUCE_PRODUCT_NAME>"
        JUCE_APPLICATION_VERSION_STRING="$<TARGET_PROPERTY:EnagaHost,JUCE_VERSION>"
)

target_link_libraries(EnagaHost
    PRIVATE
        juce::juce_audio_processors  # AudioProcessor, AudioProcessorEditor
        juce::juce_audio_utils       # AudioProcessorPlayer
        juce::juce_gui_basics        # JUCEApplication, GUI lifecycle
        juce::juce_core              # String, File, Logger, …
        juce::juce_events            # MessageManager, Timer, …
        juce::juce_audio_basics      # AudioSource, AudioBuffer, …
        juce::juce_audio_devices     # AudioDeviceManager, …
)

# iOS-specific: UIViewComponent (hosts MPVolumeView) lives in juce_gui_extra;
# MediaPlayer.framework provides MPVolumeView itself.
if(CMAKE_SYSTEM_NAME STREQUAL "iOS")
    target_link_libraries(EnagaHost PRIVATE
        juce::juce_gui_extra
        "-framework MediaPlayer"
    )
endif()
