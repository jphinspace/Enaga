/**
 * @file   lfo_component.h
 * @brief  Self-contained UI component that exposes LFO controls to the user.
 *
 * Provides:
 *   - A mode button that cycles:
 *       kDisabled → kVolume → kFilter → kBoth → kDisabled.
 *   - A "LFO Rate" slider (0.01–2.0 Hz, skewed so 0.2 Hz is at mid-travel)
 *     with a numeric text box.
 *   - An "LFO Intensity" slider (0–100) paired with a numeric text box.
 *
 * All three callbacks fire on the message thread immediately when a control
 * changes value.
 */

#ifndef ENAGA_UI_LFO_COMPONENT_H_
#define ENAGA_UI_LFO_COMPONENT_H_

#include <juce_gui_basics/juce_gui_basics.h>

#include <functional>

#include "audio/lfo_mode.h"

/**
 * UI panel for the LFO section.
 *
 * Place this component inside a parent and call setBounds() from the
 * parent's resized() override. The component manages its own internal
 * layout.
 */
class LfoComponent final : public juce::Component {
 public:
  using RateCallback = std::function<void(float /*rateHz*/)>;
  using IntensityCallback = std::function<void(float /*0-100*/)>;
  using ModeCallback = std::function<void(LfoMode)>;

  LfoComponent(RateCallback on_rate, IntensityCallback on_intensity,
               ModeCallback on_mode);

  void resized() override;

 private:
  // -------------------------------------------------------------------
  //  Setup helpers
  // -------------------------------------------------------------------

  void SetupModeButton();
  void SetupRateSlider();
  void SetupRateValueBox();
  void SetupIntensitySlider();
  void SetupIntensityValueBox();
  void SetupLabels();

  // -------------------------------------------------------------------
  //  Mode button
  // -------------------------------------------------------------------

  void UpdateModeButtonText();

  // -------------------------------------------------------------------
  //  Rate value-box / slider synchronisation
  // -------------------------------------------------------------------

  void SyncRateValueBox();
  void ApplyRateValueBox();

  // -------------------------------------------------------------------
  //  Intensity value-box / slider synchronisation
  // -------------------------------------------------------------------

  void SyncIntensityValueBox();
  void ApplyIntensityValueBox();

  // -------------------------------------------------------------------
  //  Callbacks
  // -------------------------------------------------------------------

  RateCallback on_rate_;
  IntensityCallback on_intensity_;
  ModeCallback on_mode_;

  LfoMode current_mode_{LfoMode::kDisabled};

  // -------------------------------------------------------------------
  //  Child controls (declaration order matches dependency / destruction)
  // -------------------------------------------------------------------

  juce::TextButton mode_button_;
  juce::Label rate_label_;
  juce::Slider rate_slider_;
  juce::TextEditor rate_value_box_;
  juce::Label intensity_label_;
  juce::Slider intensity_slider_;
  juce::TextEditor intensity_value_box_;
};

#endif  // ENAGA_UI_LFO_COMPONENT_H_
