/**
 * @file   MainComponent.h
 * @brief  Root UI component for the Enaga application window.
 */

#pragma once

#include "LfoComponent.h"
#include "PlayButton.h"

#if JUCE_IOS
    #include "iOSVolumeView.h"
#endif

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
 *      The discrete slider selects the noise colour (White / Pink / Brown / Grey).
 *   4. "Cutoff" label  +  continuous slider  +  numeric text box (0.0–100.0).
 *   5. "Volume" label  +  volume control (all platforms):
 *        - Desktop (macOS/Windows/Linux): juce::Slider driving app-level gain.
 *        - iOS:     MPVolumeView (native system volume slider via UIViewComponent).
 *        - Android: juce::Slider wired to juce::SystemAudioVolume.
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
    using AudioToggleCallback       = std::function<void(bool /*shouldPlay*/)>;
    using AudioFilterCallback       = std::function<void(float /*cutoff 0-100*/)>;
    using AudioGainCallback         = std::function<void(float /*gain 0-1*/)>;
    using AudioNoiseTypeCallback    = std::function<void(float /*discreteValue 1.0-4.0*/)>;
    using AudioLfoRateCallback      = std::function<void(float /*rateHz*/)>;
    using AudioLfoIntensityCallback = std::function<void(float /*0-100*/)>;
    using AudioLfoModeCallback      = std::function<void(LfoMode)>;

    MainComponent(AudioToggleCallback       onToggle,
                  AudioFilterCallback       onFilter,
                  AudioGainCallback         onGain,
                  AudioNoiseTypeCallback    onNoiseType,
                  AudioLfoRateCallback      onLfoRate,
                  AudioLfoIntensityCallback onLfoIntensity,
                  AudioLfoModeCallback      onLfoMode);

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
    void setupVolumeSlider();

    // -----------------------------------------------------------------------
    //  Painting
    // -----------------------------------------------------------------------

    void paintImageArea(juce::Graphics& g);

    // -----------------------------------------------------------------------
    //  Value-box / slider synchronisation
    // -----------------------------------------------------------------------

    void syncValueBox();
    void applyValueBox();

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

    AudioToggleCallback       audioToggle;
    AudioFilterCallback       audioFilter;
    AudioGainCallback         audioGain;
    AudioNoiseTypeCallback    audioNoiseType;
    AudioLfoRateCallback      audioLfoRate;
    AudioLfoIntensityCallback audioLfoIntensity;
    AudioLfoModeCallback      audioLfoMode;

    // Default values used at construction and as fallbacks when loading presets.
    static constexpr double defaultDiscreteValue   = 1.0;  // White noise
    static constexpr double defaultContinuousValue = 100.0;
    static constexpr double defaultVolumeValue     = 100.0;

    PlayButton       playButton;
    juce::Slider     discreteSlider;
    juce::Slider     continuousSlider;
    juce::TextEditor continuousValueBox;
    juce::Label      discreteLabel;
    juce::Label      continuousLabel;

    // Volume control: platform-specific widget + shared label.
    // iOS     – MPVolumeView (native system slider; no text box needed).
    // Android – juce::Slider wired to juce::SystemAudioVolume.
    // Desktop – juce::Slider driving app-level gain.
#if JUCE_IOS
    iOSVolumeView    mobileVolumeView;
#else
    juce::Slider     volumeSlider;
#endif
    juce::Label      volumeLabel;

    juce::Rectangle<int> imageArea;

    // LFO section — below the existing volume/cutoff controls.
    LfoComponent lfoComponent;

#if JUCE_IOS || JUCE_ANDROID
    juce::TextButton mobileMenuButton;
#else
    static constexpr int menuBarHeight = 24;
    juce::MenuBarComponent menuBar;
#endif

    std::unique_ptr<juce::FileChooser> fileChooser;
};
