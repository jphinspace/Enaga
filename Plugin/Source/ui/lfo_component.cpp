/**
 * @file   lfo_component.cpp
 * @brief  LfoComponent implementation.
 */

#include "ui/lfo_component.h"

LfoComponent::LfoComponent(RateCallback on_rate,
                           IntensityCallback on_intensity,
                           ModeCallback on_mode)
    : on_rate_      (std::move(on_rate))
    , on_intensity_ (std::move(on_intensity))
    , on_mode_      (std::move(on_mode)) {
  SetupModeButton();
  SetupRateSlider();
  SetupRateValueBox();
  SetupIntensitySlider();
  SetupIntensityValueBox();
  SetupLabels();
}

void LfoComponent::resized() {
  auto area = getLocalBounds();
  constexpr int kPad    = 6;
  constexpr int kLabelW = 88;
  constexpr int kBoxW   = 64;

  const int row_h = (area.getHeight() - 2 * kPad) / 3;

  // Row 1: mode button (full width).
  mode_button_.setBounds(area.removeFromTop(row_h));
  area.removeFromTop(kPad);

  // Row 2: rate label + rate slider + rate value box.
  auto row2 = area.removeFromTop(row_h);
  area.removeFromTop(kPad);
  rate_label_.setBounds(row2.removeFromLeft(kLabelW));
  rate_value_box_.setBounds(row2.removeFromRight(kBoxW));
  row2.removeFromRight(4);
  rate_slider_.setBounds(row2);

  // Row 3: intensity label + intensity slider + value box.
  auto row3 = area.removeFromTop(row_h);
  intensity_label_.setBounds(row3.removeFromLeft(kLabelW));
  intensity_value_box_.setBounds(row3.removeFromRight(kBoxW));
  row3.removeFromRight(4);
  intensity_slider_.setBounds(row3);
}

void LfoComponent::SetupModeButton() {
  mode_button_.setButtonText("LFO: Disabled");
  mode_button_.onClick = [this] {
    // Cycle: kDisabled → kVolume → kFilter → kBoth → kDisabled
    switch (current_mode_) {
      case LfoMode::kDisabled: current_mode_ = LfoMode::kVolume;   break;
      case LfoMode::kVolume:   current_mode_ = LfoMode::kFilter;   break;
      case LfoMode::kFilter:   current_mode_ = LfoMode::kBoth;     break;
      case LfoMode::kBoth:     current_mode_ = LfoMode::kDisabled; break;
    }
    UpdateModeButtonText();
    if (on_mode_) on_mode_(current_mode_);
  };
  addAndMakeVisible(mode_button_);
}

void LfoComponent::SetupRateSlider() {
  rate_slider_.setSliderStyle(juce::Slider::LinearHorizontal);
  rate_slider_.setTextBoxStyle(
      juce::Slider::NoTextBox, false, 0, 0);
  rate_slider_.setRange(0.01, 2.0);
  // Skew so that 0.2 Hz (a typical ocean-wave rate) sits at mid-travel.
  rate_slider_.setSkewFactorFromMidPoint(0.2);
  rate_slider_.setValue(0.1, juce::dontSendNotification);
  rate_slider_.onValueChange = [this] {
    SyncRateValueBox();
    if (on_rate_)
      on_rate_(static_cast<float>(rate_slider_.getValue()));
  };
  addAndMakeVisible(rate_slider_);
}

void LfoComponent::SetupRateValueBox() {
  // Allow only numeric input.
  rate_value_box_.setInputRestrictions(6, "0123456789.");
  rate_value_box_.setText(
      juce::String(rate_slider_.getValue(), 2), false);
  rate_value_box_.setJustification(juce::Justification::centred);
  rate_value_box_.onReturnKey = [this] { ApplyRateValueBox(); };
  rate_value_box_.onFocusLost = [this] { ApplyRateValueBox(); };
  addAndMakeVisible(rate_value_box_);
}

void LfoComponent::SetupIntensitySlider() {
  intensity_slider_.setSliderStyle(juce::Slider::LinearHorizontal);
  intensity_slider_.setTextBoxStyle(
      juce::Slider::NoTextBox, false, 0, 0);
  intensity_slider_.setRange(0.0, 100.0);
  intensity_slider_.setValue(0.0, juce::dontSendNotification);
  intensity_slider_.onValueChange = [this] {
    SyncIntensityValueBox();
    if (on_intensity_)
      on_intensity_(
          static_cast<float>(intensity_slider_.getValue()));
  };
  addAndMakeVisible(intensity_slider_);
}

void LfoComponent::SetupIntensityValueBox() {
  // Allow only numeric input.
  intensity_value_box_.setInputRestrictions(6, "0123456789.");
  intensity_value_box_.setText("0.0", false);
  intensity_value_box_.setJustification(
      juce::Justification::centred);
  intensity_value_box_.onReturnKey = [this] {
    ApplyIntensityValueBox();
  };
  intensity_value_box_.onFocusLost = [this] {
    ApplyIntensityValueBox();
  };
  addAndMakeVisible(intensity_value_box_);
}

void LfoComponent::SetupLabels() {
  rate_label_.setText("LFO Rate", juce::dontSendNotification);
  rate_label_.setJustificationType(
      juce::Justification::centredLeft);
  addAndMakeVisible(rate_label_);

  intensity_label_.setText("LFO Intensity",
                            juce::dontSendNotification);
  intensity_label_.setJustificationType(
      juce::Justification::centredLeft);
  addAndMakeVisible(intensity_label_);
}

void LfoComponent::UpdateModeButtonText() {
  const char* text = "LFO: Disabled";
  switch (current_mode_) {
    case LfoMode::kVolume: text = "LFO: Volume"; break;
    case LfoMode::kFilter: text = "LFO: Filter"; break;
    case LfoMode::kBoth:   text = "LFO: Both";   break;
    default:                                      break;
  }
  mode_button_.setButtonText(text);
}

void LfoComponent::SyncRateValueBox() {
  rate_value_box_.setText(
      juce::String(rate_slider_.getValue(), 2), false);
}

void LfoComponent::ApplyRateValueBox() {
  const double v = juce::jlimit(
      0.01, 2.0, rate_value_box_.getText().getDoubleValue());
  rate_slider_.setValue(v, juce::sendNotificationSync);
  SyncRateValueBox();
}

void LfoComponent::SyncIntensityValueBox() {
  intensity_value_box_.setText(
      juce::String(intensity_slider_.getValue(), 1), false);
}

void LfoComponent::ApplyIntensityValueBox() {
  const double v = juce::jlimit(
      0.0, 100.0,
      intensity_value_box_.getText().getDoubleValue());
  intensity_slider_.setValue(v, juce::sendNotificationSync);
  SyncIntensityValueBox();
}
