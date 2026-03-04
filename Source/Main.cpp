/**
 * @file   Main.cpp
 * @brief  Enaga application entry point — MainWindow, EnagaApplication, and
 *         the platform-specific entry point macro.
 *
 * Class responsibilities are split across separate source files:
 *   - EnagaLookAndFeel.h          : dark theme / LookAndFeel overrides
 *   - PlayButton.h                : custom play/stop toggle button
 *   - WhiteNoiseAudioSource.h/cpp : noise generator with LP filter + gain
 *   - MainComponent.h/cpp         : root UI component and preset I/O
 *
 * Built with C++23.  All areas that benefit from C++26 features are marked
 * with a "TODO:C++26" comment so the eventual migration is straightforward.
 *
 * Supported platforms (via JUCE):
 *   - macOS, Windows, Linux  (desktop)
 *   - iOS, Android           (mobile – handled by JUCE's platform glue)
 *
 * Compile-time prerequisites:
 *   cmake --preset <platform>    (see CMakePresets.json)
 */

// TODO:C++26  Replace the #include directives below with named module imports
//             once JUCE and the standard library ship module interfaces:
//
//   import std;               // replaces <cmath>, <array>, <atomic>, etc.
//   import juce;              // replaces juce_* module headers

#include "EnagaLookAndFeel.h"
#include "MainComponent.h"
#include "WhiteNoiseAudioSource.h"

#include <juce_audio_devices/juce_audio_devices.h>  // AudioDeviceManager, AudioSourcePlayer (EnagaApplication)

// ============================================================================
//  MainWindow
// ============================================================================

/**
 * Single application window hosting MainComponent.
 *
 * Keeping a window open is what keeps the JUCE event loop (and therefore the
 * audio stream) alive.  Closing the window triggers a clean exit via
 * systemRequestedQuit(), which calls shutdown() before the process exits.
 */
class MainWindow final : public juce::DocumentWindow
{
public:
    MainWindow(const juce::String& name,
               MainComponent::AudioToggleCallback       onToggle,
               MainComponent::AudioFilterCallback       onFilter,
               MainComponent::AudioGainCallback         onGain,
               MainComponent::AudioNoiseTypeCallback    onNoiseType,
               MainComponent::AudioLfoRateCallback      onLfoRate,
               MainComponent::AudioLfoIntensityCallback onLfoIntensity,
               MainComponent::AudioLfoModeCallback      onLfoMode)
        : DocumentWindow(name,
                         juce::Colour(0xff1a1a1a),
                         DocumentWindow::allButtons)
    {
        setUsingNativeTitleBar(true);
        setResizable(true, false);
        setResizeLimits(320, 240,       // minimum per issue requirement
                        10000, 10000);  // effectively unbounded maximum

        setContentOwned(
            new MainComponent(std::move(onToggle), std::move(onFilter),
                              std::move(onGain), std::move(onNoiseType),
                              std::move(onLfoRate), std::move(onLfoIntensity),
                              std::move(onLfoMode)), true);

        centreWithSize(480, 600);
        setVisible(true);
    }

    /** Closing the window exits the application cleanly. */
    void closeButtonPressed() override
    {
        juce::JUCEApplication::getInstance()->systemRequestedQuit();
    }
};

// ============================================================================
//  EnagaApplication
// ============================================================================

/**
 * Top-level JUCE application class.
 *
 * JUCE calls initialise() once the platform event loop is ready, then calls
 * shutdown() when the application is about to exit.  On mobile platforms the
 * lifecycle is driven by the OS (UIApplicationDelegate on iOS,
 * android.app.Activity on Android); JUCE abstracts this behind the same
 * JUCEApplication interface used on the desktop.
 *
 * Audio lifetime is user-controlled (play/stop button); the device is opened
 * in initialise() and closed in shutdown().
 *
 * TODO:C++26  'final' is already C++11, but once reflection lands in C++26
 *             consider annotating with [[clang::trivially_relocatable]] or the
 *             equivalent standard attribute for move-optimisation.
 */
class EnagaApplication final : public juce::JUCEApplication
{
public:
    // -----------------------------------------------------------------------
    //  JUCEApplication interface
    // -----------------------------------------------------------------------

    // TODO:C++26  Return type could become std::string_view when JUCE adopts it.
    [[nodiscard]] const juce::String getApplicationName()    override { return "Enaga"; }
    [[nodiscard]] const juce::String getApplicationVersion() override { return "0.1.0"; }
    [[nodiscard]] bool moreThanOneInstanceAllowed()           override { return false;   }

