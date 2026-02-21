/**
 * @file   Main.cpp
 * @brief  Enaga – Relaxing White Noise Generator
 *         Application skeleton
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

#include <juce_gui_basics/juce_gui_basics.h>   // JUCEApplication, MessageManager
#include <juce_core/juce_core.h>               // String, Logger

#include <iostream>                             // std::cout / std::cerr
                                                // TODO:C++26 → std::print / std::println

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
     * @param commandLine  Space-separated command-line arguments provided by
     *                     the platform.  Unused in this skeleton.
     *                     TODO:C++26  Prefer std::span<std::string_view>.
     */
    void initialise(const juce::String& commandLine) override
    {
        juce::ignoreUnused(commandLine);

        // ── Hello World ────────────────────────────────────────────────────
        // TODO:C++26  Replace with: std::println("Hello World");
        std::cout << "Hello World" << '\n';

        // ── Clean exit ─────────────────────────────────────────────────────
        // quit() posts a quit message to JUCE's event loop; shutdown() is
        // called before the process exits (see below).
        // TODO:C++26  C++26 structured concurrency (std::execution) may
        //             provide a cleaner async-shutdown pattern here.
        quit();
    }

    /**
     * Called by JUCE just before the application process exits.
     * All resource teardown belongs here to guarantee symmetric init/shutdown.
     */
    void shutdown() override
    {
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
