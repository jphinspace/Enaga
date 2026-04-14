/**
 * @file   pink_noise_generator.h
 * @brief  Pink noise generator – 1/f spectrum (−3 dB/octave).
 */

#pragma once

#include "audio/generators/noise_generator.h"

#include <juce_audio_basics/juce_audio_basics.h>

#include <array>

/**
 * Produces pink (1/f) noise using Paul Kellett's parallel-filter algorithm.
 *
 * Seven first-order IIR low-pass filters with geometrically-spaced poles
 * are summed to give an approximate −3 dB/octave power spectrum. Independent
 * state is maintained for each stereo channel so left and right are
 * decorrelated.
 */
class PinkNoiseGenerator final : public NoiseGenerator {
 public:
  void Prepare(double sample_rate) override;
  void Reset() noexcept override;
  [[nodiscard]] float NextSample(std::size_t channel) noexcept override;

 private:
  juce::Random random_;
  // 7 filter-state variables per stereo channel (Paul Kellett algorithm).
  std::array<std::array<float, 7>, 2> state_ {};
};
