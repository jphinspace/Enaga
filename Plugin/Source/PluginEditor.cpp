/**
 * @file   PluginEditor.cpp
 * @brief  EnagaEditor implementation.
 */

#include "PluginEditor.h"

// ============================================================================
//  Constructor / Destructor
// ============================================================================

EnagaEditor::EnagaEditor(EnagaProcessor& proc)
    : AudioProcessorEditor(proc)
    , processor(proc)
    , content(MainComponent::AudioCallbacks {
          [this](bool shouldPlay)
          {
              if (shouldPlay) processor.startFadeIn();
              else            processor.startFadeOut();
          },
          [this](float v)    { processor.setCutoff(v);                          },
          [this](float g)    { processor.setGain(g);                            },
          [this](float v)
          {
              processor.setNoiseType(
                  static_cast<NoiseType>(
                      juce::jlimit(0, 3, static_cast<int>(v) - 1)));
          },
          [this](float r)    { processor.setLfoRate(r);                         },
          [this](float i)    { processor.setLfoIntensity(i);                    },
          [this](LfoMode m)  { processor.setLfoMode(m);                         }
      })
{
    juce::LookAndFeel::setDefaultLookAndFeel(&lookAndFeel);

    addAndMakeVisible(content);

    setResizable(true, false);
    setResizeLimits(320, 240, 10000, 10000);
    setSize(480, 600);
}

EnagaEditor::~EnagaEditor()
{
    juce::LookAndFeel::setDefaultLookAndFeel(nullptr);
}

// ============================================================================
//  juce::Component interface
// ============================================================================

void EnagaEditor::resized()
{
    content.setBounds(getLocalBounds());
}
