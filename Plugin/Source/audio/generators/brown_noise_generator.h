/**
 * @file   brown_noise_generator.h
 * @brief  Brown (Brownian / red) noise generator – 1/f² spectrum
 *         (−6 dB/octave).
 */

#pragma once

#include <juce_audio_basics/juce_audio_basics.h>

#include <array>

#include "audio/generators/noise_generator.h"

/**
 * Produces brown noise by leaky first-order IIR integration of white noise.
 *
 * The leaky integrator prevents DC drift while preserving the −6 dB/octave
 * power spectrum. Output is normalised with ×3.5 and hard-clipped to
 * [−1, 1]. Independent state is maintained for each stereo channel.
 */
class BrownNoiseGenerator final : public NoiseGenerator {
 public:
  void Prepare(double sample_rate) override;
  void Reset() noexcept override;
  [[nodiscard]] float NextSample(std::size_t channel) noexcept override;

 private:
  juce::Random random_;
  std::array<float, 2> state_{};  // running integration value per channel
};
