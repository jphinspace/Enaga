/**
 * @file   editor_actions.cpp
 * @brief  EnagaEditorActions implementation.
 */

#include "editor_actions.h"

#include <algorithm>
#include <array>

void EnagaEditorActions::HandlePlayToggle(bool should_play) noexcept {
  if (should_play) {
    controls_.StartFadeIn();
    return;
  }

  controls_.StartFadeOut();
}

void EnagaEditorActions::HandleCutoff(float cutoff) noexcept {
  controls_.SetCutoff(cutoff);
}

void EnagaEditorActions::HandleGain(float gain) noexcept {
  controls_.SetGain(gain);
}

void EnagaEditorActions::HandleNoiseTypeSelection(
    float selected_index) noexcept {
  constexpr std::array kNoiseTypes = {
      NoiseType::kWhite,
      NoiseType::kPink,
      NoiseType::kBrown,
      NoiseType::kGrey,
  };
  const int clamped_index = std::clamp(static_cast<int>(selected_index), 1, 4);
  controls_.SetNoiseType(
      kNoiseTypes[static_cast<std::size_t>(clamped_index - 1)]);
}

void EnagaEditorActions::HandleLfoRate(float rate_hz) noexcept {
  controls_.SetLfoRate(rate_hz);
}

void EnagaEditorActions::HandleLfoIntensity(float intensity) noexcept {
  controls_.SetLfoIntensity(intensity);
}

void EnagaEditorActions::HandleLfoMode(LfoMode mode) noexcept {
  controls_.SetLfoMode(mode);
}
