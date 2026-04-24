/**
 * @file   white_noise_generator.h
 * @brief  White noise generator – flat frequency spectrum.
 */

#ifndef ENAGA_AUDIO_GENERATORS_WHITE_NOISE_GENERATOR_H_
#define ENAGA_AUDIO_GENERATORS_WHITE_NOISE_GENERATOR_H_

#include <juce_audio_basics/juce_audio_basics.h>

#include "audio/generators/noise_generator.h"

/**
 * Produces uniformly-distributed white noise with a flat power spectrum.
 * No per-channel DSP state is needed; the PRNG feeds all channels equally.
 */
class WhiteNoiseGenerator final : public NoiseGenerator {
 public:
  void Prepare(double sample_rate) override;
  void Reset() noexcept override;
  [[nodiscard]] float NextSample(std::size_t channel) noexcept override;

 private:
  juce::Random random_;
};

#endif  // ENAGA_AUDIO_GENERATORS_WHITE_NOISE_GENERATOR_H_
