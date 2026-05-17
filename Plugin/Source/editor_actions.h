/**
 * @file   editor_actions.h
 * @brief  Testable callback handlers used by EnagaEditor.
 */

#ifndef ENAGA_PLUGIN_EDITOR_ACTIONS_H_
#define ENAGA_PLUGIN_EDITOR_ACTIONS_H_

#include "processor_controls.h"

/**
 * Editor-side callback handlers independent from JUCE component construction.
 *
 * This keeps UI callback behavior unit-testable (mapping/clamping/conversion)
 * while EnagaEditor remains a thin JUCE wrapper.
 */
class EnagaEditorActions final {
 public:
  explicit EnagaEditorActions(EnagaProcessorControls& controls) noexcept
      : controls_(controls) {}

  void HandlePlayToggle(bool should_play) noexcept;
  void HandleCutoff(float cutoff) noexcept;
  void HandleGain(float gain) noexcept;
  void HandleNoiseTypeSelection(float selected_index) noexcept;
  void HandleLfoRate(float rate_hz) noexcept;
  void HandleLfoIntensity(float intensity) noexcept;
  void HandleLfoMode(LfoMode mode) noexcept;

 private:
  EnagaProcessorControls& controls_;
};

#endif  // ENAGA_PLUGIN_EDITOR_ACTIONS_H_
