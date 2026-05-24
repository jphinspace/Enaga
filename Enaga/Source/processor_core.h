/**
 * @file   processor_core.h
 * @brief  Testable audio-processing core used by EnagaProcessor.
 */

#ifndef ENAGA_PLUGIN_PROCESSOR_CORE_H_
#define ENAGA_PLUGIN_PROCESSOR_CORE_H_

#include <juce_audio_basics/juce_audio_basics.h>

#include <memory>

#include "audio/lfo_mode.h"
#include "audio/noise_type.h"

/** Abstract noise-source interface for processor-core dependency injection. */
class EnagaNoiseSource {
 public:
  virtual ~EnagaNoiseSource() = default;

  virtual void SetCutoff(float normalised_0_to_100) noexcept = 0;
  virtual void SetGain(float new_gain) noexcept = 0;
  virtual void SetNoiseType(NoiseType type) noexcept = 0;
  virtual void SetLfoRate(float rate_hz) noexcept = 0;
  virtual void SetLfoIntensity(float intensity) noexcept = 0;
  virtual void SetLfoMode(LfoMode mode) noexcept = 0;
  virtual void StartFadeIn() noexcept = 0;
  virtual void StartFadeOut() noexcept = 0;
  virtual void PrepareToPlay(int samples_per_block_expected,
                             double new_sample_rate) = 0;
  virtual void ReleaseResources() = 0;
  virtual void GetNextAudioBlock(const juce::AudioSourceChannelInfo& info) = 0;
};

/** Processor logic extracted from EnagaProcessor for unit-level testing. */
class EnagaProcessorCore final {
 public:
  EnagaProcessorCore();
  explicit EnagaProcessorCore(std::unique_ptr<EnagaNoiseSource> noise_source);

  void SetCutoff(float normalised_0_to_100) noexcept;
  void SetGain(float new_gain) noexcept;
  void SetNoiseType(NoiseType type) noexcept;
  void SetLfoRate(float rate_hz) noexcept;
  void SetLfoIntensity(float intensity) noexcept;
  void SetLfoMode(LfoMode mode) noexcept;
  void StartFadeIn() noexcept;
  void StartFadeOut() noexcept;

  void PrepareToPlay(double sample_rate,
                     int maximum_expected_samples_per_block);
  void ReleaseResources();
  void ProcessBlock(juce::AudioBuffer<float>& buffer);

 private:
  std::unique_ptr<EnagaNoiseSource> noise_source_;
};

#endif  // ENAGA_PLUGIN_PROCESSOR_CORE_H_
