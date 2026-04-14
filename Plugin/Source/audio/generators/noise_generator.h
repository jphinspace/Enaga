/**
 * @file   noise_generator.h
 * @brief  Abstract base class for per-sample noise generators.
 */

#pragma once

#include <cstddef>

/**
 * Interface for a single-sample stereo noise generator.
 *
 * Implementations hold only their own DSP state (filter history, running
 * integrals, etc.) and are called exclusively from the audio thread.
 */
class NoiseGenerator {
 public:
  virtual ~NoiseGenerator() = default;

  /** Called when the audio device opens or the sample rate changes. */
  virtual void Prepare(double sample_rate) = 0;

  /** Reset all internal filter / integration state to zero. */
  virtual void Reset() noexcept = 0;

  /**
   * Return the next output sample for the given stereo channel index
   * (0 or 1). Must only be called on the audio thread.
   */
  [[nodiscard]] virtual float NextSample(
      std::size_t channel) noexcept = 0;
};
