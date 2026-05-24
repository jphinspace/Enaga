/**
 * @file   editor.cpp
 * @brief  EnagaEditor implementation.
 */

#include "editor.h"

#include "audio/noise_type.h"

namespace {

MainComponent::AudioCallbacks BuildAudioCallbacks(EnagaEditorActions& actions) {
  return MainComponent::AudioCallbacks{
      [&actions](bool should_play) { actions.HandlePlayToggle(should_play); },
      [&actions](float value) { actions.HandleCutoff(value); },
      [&actions](float gain) { actions.HandleGain(gain); },
      [&actions](float value) { actions.HandleNoiseTypeSelection(value); },
      [&actions](float rate_hz) { actions.HandleLfoRate(rate_hz); },
      [&actions](float intensity) { actions.HandleLfoIntensity(intensity); },
      [&actions](LfoMode mode) { actions.HandleLfoMode(mode); }};
}

}  // namespace

// ============================================================================
//  Constructor / Destructor
// ============================================================================

EnagaEditor::EnagaEditor(EnagaProcessor& proc)
    : AudioProcessorEditor(proc),
      actions_(proc),
      content_(BuildAudioCallbacks(actions_)) {
  juce::LookAndFeel::setDefaultLookAndFeel(&look_and_feel_);

  addAndMakeVisible(content_);

  setResizable(true, false);
  setResizeLimits(320, 240, 10000, 10000);
  setSize(480, 600);
}

EnagaEditor::~EnagaEditor() {
  juce::LookAndFeel::setDefaultLookAndFeel(nullptr);
}

// ============================================================================
//  juce::Component interface
// ============================================================================

void EnagaEditor::resized() { content_.setBounds(getLocalBounds()); }
