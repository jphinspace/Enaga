/**
 * @file   WhiteNoiseAudioSource.cpp
 * @brief  WhiteNoiseAudioSource implementation.
 */

#include "WhiteNoiseAudioSource.h"

#include <cmath>

void WhiteNoiseAudioSource::setCutoff(float normalised0to100) noexcept
{
    cutoff.store(juce::jlimit(0.0f, 100.0f, normalised0to100),
                 std::memory_order_relaxed);
}

void WhiteNoiseAudioSource::setGain(float newGain) noexcept
{
    gain.store(juce::jlimit(0.0f, 1.0f, newGain), std::memory_order_relaxed);
}

void WhiteNoiseAudioSource::setNoiseType(NoiseType type) noexcept
{
    noiseType.store(static_cast<int>(type), std::memory_order_relaxed);
}

void WhiteNoiseAudioSource::startFadeIn() noexcept
{
    fadeTarget.store(1.0f, std::memory_order_relaxed);
}

void WhiteNoiseAudioSource::startFadeOut() noexcept
{
    fadeTarget.store(0.0f, std::memory_order_relaxed);
}

void WhiteNoiseAudioSource::prepareToPlay(int /*samplesPerBlockExpected*/,
                                          double newSampleRate)
{
    sampleRate  = newSampleRate;
    fadeCurrent = fadeTarget.load(std::memory_order_relaxed);
    fadeStep    = (sampleRate > 0.0)
               ? 1.0f / (fadeDurationSeconds * static_cast<float>(sampleRate))
               : 0.0f;

    lastCutoff = cutoff.load(std::memory_order_relaxed);
    updateLpFilters();
    for (auto& f : lpFilters)
        f.reset();

    whiteGen.prepare(sampleRate);
    pinkGen.prepare(sampleRate);
    brownGen.prepare(sampleRate);
    greyGen.prepare(sampleRate);
}

void WhiteNoiseAudioSource::releaseResources()
{
    for (auto& f : lpFilters)
        f.reset();

    whiteGen.reset();
    pinkGen.reset();
    brownGen.reset();
    greyGen.reset();
}

NoiseGenerator* WhiteNoiseAudioSource::activeGenerator() noexcept
{
    switch (static_cast<NoiseType>(noiseType.load(std::memory_order_relaxed)))
    {
        case NoiseType::Pink:  return &pinkGen;
        case NoiseType::Brown: return &brownGen;
        case NoiseType::Grey:  return &greyGen;
        default:               return &whiteGen;
    }
}

void WhiteNoiseAudioSource::getNextAudioBlock(const juce::AudioSourceChannelInfo& info)
{
    // Re-coefficient the LP filter if the cutoff changed since the last block.
    const float c = cutoff.load(std::memory_order_relaxed);
    if (c != lastCutoff)
    {
        lastCutoff = c;
        updateLpFilters();
    }

    const float g      = gain.load(std::memory_order_relaxed);
    const float target = fadeTarget.load(std::memory_order_relaxed);

    // Read the noise type once so the whole block uses the same generator.
    NoiseGenerator* gen = activeGenerator();

    for (int ch = 0; ch < info.buffer->getNumChannels(); ++ch)
    {
        auto* out = info.buffer->getWritePointer(ch, info.startSample);
        const auto chIdx = static_cast<std::size_t>(juce::jmin(ch, 1));

        for (int i = 0; i < info.numSamples; ++i)
            out[i] = gen->nextSample(chIdx);

        lpFilters[chIdx].processSamples(out, info.numSamples);

        // Apply user gain and per-sample fade ramp.
        // All channels start from the same fadeCurrent so they stay in sync;
        // fadeCurrent is updated only once (after channel 0).
        float fc = fadeCurrent;
        for (int i = 0; i < info.numSamples; ++i)
        {
            out[i] *= g * fc;
            if (fc != target)
                fc = (target > fc) ? juce::jmin(fc + fadeStep, target)
                                   : juce::jmax(fc - fadeStep, target);
        }

        if (ch == 0)
            fadeCurrent = fc;
    }
}

void WhiteNoiseAudioSource::updateLpFilters()
{
    if (sampleRate <= 0.0) return;

    // Logarithmic map: 0 → 20 Hz, 100 → 20 000 Hz.
    const double freq = 20.0 * std::pow(1000.0, static_cast<double>(lastCutoff) / 100.0);
    const auto   coef = juce::IIRCoefficients::makeLowPass(sampleRate, freq);
    for (auto& f : lpFilters)
        f.setCoefficients(coef);
}
