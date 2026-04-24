/**
 * @file   main.cpp
 * @brief  Generic VST plugin host — PluginWindow, EnagaHostApplication, and
 *         the platform-specific entry point macro.
 *
 * This host is not Enaga-specific: it owns the platform lifecycle
 * (AudioDeviceManager, window management) and routes audio through any
 * juce::AudioProcessor via juce::AudioProcessorPlayer. The concrete plugin
 * being hosted (EnagaProcessor) is the only point where Enaga-specific code
 * appears; swapping in a different plugin requires only changing that
 * instantiation.
 *
 * Built with C++26. Items that depend on JUCE adopting C++26 interfaces
 * (modules, `std::string_view` return types, `std::execution::main`, etc.)
 * are noted inline with a "NOTE(juce)" comment.
 *
 * Supported platforms (via JUCE):
 *   - macOS, Windows, Linux  (desktop)
 *   - iOS, Android           (mobile – handled by JUCE's platform glue)
 *
 * Compile-time prerequisites:
 *   cmake --preset <platform>    (see CMakePresets.json)
 */

// NOTE(juce)  Replace the #include directives below with named module imports
//             once JUCE ships module interfaces:
//
//   import std;               // replaces <cmath>, <array>, <atomic>, etc.
//   import juce;              // replaces juce_* module headers

#include <juce_audio_devices/juce_audio_devices.h>
#include <juce_audio_utils/juce_audio_utils.h>

#include "plugin_processor.h"

// ============================================================================
//  PluginWindow
// ============================================================================

/**
 * Generic application window that hosts any juce::AudioProcessorEditor.
 *
 * The window obtains its content by calling processor.createEditorIfNeeded(),
 * so it has no knowledge of the specific plugin being hosted. Closing the
 * window triggers a clean application exit.
 */
class PluginWindow final : public juce::DocumentWindow {
 public:
  /**
   * @param name       Window title bar text.
   * @param processor  The plugin whose editor should be displayed.
   */
  PluginWindow(const juce::String& name, juce::AudioProcessor& processor)
      : DocumentWindow(
            name,
            juce::Desktop::getInstance().getDefaultLookAndFeel().findColour(
                juce::ResizableWindow::backgroundColourId),
            DocumentWindow::allButtons) {
    setUsingNativeTitleBar(true);

    // createEditorIfNeeded() returns the plugin's AudioProcessorEditor.
    // setContentOwned(true) hands ownership to the DocumentWindow.
    auto* editor = processor.createEditorIfNeeded();
    jassert(editor != nullptr);
    setContentOwned(editor, true);

    // The editor called setSize() in its constructor; centreWithSize
    // picks up those dimensions automatically.
    centreWithSize(getWidth(), getHeight());
    setVisible(true);
  }

  /** Closing the window exits the application cleanly. */
  void closeButtonPressed() override {
    juce::JUCEApplication::getInstance()->systemRequestedQuit();
  }
};

// ============================================================================
//  EnagaHostApplication
// ============================================================================

/**
 * Top-level JUCE application class for the standalone plugin host.
 *
 * Responsibilities:
 *   - Platform lifecycle (initialise / shutdown).
 *   - Audio device management (open default output, no input).
 *   - Bridging the hosted AudioProcessor to the audio device via
 *     AudioProcessorPlayer.
 *   - Creating the PluginWindow that presents the plugin's editor.
 *
 * The host is generic: audio routing uses only the juce::AudioProcessor
 * interface; only the instantiation of EnagaProcessor is plugin-specific.
 *
 * NOTE(juce)  Once JUCE adopts `std::string_view` return types, these
 *             overrides can drop the heap-allocating `juce::String`.
 */
class EnagaHostApplication final : public juce::JUCEApplication {
 public:
  // -------------------------------------------------------------------
  //  JUCEApplication interface
  // -------------------------------------------------------------------

