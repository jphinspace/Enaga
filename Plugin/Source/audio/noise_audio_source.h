/**
 * @file   noise_audio_source.h
 * @brief  JUCE AudioSource that delegates sample generation to a pluggable
 *         NoiseGenerator, then applies a low-pass filter, gain, fade, and
 *         LFO.
 */

#ifndef ENAGA_AUDIO_NOISE_AUDIO_SOURCE_H_
#define ENAGA_AUDIO_NOISE_AUDIO_SOURCE_H_

#include <juce_audio_basics/juce_audio_basics.h>

#include <array>
#include <atomic>

#include "audio/generators/brown_noise_generator.h"
#include "audio/generators/grey_noise_generator.h"
#include "audio/generators/pink_noise_generator.h"
#include "audio/generators/white_noise_generator.h"
#include "audio/lfo_engine.h"
#include "audio/noise_type.h"

/**
 * JUCE AudioSource that generates noise of a selectable spectral colour.
 *
 * The noise type, LP-filter cutoff (0–100 normalised → 20 Hz–20 kHz log),
 * gain, and LFO parameters are written by the UI thread via lock-free
 * atomics and applied on the audio thread. All four generator objects are
 * kept alive for the lifetime of this source so that switching types is
 * always safe.
 *
 * On desktop, gain is driven by the Volume slider. On iOS/Android the
 * atomic is left at 1.0f and loudness is handled by the OS media volume.
 *
 * The LFO modulates gain and/or cutoff within the bounds set by the user's
 * existing Volume and Cutoff sliders (it never exceeds those values).
 */
class NoiseAudioSource final : public juce::AudioSource {
 public:
  /** Duration of the fade-in and fade-out ramp in seconds. */
  static constexpr float kFadeDurationSeconds = 0.25f;

  /**
   * Set the low-pass filter cutoff on a normalised 0–100 scale.
   * 0 -> 20 Hz (heavy filtering), 100 -> 20 kHz (wide open).
   * Thread-safe: called from the message thread, applied on the audio
   * thread.
   */
  void SetCutoff(float normalised_0_to_100) noexcept;

  /**
   * Set output amplitude multiplier in [0, 1].
   * Thread-safe: called from the message thread, read on the audio thread.
   */
  void SetGain(float new_gain) noexcept;

  /**
   * Set the spectral colour of the generated noise.
   * Thread-safe: called from the message thread, applied on the audio
   * thread.
   */
  void SetNoiseType(NoiseType type) noexcept;

  // -------------------------------------------------------------------
  //  LFO controls
  // -------------------------------------------------------------------

  /**
   * Set the LFO oscillation rate.
   * @param rate_hz  Rate in Hz; clamped to [0.01, 2.0].
   */
  void SetLfoRate(float rate_hz) noexcept;

  /**
   * Set the LFO modulation depth.
   * @param intensity  Intensity on a 0-100 scale; 0 = no mod, 100 = full
   *                   swing.
   */
  void SetLfoIntensity(float intensity) noexcept;

  /**
   * Set which audio parameter(s) the LFO modulates.
   * The LFO never exceeds the bounds set by the Volume and Cutoff sliders.
   */
  void SetLfoMode(LfoMode mode) noexcept;

  /**
   * Begin a linear fade-in from silence to full level over
   * kFadeDurationSeconds. Thread-safe.
   */
  void StartFadeIn() noexcept;

  /**
   * Begin a linear fade-out from full level to silence over
   * kFadeDurationSeconds. Thread-safe.
   */
  void StartFadeOut() noexcept;

  void prepareToPlay(int samples_per_block_expected,
                     double new_sample_rate) override;
  void releaseResources() override;
  void getNextAudioBlock(const juce::AudioSourceChannelInfo& info) override;

 private:
  /** Recompute LP IIR coefficients from last_cutoff_ (audio thread). */
  void UpdateLpFilters();

  /** Return a pointer to the currently active generator (audio thread). */
  [[nodiscard]] NoiseGenerator* ActiveGenerator() noexcept;

  // One generator per noise type – all prepared simultaneously so that
  // switching between types never requires a re-initialise.
  WhiteNoiseGenerator white_gen_;
  PinkNoiseGenerator pink_gen_;
  BrownNoiseGenerator brown_gen_;
  GreyNoiseGenerator grey_gen_;

  std::atomic<int> noise_type_{0};        // cast to NoiseType
  std::atomic<float> cutoff_{100.0f};     // normalised 0-100
  float last_cutoff_{100.0f};             // applied value (audio thread)
  std::atomic<float> gain_{1.0f};         // amplitude multiplier [0,1]
  std::atomic<float> fade_target_{0.0f};  // 0 = silent, 1 = full
  float fade_current_{0.0f};              // current fade (audio thread)
  float fade_step_{0.0f};                 // per-sample ramp (audio thread)
  double sample_rate_{44100.0};

  std::array<juce::IIRFilter, 2> lp_filters_;  // one per stereo channel

  LfoEngine lfo_;  // rate/intensity/mode atomics, phase audio-thread only
};

#endif  // ENAGA_AUDIO_NOISE_AUDIO_SOURCE_H_
