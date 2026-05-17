/**
 * @file   editor_actions_test.cpp
 * @brief  Unit tests for EnagaEditorActions callback mapping.
 */

#include "editor_actions.h"

namespace {

class FakeProcessorControls final : public EnagaProcessorControls {
 public:
  void SetCutoff(float normalised_0_to_100) noexcept override {
    last_cutoff = normalised_0_to_100;
    set_cutoff_calls++;
  }

  void SetGain(float new_gain) noexcept override {
    last_gain = new_gain;
    set_gain_calls++;
  }

  void SetNoiseType(NoiseType type) noexcept override {
    last_noise_type = type;
    set_noise_type_calls++;
  }

  void SetLfoRate(float rate_hz) noexcept override {
    last_lfo_rate = rate_hz;
    set_lfo_rate_calls++;
  }

  void SetLfoIntensity(float intensity) noexcept override {
    last_lfo_intensity = intensity;
    set_lfo_intensity_calls++;
  }

  void SetLfoMode(LfoMode mode) noexcept override {
    last_lfo_mode = mode;
    set_lfo_mode_calls++;
  }

  void StartFadeIn() noexcept override { fade_in_calls++; }

  void StartFadeOut() noexcept override { fade_out_calls++; }

  float last_cutoff = 0.0f;
  float last_gain = 0.0f;
  NoiseType last_noise_type = NoiseType::kWhite;
  float last_lfo_rate = 0.0f;
  float last_lfo_intensity = 0.0f;
  LfoMode last_lfo_mode = LfoMode::kDisabled;

  int set_cutoff_calls = 0;
  int set_gain_calls = 0;
  int set_noise_type_calls = 0;
  int set_lfo_rate_calls = 0;
  int set_lfo_intensity_calls = 0;
  int set_lfo_mode_calls = 0;
  int fade_in_calls = 0;
  int fade_out_calls = 0;
};

bool RunEditorActionsRoutingTest() {
  FakeProcessorControls controls;
  EnagaEditorActions actions(controls);

  actions.HandlePlayToggle(true);
  actions.HandlePlayToggle(false);
  actions.HandleCutoff(75.0f);
  actions.HandleGain(0.5f);
  actions.HandleLfoRate(1.25f);
  actions.HandleLfoIntensity(65.0f);
  actions.HandleLfoMode(LfoMode::kFilter);

  return controls.fade_in_calls == 1 && controls.fade_out_calls == 1 &&
         controls.set_cutoff_calls == 1 && controls.last_cutoff == 75.0f &&
         controls.set_gain_calls == 1 && controls.last_gain == 0.5f &&
         controls.set_lfo_rate_calls == 1 && controls.last_lfo_rate == 1.25f &&
         controls.set_lfo_intensity_calls == 1 &&
         controls.last_lfo_intensity == 65.0f &&
         controls.set_lfo_mode_calls == 1 &&
         controls.last_lfo_mode == LfoMode::kFilter;
}

bool RunEditorActionsNoiseTypeClampTest() {
  FakeProcessorControls controls;
  EnagaEditorActions actions(controls);

  actions.HandleNoiseTypeSelection(0.0f);
  const bool lower_bound_ok = controls.last_noise_type == NoiseType::kWhite;

  actions.HandleNoiseTypeSelection(2.0f);
  const bool middle_value_ok = controls.last_noise_type == NoiseType::kPink;

  actions.HandleNoiseTypeSelection(9.0f);
  const bool upper_bound_ok = controls.last_noise_type == NoiseType::kGrey;

  return controls.set_noise_type_calls == 3 && lower_bound_ok &&
         middle_value_ok && upper_bound_ok;
}

}  // namespace

bool RunEditorActionsTests() {
  return RunEditorActionsRoutingTest() && RunEditorActionsNoiseTypeClampTest();
}
