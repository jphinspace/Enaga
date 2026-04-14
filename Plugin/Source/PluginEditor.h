/**
 * @file   PluginEditor.h
 * @brief  AudioProcessorEditor for the Enaga plugin.
 *
 * Wraps MainComponent inside JUCE's AudioProcessorEditor interface and wires
 * all UI callbacks back to EnagaProcessor.
 */

#pragma once

#include "PluginProcessor.h"
#include "ui/EnagaLookAndFeel.h"
#include "ui/MainComponent.h"

#include <juce_audio_processors/juce_audio_processors.h>

/**
 * Plugin editor that hosts MainComponent.
 *
 * The editor owns the LookAndFeel for the duration of its lifetime and
 * constructs a MainComponent whose AudioCallbacks are wired directly to
 * the corresponding parameter setters on the owning EnagaProcessor.
 */
class EnagaEditor final : public juce::AudioProcessorEditor
{
public:
    explicit EnagaEditor(EnagaProcessor& processor);
    ~EnagaEditor() override;

    // -----------------------------------------------------------------------
    //  juce::Component interface
    // -----------------------------------------------------------------------

    void resized() override;

private:
    EnagaProcessor&   processor;
    EnagaLookAndFeel  lookAndFeel;   // must outlive content
    MainComponent     content;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(EnagaEditor)
};
