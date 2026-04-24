/**
 * @file   main_component.h
 * @brief  Root UI component for the Enaga application window.
 */

#ifndef ENAGA_UI_MAIN_COMPONENT_H_
#define ENAGA_UI_MAIN_COMPONENT_H_

#include "ui/lfo_component.h"
#include "ui/play_button.h"

#if JUCE_IOS
#include "platform/ios_volume_view.h"
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
 *      The discrete slider selects the noise colour
 *      (White / Pink / Brown / Grey).
 *   4. "Cutoff" label  +  continuous slider  +  numeric text box
 *      (0.0–100.0).
 *   5. "Volume" label  +  volume control (all platforms):
 *        - Desktop (macOS/Windows/Linux): juce::Slider driving app-level
 *          gain.
 *        - iOS:     MPVolumeView (native system volume slider via
 *          UIViewComponent).
 *        - Android: juce::Slider wired to juce::SystemAudioVolume.
 *
 * Platform-specific navigation:
 *   - Desktop (macOS/Windows/Linux) : juce::MenuBarComponent at the top.
 *   - iOS                           : Gear icon button (⚙) top-right.
 *   - Android                       : Hamburger button (☰) top-left.
 *
 * All menu options are available on every platform; "Quit" is added only
 * on desktop builds.
 */
class MainComponent final : public juce::Component, public juce::MenuBarModel {
 public:
  using AudioToggleCallback = std::function<void(bool)>;
  using AudioFilterCallback = std::function<void(float)>;
  using AudioGainCallback = std::function<void(float)>;
  using AudioNoiseTypeCallback = std::function<void(float)>;
  using AudioLfoRateCallback = std::function<void(float)>;
  using AudioLfoIntensityCallback = std::function<void(float)>;
  using AudioLfoModeCallback = std::function<void(LfoMode)>;

  /** All audio-layer callbacks bundled into a single struct.
   *
   *  Passing one struct instead of seven individual parameters keeps
   *  constructor arity low and means adding a new callback only requires
   *  changing this struct definition rather than every call site.
   */
  struct AudioCallbacks {
    AudioToggleCallback on_toggle;
    AudioFilterCallback on_filter;
    AudioGainCallback on_gain;
    AudioNoiseTypeCallback on_noise_type;
    AudioLfoRateCallback on_lfo_rate;
    AudioLfoIntensityCallback on_lfo_intensity;
    AudioLfoModeCallback on_lfo_mode;
  };

  explicit MainComponent(AudioCallbacks callbacks);

  ~MainComponent() override;

  // -------------------------------------------------------------------
  //  juce::MenuBarModel interface
  // -------------------------------------------------------------------

  [[nodiscard]] juce::StringArray getMenuBarNames() override;

  [[nodiscard]] juce::PopupMenu getMenuForIndex(
      int top_level_menu_index, const juce::String& menu_name) override;

  void menuItemSelected(int menu_item_id, int top_level_menu_index) override;

  // -------------------------------------------------------------------
  //  juce::Component interface
  // -------------------------------------------------------------------

  void resized() override;
  void paint(juce::Graphics& g) override;

 private:
  // -------------------------------------------------------------------
  //  Setup helpers
  // -------------------------------------------------------------------

  void SetupPlayButton();
  void SetupDiscreteSlider();
  void SetupContinuousSlider();
  void SetupValueBox();
  void SetupLabels();
  void SetupMobileMenuButton();
  void SetupVolumeSlider();

  // -------------------------------------------------------------------
  //  Painting
  // -------------------------------------------------------------------

  void PaintImageArea(juce::Graphics& g);

  // -------------------------------------------------------------------
  //  Value-box / slider synchronisation
  // -------------------------------------------------------------------

  void SyncValueBox();
  void ApplyValueBox();

  // -------------------------------------------------------------------
  //  Preset I/O
  // -------------------------------------------------------------------

  void SavePreset();
  void LoadPreset();

#if JUCE_IOS || JUCE_ANDROID
  void ShowMobileMenu();
#endif

  // -------------------------------------------------------------------
  //  Data members (declaration order matches dependency chain;
  //  destruction is in reverse order)
  // -------------------------------------------------------------------

  AudioCallbacks callbacks_;

  // Default values used at construction and as fallbacks for presets.
  static constexpr double kDefaultDiscreteValue = 1.0;  // White noise
  static constexpr double kDefaultContinuousValue = 100.0;
  static constexpr double kDefaultVolumeValue = 100.0;

  PlayButton play_button_;
  juce::Slider discrete_slider_;
  juce::Slider continuous_slider_;
  juce::TextEditor continuous_value_box_;
  juce::Label discrete_label_;
  juce::Label continuous_label_;

  // Volume control: platform-specific widget + shared label.
  // iOS     – MPVolumeView (native system slider; no text box needed).
  // Android – juce::Slider wired to juce::SystemAudioVolume.
  // Desktop – juce::Slider driving app-level gain.
#if JUCE_IOS
  IOSVolumeView mobile_volume_view_;
#else
  juce::Slider volume_slider_;
#endif
  juce::Label volume_label_;

  juce::Rectangle<int> image_area_;

  // LFO section — below the existing volume/cutoff controls.
  LfoComponent lfo_component_;

#if JUCE_IOS || JUCE_ANDROID
  juce::TextButton mobile_menu_button_;
#else
  static constexpr int kMenuBarHeight = 24;
  juce::MenuBarComponent menu_bar_;
#endif

  std::unique_ptr<juce::FileChooser> file_chooser_;
};

#endif  // ENAGA_UI_MAIN_COMPONENT_H_
