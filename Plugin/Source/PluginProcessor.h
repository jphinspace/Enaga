/**
 * @file   PluginProcessor.h
 * @brief  AudioProcessor for the Enaga plugin.
 *
 * Wraps NoiseAudioSource inside JUCE's AudioProcessor interface so that
 * Enaga can be loaded by any VST3/AU-compatible DAW as well as by the
 * standalone host application.
 */

#pragma once

#include "audio/NoiseAudioSource.h"
#include "audio/LfoMode.h"
#include "audio/NoiseType.h"

#include <juce_audio_processors/juce_audio_processors.h>

/**
 * Audio processor for the Enaga noise generator.
 *
 * All parameter mutations are delegated to the underlying NoiseAudioSource
 * via the same lock-free atomics that the original standalone app used.
 * The processBlock() method converts JUCE's buffer/MIDI block pair into the
 * AudioSourceChannelInfo expected by NoiseAudioSource.
 */
class EnagaProcessor final : public juce::AudioProcessor
{
public:
    EnagaProcessor();
    ~EnagaProcessor() override = default;

    // -----------------------------------------------------------------------
    //  Audio parameter setters  (called from the message thread via callbacks)
    // -----------------------------------------------------------------------

    void setCutoff(float normalised0to100) noexcept;
    void setGain(float newGain) noexcept;
    void setNoiseType(NoiseType type) noexcept;
    void setLfoRate(float rateHz) noexcept;
    void setLfoIntensity(float intensity) noexcept;
    void setLfoMode(LfoMode mode) noexcept;
    void startFadeIn() noexcept;
    void startFadeOut() noexcept;

    // -----------------------------------------------------------------------
    //  juce::AudioProcessor interface
    // -----------------------------------------------------------------------

    void prepareToPlay(double sampleRate,
                       int    maximumExpectedSamplesPerBlock) override;
    void releaseResources() override;
    void processBlock(juce::AudioBuffer<float>& buffer,
                      juce::MidiBuffer&         midiMessages) override;

    [[nodiscard]] juce::AudioProcessorEditor* createEditor() override;
    [[nodiscard]] bool hasEditor() const override { return true; }

    [[nodiscard]] const juce::String getName() const override;

    [[nodiscard]] bool   acceptsMidi()  const override { return false;  }
    [[nodiscard]] bool   producesMidi() const override { return false;  }
    [[nodiscard]] bool   isMidiEffect() const override { return false;  }
    [[nodiscard]] double getTailLengthSeconds() const override { return 0.0; }

    [[nodiscard]] int  getNumPrograms()  override { return 1; }
    [[nodiscard]] int  getCurrentProgram() override { return 0; }
    void setCurrentProgram(int) override {}
    [[nodiscard]] const juce::String getProgramName(int) override { return {}; }
    void changeProgramName(int, const juce::String&) override {}

    void getStateInformation(juce::MemoryBlock& destData) override;
    void setStateInformation(const void* data, int sizeInBytes) override;

private:
    NoiseAudioSource noiseSource;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(EnagaProcessor)
};