  // NOTE(juce)  Return type could become std::string_view once JUCE adopts it.
  [[nodiscard]] const juce::String getApplicationName() override {
    return juce::String(JUCE_APPLICATION_NAME_STRING);
  }
  [[nodiscard]] const juce::String getApplicationVersion() override {
    return juce::String(JUCE_APPLICATION_VERSION_STRING);
  }
  [[nodiscard]] bool moreThanOneInstanceAllowed() override { return false; }

  /**
   * Called by JUCE after all platform initialisation is complete.
   *
   * Instantiates the plugin, wires it to the audio device, and opens
   * the main window. Audio streaming starts only when the plugin's play
   * button is pressed (the plugin's fade envelope is silent at startup).
   *
   * @param commandLine  Space-separated command-line arguments provided
   *                     by the platform. Unused here.
   *                     NOTE(juce)  Prefer std::span<std::string_view> once
   *                     JUCE exposes that signature.
   */
  void initialise(const juce::String& command_line) override {
    juce::ignoreUnused(command_line);

    // Instantiate the plugin. This is the only Enaga-specific line in
    // the host; replacing EnagaProcessor with another AudioProcessor
    // subclass would host a completely different plugin.
    processor_ = std::make_unique<EnagaProcessor>();

    // Create the window; the window calls processor.createEditorIfNeeded()
    // internally so the host never touches Enaga-specific editor types.
    plugin_window_ =
        std::make_unique<PluginWindow>(getApplicationName(), *processor_);

    // Open the platform's default audio output device (stereo, no input).
    const auto error = device_manager_.initialiseWithDefaultDevices(0, 2);
    if (error.isNotEmpty()) {
      juce::Logger::writeToLog("Audio device error: " + error);
      juce::AlertWindow::showMessageBoxAsync(
          juce::MessageBoxIconType::WarningIcon, "Audio Error",
          "Could not open the audio device:\n\n" + error +
              "\n\nClose and re-open the application after checking "
              "your audio settings.",
          "OK");
      return;
    }

    // Bridge the AudioProcessor to the hardware device.
    // AudioProcessorPlayer acts as the juce::AudioIODeviceCallback that
    // calls processor->processBlock() on every audio callback.
    processor_player_.setProcessor(processor_.get());
    device_manager_.addAudioCallback(&processor_player_);
  }

  /**
   * Called by JUCE just before the application process exits.
   * Tears down audio in the reverse order of initialise() to avoid
   * use-after-free on the audio thread.
   */
  void shutdown() override {
    plugin_window_.reset();
    device_manager_.removeAudioCallback(&processor_player_);
    processor_player_.setProcessor(nullptr);
    device_manager_.closeAudioDevice();
    processor_.reset();
  }

  /** OS-level quit request (e.g. Command-Q on macOS, Alt-F4 on Windows). */
  void systemRequestedQuit() override { quit(); }

  /** Called when a second instance tries to launch (desktop only). */
  void anotherInstanceStarted(const juce::String& command_line) override {
    juce::ignoreUnused(command_line);
  }

 private:
  // Member declaration order mirrors the dependency chain so that
  // destruction (LIFO) is safe without explicit teardown ordering.
  juce::AudioDeviceManager device_manager_;  // owns the hardware device
  juce::AudioProcessorPlayer
      processor_player_;  // bridges AudioProcessor → device
  std::unique_ptr<EnagaProcessor> processor_;    // the hosted plugin
  std::unique_ptr<PluginWindow> plugin_window_;  // destroyed first (UI last)
};

// ============================================================================
//  Platform entry point
// ============================================================================

// START_JUCE_APPLICATION expands to the appropriate platform entry point:
//   • main()                    on macOS / Windows / Linux
//   • UIApplicationMain(…)      on iOS
//   • android_main(…)           on Android
//
// NOTE(juce)  START_JUCE_APPLICATION may be replaced by std::execution::main
//             once JUCE adopts the C++26 execution model.
START_JUCE_APPLICATION(EnagaHostApplication)
