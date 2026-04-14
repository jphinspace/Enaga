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
    Source/Main.cpp

    # Plugin sources compiled directly into the host so that EnagaProcessor
    # can be instantiated without loading a plugin binary at runtime.
    # PluginEntry.cpp is intentionally omitted — createPluginFilter() is not
    # needed in the host context.
    ${CMAKE_SOURCE_DIR}/Plugin/Source/PluginProcessor.cpp
    ${CMAKE_SOURCE_DIR}/Plugin/Source/PluginEditor.cpp

    # Audio engine
    ${CMAKE_SOURCE_DIR}/Plugin/Source/audio/NoiseAudioSource.cpp
    ${CMAKE_SOURCE_DIR}/Plugin/Source/audio/LfoEngine.cpp
    ${CMAKE_SOURCE_DIR}/Plugin/Source/audio/generators/WhiteNoiseGenerator.cpp
    ${CMAKE_SOURCE_DIR}/Plugin/Source/audio/generators/BrownNoiseGenerator.cpp
    ${CMAKE_SOURCE_DIR}/Plugin/Source/audio/generators/PinkNoiseGenerator.cpp
    ${CMAKE_SOURCE_DIR}/Plugin/Source/audio/generators/GreyNoiseGenerator.cpp

    # UI
    ${CMAKE_SOURCE_DIR}/Plugin/Source/ui/EnagaLookAndFeel.cpp
    ${CMAKE_SOURCE_DIR}/Plugin/Source/ui/PlayButton.cpp
    ${CMAKE_SOURCE_DIR}/Plugin/Source/ui/LfoComponent.cpp
    ${CMAKE_SOURCE_DIR}/Plugin/Source/ui/MainComponent.cpp
)

# iOS-specific: native MPVolumeView wrapper (Objective-C++, .mm extension required).
if(CMAKE_SYSTEM_NAME STREQUAL "iOS")
    target_sources(EnagaHost PRIVATE
        ${CMAKE_SOURCE_DIR}/Plugin/Source/platform/IOSVolumeView.mm
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
