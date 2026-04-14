/**
 * @file   PluginProcessor.cpp
 * @brief  EnagaProcessor implementation.
 */

#include "PluginProcessor.h"
#include "PluginEditor.h"

// ============================================================================
//  Constructor
// ============================================================================

EnagaProcessor::EnagaProcessor()
    : AudioProcessor(BusesProperties()
                         .withOutput("Output",
                                     juce::AudioChannelSet::stereo(),
                                     true))
{
}

// ============================================================================
//  Parameter setters — thin delegation to NoiseAudioSource
// ============================================================================

void EnagaProcessor::setCutoff(float normalised0to100) noexcept
{
    noiseSource.setCutoff(normalised0to100);
}

void EnagaProcessor::setGain(float newGain) noexcept
{
    noiseSource.setGain(newGain);
}

void EnagaProcessor::setNoiseType(NoiseType type) noexcept
{
    noiseSource.setNoiseType(type);
}

void EnagaProcessor::setLfoRate(float rateHz) noexcept
{
    noiseSource.setLfoRate(rateHz);
}

void EnagaProcessor::setLfoIntensity(float intensity) noexcept
{
    noiseSource.setLfoIntensity(intensity);
}

void EnagaProcessor::setLfoMode(LfoMode mode) noexcept
{
    noiseSource.setLfoMode(mode);
}

void EnagaProcessor::startFadeIn() noexcept
{
    noiseSource.startFadeIn();
}

void EnagaProcessor::startFadeOut() noexcept
{
    noiseSource.startFadeOut();
}

// ============================================================================
//  juce::AudioProcessor interface
// ============================================================================

void EnagaProcessor::prepareToPlay(double sampleRate,
                                   int    maximumExpectedSamplesPerBlock)
{
    noiseSource.prepareToPlay(maximumExpectedSamplesPerBlock, sampleRate);
}

void EnagaProcessor::releaseResources()
{
    noiseSource.releaseResources();
}

void EnagaProcessor::processBlock(juce::AudioBuffer<float>& buffer,
                                  juce::MidiBuffer& /*midiMessages*/)
{
    juce::AudioSourceChannelInfo info(&buffer, 0, buffer.getNumSamples());
    noiseSource.getNextAudioBlock(info);
}

juce::AudioProcessorEditor* EnagaProcessor::createEditor()
{
    return new EnagaEditor(*this);
}

const juce::String EnagaProcessor::getName() const
{
    return "Enaga";
}

void EnagaProcessor::getStateInformation(juce::MemoryBlock& /*destData*/)
{
    // TODO: serialize plugin state (noise type, cutoff, gain, LFO settings)
}

void EnagaProcessor::setStateInformation(const void* /*data*/,
                                         int         /*sizeInBytes*/)
{
    // TODO: deserialize plugin state
}
