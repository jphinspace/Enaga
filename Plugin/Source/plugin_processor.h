/**
 * @file   plugin_processor.h
 * @brief  AudioProcessor for the Enaga plugin.
 *
 * Wraps NoiseAudioSource inside JUCE's AudioProcessor interface so that
 * Enaga can be loaded by any VST3/AU-compatible DAW as well as by the
 * standalone host application.
 */

#pragma once

#include "audio/noise_audio_source.h"
#include "audio/lfo_mode.h"
#include "audio/noise_type.h"

#include <juce_audio_processors/juce_audio_processors.h>

/**
 * Audio processor for the Enaga noise generator.
 *
 * All parameter mutations are delegated to the underlying NoiseAudioSource
 * via the same lock-free atomics that the original standalone app used.
 * The processBlock() method converts JUCE's buffer/MIDI block pair into the
 * AudioSourceChannelInfo expected by NoiseAudioSource.
 */
class EnagaProcessor final : public juce::AudioProcessor {
 public:
  EnagaProcessor();
  ~EnagaProcessor() override = default;

  // -------------------------------------------------------------------
  //  Audio parameter setters (called from the message thread via callbacks)
  // -------------------------------------------------------------------

  void SetCutoff(float normalised_0_to_100) noexcept;
  void SetGain(float new_gain) noexcept;
  void SetNoiseType(NoiseType type) noexcept;
  void SetLfoRate(float rate_hz) noexcept;
  void SetLfoIntensity(float intensity) noexcept;
  void SetLfoMode(LfoMode mode) noexcept;
  void StartFadeIn() noexcept;
  void StartFadeOut() noexcept;

  // -------------------------------------------------------------------
  //  juce::AudioProcessor interface
  // -------------------------------------------------------------------

  void prepareToPlay(double sample_rate,
                     int maximum_expected_samples_per_block) override;
  void releaseResources() override;
  void processBlock(juce::AudioBuffer<float>& buffer,
                    juce::MidiBuffer& midi_messages) override;

  [[nodiscard]] juce::AudioProcessorEditor* createEditor() override;
  [[nodiscard]] bool hasEditor() const override { return true; }

  [[nodiscard]] const juce::String getName() const override;

  [[nodiscard]] bool   acceptsMidi()  const override { return false;  }
  [[nodiscard]] bool   producesMidi() const override { return false;  }
  [[nodiscard]] bool   isMidiEffect() const override { return false;  }
  [[nodiscard]] double getTailLengthSeconds() const override { return 0.0; }

  [[nodiscard]] int  getNumPrograms()    override { return 1;  }
  [[nodiscard]] int  getCurrentProgram() override { return 0;  }
  void setCurrentProgram(int) override {}
  [[nodiscard]] const juce::String getProgramName(int) override {
    return {};
  }
  void changeProgramName(int, const juce::String&) override {}

  void getStateInformation(juce::MemoryBlock& dest_data) override;
  void setStateInformation(const void* data,
                           int size_in_bytes) override;

 private:
  NoiseAudioSource noise_source_;

  JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(EnagaProcessor)
};
