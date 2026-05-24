/**
 * @file   processor_controls.h
 * @brief  Abstract processor-control interface used by editor-side callbacks.
 */

#ifndef ENAGA_PLUGIN_PROCESSOR_CONTROLS_H_
#define ENAGA_PLUGIN_PROCESSOR_CONTROLS_H_

#include "audio/lfo_mode.h"
#include "audio/noise_type.h"

/**
 * Interface for mutating Enaga audio parameters.
 *
 * Extracted from EnagaProcessor to allow editor callback logic to be tested
 * with fakes, without constructing JUCE editor/processor objects.
 */
class EnagaProcessorControls {
 public:
  virtual ~EnagaProcessorControls() = default;

  virtual void SetCutoff(float normalised_0_to_100) noexcept = 0;
  virtual void SetGain(float new_gain) noexcept = 0;
  virtual void SetNoiseType(NoiseType type) noexcept = 0;
  virtual void SetLfoRate(float rate_hz) noexcept = 0;
  virtual void SetLfoIntensity(float intensity) noexcept = 0;
  virtual void SetLfoMode(LfoMode mode) noexcept = 0;
  virtual void StartFadeIn() noexcept = 0;
  virtual void StartFadeOut() noexcept = 0;
};

#endif  // ENAGA_PLUGIN_PROCESSOR_CONTROLS_H_
