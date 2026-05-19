/**
 * @file   lfo_component.cpp
 * @brief  LfoComponent implementation.
 */

#include "ui/lfo_component.h"

#include <array>
#include <utility>

namespace {
constexpr std::array<LfoMode, 4> kLfoModeCycle = {
    LfoMode::kDisabled,
    LfoMode::kVolume,
    LfoMode::kFilter,
    LfoMode::kBoth,
};

static_assert(std::to_underlying(LfoMode::kDisabled) == 0);
static_assert(std::to_underlying(LfoMode::kVolume) == 1);
static_assert(std::to_underlying(LfoMode::kFilter) == 2);
static_assert(std::to_underlying(LfoMode::kBoth) == 3);

constexpr std::array<const char*, kLfoModeCycle.size()> kLfoModeLabels = {
    "LFO: Disabled",
    "LFO: Volume",
    "LFO: Filter",
    "LFO: Both",
};

[[nodiscard]] std::size_t ToModeIndex(LfoMode mode) {
  const auto mode_index = static_cast<std::size_t>(std::to_underlying(mode));
  if (mode_index < kLfoModeCycle.size()) {
    jassert(kLfoModeCycle[mode_index] == mode);
    jassert(mode_index < kLfoModeCycle.size());
    return mode_index;
  }
  jassertfalse;
  return 0;
}
}  // namespace

LfoComponent::LfoComponent(RateCallback on_rate, IntensityCallback on_intensity,
                           ModeCallback on_mode)
    : on_rate_(std::move(on_rate)),
      on_intensity_(std::move(on_intensity)),
      on_mode_(std::move(on_mode)) {
  SetupModeButton();
  SetupRateSlider();
  SetupRateValueBox();
  SetupIntensitySlider();
  SetupIntensityValueBox();
  SetupLabels();
}

void LfoComponent::resized() {
  auto area = getLocalBounds();
  constexpr int kPad = 6;
  constexpr int kLabelW = 88;
  constexpr int kBoxW = 64;

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
  mode_button_.setButtonText(kLfoModeLabels[0]);
  mode_button_.onClick = [this] {
    const std::size_t mode_index = ToModeIndex(current_mode_);
    current_mode_ = kLfoModeCycle[(mode_index + 1) % kLfoModeCycle.size()];
    UpdateModeButtonText();
    if (on_mode_) on_mode_(current_mode_);
  };
  addAndMakeVisible(mode_button_);
}

void LfoComponent::SetupRateSlider() {
  rate_slider_.setSliderStyle(juce::Slider::LinearHorizontal);
  rate_slider_.setTextBoxStyle(juce::Slider::NoTextBox, false, 0, 0);
  rate_slider_.setRange(0.01, 2.0);
  // Skew so that 0.2 Hz (a typical ocean-wave rate) sits at mid-travel.
  rate_slider_.setSkewFactorFromMidPoint(0.2);
  rate_slider_.setValue(0.1, juce::dontSendNotification);
  rate_slider_.onValueChange = [this] {
    SyncRateValueBox();
    if (on_rate_) on_rate_(static_cast<float>(rate_slider_.getValue()));
  };
  addAndMakeVisible(rate_slider_);
}

void LfoComponent::SetupRateValueBox() {
  // Allow only numeric input.
  rate_value_box_.setInputRestrictions(6, "0123456789.");
  rate_value_box_.setText(juce::String(rate_slider_.getValue(), 2), false);
  rate_value_box_.setJustification(juce::Justification::centred);
  rate_value_box_.onReturnKey = [this] { ApplyRateValueBox(); };
  rate_value_box_.onFocusLost = [this] { ApplyRateValueBox(); };
  addAndMakeVisible(rate_value_box_);
}

void LfoComponent::SetupIntensitySlider() {
  intensity_slider_.setSliderStyle(juce::Slider::LinearHorizontal);
  intensity_slider_.setTextBoxStyle(juce::Slider::NoTextBox, false, 0, 0);
  intensity_slider_.setRange(0.0, 100.0);
  intensity_slider_.setValue(0.0, juce::dontSendNotification);
  intensity_slider_.onValueChange = [this] {
    SyncIntensityValueBox();
    if (on_intensity_)
      on_intensity_(static_cast<float>(intensity_slider_.getValue()));
  };
  addAndMakeVisible(intensity_slider_);
}

void LfoComponent::SetupIntensityValueBox() {
  // Allow only numeric input.
  intensity_value_box_.setInputRestrictions(6, "0123456789.");
  intensity_value_box_.setText("0.0", false);
  intensity_value_box_.setJustification(juce::Justification::centred);
  intensity_value_box_.onReturnKey = [this] { ApplyIntensityValueBox(); };
  intensity_value_box_.onFocusLost = [this] { ApplyIntensityValueBox(); };
  addAndMakeVisible(intensity_value_box_);
}

void LfoComponent::SetupLabels() {
  rate_label_.setText("LFO Rate", juce::dontSendNotification);
  rate_label_.setJustificationType(juce::Justification::centredLeft);
  addAndMakeVisible(rate_label_);

  intensity_label_.setText("LFO Intensity", juce::dontSendNotification);
  intensity_label_.setJustificationType(juce::Justification::centredLeft);
  addAndMakeVisible(intensity_label_);
}

void LfoComponent::UpdateModeButtonText() {
  mode_button_.setButtonText(kLfoModeLabels[ToModeIndex(current_mode_)]);
}

void LfoComponent::SyncRateValueBox() {
  rate_value_box_.setText(juce::String(rate_slider_.getValue(), 2), false);
}

void LfoComponent::ApplyRateValueBox() {
  const double v =
      juce::jlimit(0.01, 2.0, rate_value_box_.getText().getDoubleValue());
  rate_slider_.setValue(v, juce::sendNotificationSync);
  SyncRateValueBox();
}

void LfoComponent::SyncIntensityValueBox() {
  intensity_value_box_.setText(juce::String(intensity_slider_.getValue(), 1),
                               false);
}

void LfoComponent::ApplyIntensityValueBox() {
  const double v =
      juce::jlimit(0.0, 100.0, intensity_value_box_.getText().getDoubleValue());
  intensity_slider_.setValue(v, juce::sendNotificationSync);
  SyncIntensityValueBox();
}
