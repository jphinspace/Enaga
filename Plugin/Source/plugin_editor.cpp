/**
 * @file   plugin_editor.cpp
 * @brief  EnagaEditor implementation.
 */

#include "plugin_editor.h"

// ============================================================================
//  Constructor / Destructor
// ============================================================================

EnagaEditor::EnagaEditor(EnagaProcessor& proc)
    : AudioProcessorEditor(proc),
      processor_(proc),
      content_(MainComponent::AudioCallbacks{
          [this](bool should_play) {
            if (should_play)
              processor_.StartFadeIn();
            else
              processor_.StartFadeOut();
          },
          [this](float v) { processor_.SetCutoff(v); },
          [this](float g) { processor_.SetGain(g); },
          [this](float v) {
            processor_.SetNoiseType(static_cast<NoiseType>(
                juce::jlimit(0, 3, static_cast<int>(v) - 1)));
          },
          [this](float r) { processor_.SetLfoRate(r); },
          [this](float i) { processor_.SetLfoIntensity(i); },
          [this](LfoMode m) { processor_.SetLfoMode(m); }}) {
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
