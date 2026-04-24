/**
 * @file   plugin_editor.h
 * @brief  AudioProcessorEditor for the Enaga plugin.
 *
 * Wraps MainComponent inside JUCE's AudioProcessorEditor interface and
 * wires all UI callbacks back to EnagaProcessor.
 */

#ifndef ENAGA_PLUGIN_EDITOR_H_
#define ENAGA_PLUGIN_EDITOR_H_

#include <juce_audio_processors/juce_audio_processors.h>

#include "plugin_processor.h"
#include "ui/enaga_look_and_feel.h"
#include "ui/main_component.h"

/**
 * Plugin editor that hosts MainComponent.
 *
 * The editor owns the LookAndFeel for the duration of its lifetime and
 * constructs a MainComponent whose AudioCallbacks are wired directly to
 * the corresponding parameter setters on the owning EnagaProcessor.
 */
class EnagaEditor final : public juce::AudioProcessorEditor {
 public:
  explicit EnagaEditor(EnagaProcessor& processor);
  ~EnagaEditor() override;

  // -------------------------------------------------------------------
  //  juce::Component interface
  // -------------------------------------------------------------------

  void resized() override;

 private:
  EnagaProcessor& processor_;
  EnagaLookAndFeel look_and_feel_;  // must outlive content_
  MainComponent content_;

  JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(EnagaEditor)
};

#endif  // ENAGA_PLUGIN_EDITOR_H_
