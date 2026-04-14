/**
 * @file   plugin_processor.cpp
 * @brief  EnagaProcessor implementation.
 */

#include "plugin_processor.h"
#include "plugin_editor.h"

// ============================================================================
//  Constructor
// ============================================================================

EnagaProcessor::EnagaProcessor()
    : AudioProcessor(BusesProperties()
                         .withOutput("Output",
                                     juce::AudioChannelSet::stereo(),
                                     true)) {}

// ============================================================================
//  Parameter setters — thin delegation to NoiseAudioSource
// ============================================================================

void EnagaProcessor::SetCutoff(float normalised_0_to_100) noexcept {
  noise_source_.SetCutoff(normalised_0_to_100);
}

void EnagaProcessor::SetGain(float new_gain) noexcept {
  noise_source_.SetGain(new_gain);
}

void EnagaProcessor::SetNoiseType(NoiseType type) noexcept {
  noise_source_.SetNoiseType(type);
}

void EnagaProcessor::SetLfoRate(float rate_hz) noexcept {
  noise_source_.SetLfoRate(rate_hz);
}

void EnagaProcessor::SetLfoIntensity(float intensity) noexcept {
  noise_source_.SetLfoIntensity(intensity);
}

void EnagaProcessor::SetLfoMode(LfoMode mode) noexcept {
  noise_source_.SetLfoMode(mode);
}

void EnagaProcessor::StartFadeIn() noexcept {
  noise_source_.StartFadeIn();
}

void EnagaProcessor::StartFadeOut() noexcept {
  noise_source_.StartFadeOut();
}

// ============================================================================
//  juce::AudioProcessor interface
// ============================================================================

void EnagaProcessor::prepareToPlay(
    double sample_rate, int maximum_expected_samples_per_block) {
  noise_source_.prepareToPlay(maximum_expected_samples_per_block,
                              sample_rate);
}

void EnagaProcessor::releaseResources() {
  noise_source_.releaseResources();
}

void EnagaProcessor::processBlock(
    juce::AudioBuffer<float>& buffer,
    juce::MidiBuffer& /*midi_messages*/) {
  juce::AudioSourceChannelInfo info(
      &buffer, 0, buffer.getNumSamples());
  noise_source_.getNextAudioBlock(info);
}

juce::AudioProcessorEditor* EnagaProcessor::createEditor() {
  return new EnagaEditor(*this);
}

const juce::String EnagaProcessor::getName() const {
  return "Enaga";
}

void EnagaProcessor::getStateInformation(
    juce::MemoryBlock& /*dest_data*/) {
  // TODO: serialize plugin state (noise type, cutoff, gain, LFO settings)
}

void EnagaProcessor::setStateInformation(
    const void* /*data*/, int /*size_in_bytes*/) {
  // TODO: deserialize plugin state
}
