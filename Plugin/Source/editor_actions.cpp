/**
 * @file   editor_actions.cpp
 * @brief  EnagaEditorActions implementation.
 */

#include "editor_actions.h"

#include <algorithm>
#include <array>
#include <cstddef>
#include <utility>

namespace {
template <std::size_t kCount>
consteval bool HasZeroBasedContiguousOrdinals(
    const std::array<NoiseType, kCount>& values) {
  if (std::to_underlying(values.front()) != 0) {
    return false;
  }

  for (std::size_t i = 1; i < values.size(); ++i) {
    if (std::to_underlying(values[i]) !=
        std::to_underlying(values[i - 1]) + 1) {
      return false;
    }
  }

  return true;
}
}  // namespace

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
  static_assert(HasZeroBasedContiguousOrdinals(kNoiseTypes));

  // Slider positions are 1..N in the UI.
  constexpr int kUiSelectionBase = 1;
  constexpr int kMaxSelection =
      kUiSelectionBase + static_cast<int>(kNoiseTypes.size()) - 1;

  const int clamped_index = std::clamp(static_cast<int>(selected_index),
                                       kUiSelectionBase, kMaxSelection);
  const auto noise_type_index =
      static_cast<std::size_t>(clamped_index - kUiSelectionBase);
  controls_.SetNoiseType(kNoiseTypes[noise_type_index]);
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
