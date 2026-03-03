/**
 * @file   MainComponent.h
 * @brief  Root UI component for the Enaga application window.
 */

#pragma once

#include "PlayButton.h"

#include <juce_core/juce_core.h>
#include <juce_gui_basics/juce_gui_basics.h>

#include <functional>
#include <memory>

/**
 * Root UI component that fills the application window.
 *
 * Layout (top to bottom):
 *   1. Menu bar (desktop) or image area with overlay button (mobile).
 *   2. Image / branding area with decorative waveform.
 *   3. Play button  +  discrete (stepped) slider with label.
 *   4. "Cutoff" label  +  continuous slider  +  numeric text box (0.0–100.0).
 *   5. "Volume" label  +  volume slider  +  numeric text box  (desktop only).
 *      On iOS/Android perceived loudness is controlled by the system media
 *      volume (hardware buttons), so no application-side slider is shown.
 *
 * Platform-specific navigation:
 *   - Desktop (macOS/Windows/Linux) : juce::MenuBarComponent at the top.
 *   - iOS                           : Gear icon button (⚙) top-right of image.
 *   - Android                       : Hamburger button (☰) top-left of image.
 *
 * All menu options are available on every platform; "Quit" is added only on
 * desktop builds.
 */
class MainComponent final : public juce::Component
                          , public juce::MenuBarModel
{
public:
    using AudioToggleCallback  = std::function<void(bool /*shouldPlay*/)>;
    using AudioFilterCallback  = std::function<void(float /*cutoff 0-100*/)>;
    using AudioGainCallback    = std::function<void(float /*gain 0-1*/)>;

    MainComponent(AudioToggleCallback onToggle,
                  AudioFilterCallback onFilter,
                  AudioGainCallback   onGain);

    ~MainComponent() override;

    // -----------------------------------------------------------------------
    //  juce::MenuBarModel interface
    // -----------------------------------------------------------------------

    [[nodiscard]] juce::StringArray getMenuBarNames() override;

    [[nodiscard]] juce::PopupMenu getMenuForIndex(int topLevelMenuIndex,
                                                  const juce::String& menuName) override;

    void menuItemSelected(int menuItemID, int topLevelMenuIndex) override;

    // -----------------------------------------------------------------------
    //  juce::Component interface
    // -----------------------------------------------------------------------

    void resized() override;
    void paint(juce::Graphics& g) override;

private:
    // -----------------------------------------------------------------------
    //  Setup helpers
    // -----------------------------------------------------------------------

    void setupPlayButton();
    void setupDiscreteSlider();
    void setupContinuousSlider();
    void setupValueBox();
    void setupLabels();
    void setupMobileMenuButton();

#if ! (JUCE_IOS || JUCE_ANDROID)
    void setupVolumeSlider();
#endif

    // -----------------------------------------------------------------------
    //  Painting
    // -----------------------------------------------------------------------

    void paintImageArea(juce::Graphics& g);

    // -----------------------------------------------------------------------
    //  Value-box / slider synchronisation
    // -----------------------------------------------------------------------

    void syncValueBox();
    void applyValueBox();

#if ! (JUCE_IOS || JUCE_ANDROID)
    void syncVolumeValueBox();
    void applyVolumeValueBox();
#endif

    // -----------------------------------------------------------------------
    //  Preset I/O
    // -----------------------------------------------------------------------

    void savePreset();
    void loadPreset();

#if JUCE_IOS || JUCE_ANDROID
    void showMobileMenu();
#endif

    // -----------------------------------------------------------------------
    //  Data members (declaration order matches dependency chain;
    //  destruction is in reverse order — see LIFO note in EnagaApplication)
    // -----------------------------------------------------------------------

    AudioToggleCallback audioToggle;
    AudioFilterCallback audioFilter;
    AudioGainCallback   audioGain;

    // Default values used at construction and as fallbacks when loading presets.
    static constexpr double defaultDiscreteValue   = 3.0;
    static constexpr double defaultContinuousValue = 100.0;
    static constexpr double defaultVolumeValue     = 100.0;

    PlayButton       playButton;
    juce::Slider     discreteSlider;
    juce::Slider     continuousSlider;
    juce::TextEditor continuousValueBox;
    juce::Label      discreteLabel;
    juce::Label      continuousLabel;

#if ! (JUCE_IOS || JUCE_ANDROID)
    juce::Slider     volumeSlider;
    juce::Label      volumeLabel;
    juce::TextEditor volumeValueBox;
#endif

    juce::Rectangle<int> imageArea;

#if JUCE_IOS || JUCE_ANDROID
    juce::TextButton mobileMenuButton;
#else
    static constexpr int menuBarHeight = 24;
    juce::MenuBarComponent menuBar;
#endif

    std::unique_ptr<juce::FileChooser> fileChooser;
};
