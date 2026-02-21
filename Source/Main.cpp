/**
 * @file   Main.cpp
 * @brief  Enaga – Relaxing White Noise Generator
 *
 * Generates white noise via JUCE's audio-device API while the application is
 * running.  Audio stops automatically when the app is closed.  Perceived
 * volume is governed entirely by the platform (OS) volume control.
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
//   import std;               // replaces <iostream>
//   import juce;              // replaces juce_* module headers

#include <juce_audio_devices/juce_audio_devices.h>  // AudioDeviceManager, AudioSourcePlayer
#include <juce_gui_basics/juce_gui_basics.h>         // JUCEApplication, DocumentWindow
#include <juce_core/juce_core.h>                     // String, Logger, Random

// ============================================================================
//  WhiteNoiseAudioSource
// ============================================================================

/**
 * Fills every audio buffer with uniformly distributed random samples in
 * [-1, 1], producing flat-spectrum (white) noise.
 *
 * Output amplitude is at full scale; the OS/hardware volume control governs
 * the perceived loudness — no application-side gain stage is required.
 */
class WhiteNoiseAudioSource final : public juce::AudioSource
{
public:
    void prepareToPlay(int /*samplesPerBlockExpected*/, double /*sampleRate*/) override {}
    void releaseResources() override {}

    void getNextAudioBlock(const juce::AudioSourceChannelInfo& info) override
    {
        for (int ch = 0; ch < info.buffer->getNumChannels(); ++ch)
        {
            auto* out = info.buffer->getWritePointer(ch, info.startSample);
            for (int i = 0; i < info.numSamples; ++i)
                out[i] = random.nextFloat() * 2.0f - 1.0f;
        }
    }

private:
    juce::Random random;  // PRNG seeded from system entropy at construction
};

// ============================================================================
//  MainWindow
// ============================================================================

/**
 * Single application window.
 *
 * Keeping a window open is what keeps the JUCE event loop (and therefore the
 * audio stream) running.  Closing the window triggers a clean exit via
 * systemRequestedQuit(), which calls shutdown() before the process exits.
 */
class MainWindow final : public juce::DocumentWindow
{
public:
    explicit MainWindow(const juce::String& name)
        : DocumentWindow(name,
                         juce::Desktop::getInstance().getDefaultLookAndFeel()
                             .findColour(juce::ResizableWindow::backgroundColourId),
                         DocumentWindow::allButtons)
    {
        setUsingNativeTitleBar(true);
        setResizable(true, false);

        auto* label = new juce::Label(
            "status",
            "White noise is playing.\nUse your system volume to adjust loudness.");
        label->setFont(juce::Font(18.0f));
        label->setJustificationType(juce::Justification::centred);
        label->setSize(400, 120);
        setContentOwned(label, true);

        centreWithSize(getWidth(), getHeight());
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
 * Audio lifetime mirrors application lifetime:
 *   initialise() → open default audio device, begin streaming white noise
 *   shutdown()   → stop the stream, release the device
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
     * Opens the default audio output device (CoreAudio / WASAPI / ALSA,
     * depending on the platform) and begins streaming white noise.
     *
     * @param commandLine  Space-separated command-line arguments provided by
     *                     the platform.  Unused here.
     *                     TODO:C++26  Prefer std::span<std::string_view>.
     */
    void initialise(const juce::String& commandLine) override
    {
        juce::ignoreUnused(commandLine);

        // Show the main window first so the event loop stays alive even if
        // audio initialisation fails.
        mainWindow = std::make_unique<MainWindow>(getApplicationName());

        // Open the platform's default audio output device (stereo, no input).
        // AudioDeviceManager automatically honours the OS volume control.
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

        // Connect: noiseSource → sourcePlayer → deviceManager.
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
    // Declaration order matches the dependency chain (destroyed in reverse).
    juce::AudioDeviceManager     deviceManager;  // owns the hardware device
    juce::AudioSourcePlayer      sourcePlayer;   // bridges AudioSource → device
    WhiteNoiseAudioSource        noiseSource;    // generates the noise samples
    std::unique_ptr<MainWindow>  mainWindow;     // destroyed first (UI last)
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
