/**
 * @file   processor.cpp
 * @brief  EnagaProcessor implementation.
 */

#include "processor.h"

#include <memory>

#include "editor.h"

// ============================================================================
//  Constructor
// ============================================================================

EnagaProcessor::EnagaProcessor()
    : AudioProcessor(BusesProperties().withOutput(
          "Output", juce::AudioChannelSet::stereo(), true)) {}

// ============================================================================
//  Parameter setters — thin delegation to NoiseAudioSource
// ============================================================================

void EnagaProcessor::SetCutoff(float normalised_0_to_100) noexcept {
  core_.SetCutoff(normalised_0_to_100);
}

void EnagaProcessor::SetGain(float new_gain) noexcept {
  core_.SetGain(new_gain);
}

void EnagaProcessor::SetNoiseType(NoiseType type) noexcept {
  core_.SetNoiseType(type);
}

void EnagaProcessor::SetLfoRate(float rate_hz) noexcept {
  core_.SetLfoRate(rate_hz);
}

void EnagaProcessor::SetLfoIntensity(float intensity) noexcept {
  core_.SetLfoIntensity(intensity);
}

void EnagaProcessor::SetLfoMode(LfoMode mode) noexcept {
  core_.SetLfoMode(mode);
}

void EnagaProcessor::StartFadeIn() noexcept { core_.StartFadeIn(); }

void EnagaProcessor::StartFadeOut() noexcept { core_.StartFadeOut(); }

// ============================================================================
//  juce::AudioProcessor interface
// ============================================================================

void EnagaProcessor::prepareToPlay(double sample_rate,
                                   int maximum_expected_samples_per_block) {
  core_.PrepareToPlay(sample_rate, maximum_expected_samples_per_block);
}

void EnagaProcessor::releaseResources() { core_.ReleaseResources(); }

void EnagaProcessor::processBlock(juce::AudioBuffer<float>& buffer,
                                  juce::MidiBuffer& /*midi_messages*/) {
  core_.ProcessBlock(buffer);
}

juce::AudioProcessorEditor* EnagaProcessor::createEditor() {
  return std::make_unique<EnagaEditor>(*this).release();
}

const juce::String EnagaProcessor::getName() const { return "Enaga"; }

void EnagaProcessor::getStateInformation(juce::MemoryBlock& /*dest_data*/) {
  // TODO: serialize plugin state (noise type, cutoff, gain, LFO settings)
}

void EnagaProcessor::setStateInformation(const void* /*data*/,
                                         int /*size_in_bytes*/) {
  // TODO: deserialize plugin state
}
