/**
 * @file   lfo_engine.h
 * @brief  Thread-safe, sine-wave low-frequency oscillator used to modulate
 *         volume and/or low-pass filter cutoff within user-set bounds.
 *
 * Rate, intensity, and mode are written by the UI thread via lock-free
 * atomics. Phase is maintained exclusively on the audio thread.
 *
 * Usage (audio thread):
 * @code
 *   // Once per buffer:
 *   const float lfo_norm = engine.Tick(num_samples, sample_rate); // [0,1]
 *   const float modulated = engine.ApplyToMax(max_value, lfo_norm);
 * @endcode
 */

#ifndef ENAGA_AUDIO_LFO_ENGINE_H_
#define ENAGA_AUDIO_LFO_ENGINE_H_

#include <atomic>

#include "audio/lfo_mode.h"

/**
 * Sine-wave LFO oscillator with lock-free parameter updates.
 *
 * The LFO produces a normalised sine value in [0, 1] where 1 is the peak
 * (full value) and 0 is the trough (most reduced value). Intensity maps
 * 0–100 → no modulation–full modulation: at intensity 100 the output
 * swings from the user's maximum value all the way down to zero.
 */
class LfoEngine final {
 public:
  /**
   * Advance the LFO phase by @p num_samples and return the LFO value at
   * the start of the block, normalised to [0, 1].
   *
   * Must be called exclusively from the audio thread.
   *
   * @param num_samples  Number of samples in the current audio buffer.
   * @param sample_rate  Current hardware sample rate in Hz.
   * @returns            Normalised LFO value: 1 = peak, 0 = trough.
   */
  [[nodiscard]] float Tick(int num_samples, double sample_rate) noexcept;

  /**
   * Apply LFO modulation to a user-defined maximum value.
   *
   * @param max_val   The ceiling (set by the user's slider); never exceeded.
   * @param lfo_norm  Normalised LFO value from Tick() in [0, 1].
   * @returns         A value in [max_val * (1 - intensity/100), max_val].
   */
  [[nodiscard]] float ApplyToMax(float max_val, float lfo_norm) const noexcept;

  /**
   * Set the LFO oscillation rate.
   * @param rate_hz  Rate in Hz; clamped to [0.01, 2.0].
   */
  void SetRate(float rate_hz) noexcept;

  /**
   * Set the LFO modulation depth.
   * @param intensity  Intensity 0–100; 0 = no modulation, 100 = full swing.
   */
  void SetIntensity(float intensity) noexcept;

  /** Set which audio parameter(s) the LFO modulates. */
  void SetMode(LfoMode mode) noexcept;

  /** Return the current LFO mode. */
  [[nodiscard]] LfoMode GetMode() const noexcept;

  /** Return the current intensity in [0, 100]. */
  [[nodiscard]] float GetIntensity() const noexcept;

  /** Reset the LFO phase to zero (audio thread). */
  void Reset() noexcept;

 private:
  std::atomic<float> rate_{0.1f};       // Hz; clamped to [0.01, 2.0]
  std::atomic<float> intensity_{0.0f};  // 0–100
  std::atomic<int> mode_{0};            // cast to LfoMode; UI thread writes
  double phase_{0.0};                   // radians; audio thread only
};

#endif  // ENAGA_AUDIO_LFO_ENGINE_H_