    /**
     * Called by JUCE after all platform initialisation is complete.
     *
     * Sets the global look and feel, creates the main window, and opens the
     * default audio output device (CoreAudio / WASAPI / ALSA, depending on
     * the platform).  Audio streaming starts only when the user presses the
     * play button.
     *
     * @param commandLine  Space-separated command-line arguments provided by
     *                     the platform.  Unused here.
     *                     TODO:C++26  Prefer std::span<std::string_view>.
     */
    void initialise(const juce::String& commandLine) override
    {
        juce::ignoreUnused(commandLine);

        // Apply dark theme globally before any window is created.
        juce::LookAndFeel::setDefaultLookAndFeel(&lookAndFeel);

        // Create the main window; audio starts only when the play button is pressed.
        mainWindow = std::make_unique<MainWindow>(
            getApplicationName(),
            [this](bool shouldPlay) { toggleAudio(shouldPlay);                  },
            [this](float v)         { noiseSource.setCutoff(v);                 },
            [this](float g)         { noiseSource.setGain(g);                   },
            [this](float v)         { noiseSource.setNoiseType(
                                          static_cast<NoiseType>(
                                              juce::jlimit(0, 3,
                                                  static_cast<int>(v) - 1))); },
            [this](float r)         { noiseSource.setLfoRate(r);                },
            [this](float i)         { noiseSource.setLfoIntensity(i);           },
            [this](LfoMode m)       { noiseSource.setLfoMode(m);                });

        // Open the platform's default audio output device (stereo, no input).
        const auto error = deviceManager.initialiseWithDefaultDevices(0, 2);
        if (error.isNotEmpty())
        {
            juce::Logger::writeToLog("Audio device error: " + error);
            juce::AlertWindow::showMessageBoxAsync(
                juce::MessageBoxIconType::WarningIcon,
                "Audio Error",
                "Could not open the audio device:\n\n" + error
                    + "\n\nClose and re-open the application after checking "
                      "your audio settings.",
                "OK");
            return;
        }

        // Connect source to player and register the callback for the duration of
        // the session.  The fade envelope (fadeCurrent = 0 at startup) keeps the
        // output silent until the play button is pressed.
        sourcePlayer.setSource(&noiseSource);
        deviceManager.addAudioCallback(&sourcePlayer);
    }

    /**
     * Called by JUCE just before the application process exits.
     * Tears down audio in the reverse order of initialise() to avoid
     * use-after-free on the audio thread.
     */
    void shutdown() override
    {
        mainWindow.reset();
        juce::LookAndFeel::setDefaultLookAndFeel(nullptr);
        deviceManager.removeAudioCallback(&sourcePlayer);
        sourcePlayer.setSource(nullptr);
        deviceManager.closeAudioDevice();
    }

    /** OS-level quit request (e.g. Command-Q on macOS, Alt-F4 on Windows). */
    void systemRequestedQuit() override
    {
        quit();
    }

    /** Called when a second instance tries to launch (desktop only). */
    void anotherInstanceStarted(const juce::String& commandLine) override
    {
        juce::ignoreUnused(commandLine);
    }

private:
    /** Fades in or out based on whether the play button is on or off. */
    void toggleAudio(bool shouldPlay)
    {
        if (shouldPlay) noiseSource.startFadeIn(); else noiseSource.startFadeOut();
    }

    // Declaration order matches the dependency chain (destroyed in reverse).
    EnagaLookAndFeel             lookAndFeel;   // must outlive all UI components
    juce::AudioDeviceManager     deviceManager; // owns the hardware device
    juce::AudioSourcePlayer      sourcePlayer;  // bridges AudioSource → device
    WhiteNoiseAudioSource        noiseSource;   // generates the noise samples
    std::unique_ptr<MainWindow>  mainWindow;    // destroyed first (UI last)
};

// ============================================================================
//  Platform entry point
// ============================================================================

// START_JUCE_APPLICATION expands to the appropriate platform entry point:
//   • main()                    on macOS / Windows / Linux
//   • UIApplicationMain(…)      on iOS
//   • android_main(…)           on Android
//
// TODO:C++26  If JUCE adopts std::execution::main this macro may be removed
//             in favour of a standard mechanism.
START_JUCE_APPLICATION(EnagaApplication)
