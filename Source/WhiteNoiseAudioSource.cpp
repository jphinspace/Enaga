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

    // Pre-compute the per-sample fade step (constant while sampleRate is fixed).
    static constexpr float fadeDuration = 0.25f; // seconds
    fadeStep = (sampleRate > 0.0)
             ? 1.0f / (fadeDuration * static_cast<float>(sampleRate))
             : 0.0f;

    lastCutoff  = cutoff.load(std::memory_order_relaxed);
    updateFilters();
    for (auto& f : filters)
        f.reset();
}

void WhiteNoiseAudioSource::releaseResources()
{
    for (auto& f : filters)
        f.reset();
}

void WhiteNoiseAudioSource::getNextAudioBlock(const juce::AudioSourceChannelInfo& info)
{
    // Re-coefficient the filters if the cutoff changed since the last block.
    const float c = cutoff.load(std::memory_order_relaxed);
    if (c != lastCutoff)
    {
        lastCutoff = c;
        updateFilters();
    }

    const float g      = gain.load(std::memory_order_relaxed);
    const float target = fadeTarget.load(std::memory_order_relaxed);

    for (int ch = 0; ch < info.buffer->getNumChannels(); ++ch)
    {
        auto* out = info.buffer->getWritePointer(ch, info.startSample);
        for (int i = 0; i < info.numSamples; ++i)
            out[i] = random.nextFloat() * 2.0f - 1.0f;

        if (ch < static_cast<int>(filters.size()))
            filters[static_cast<size_t>(ch)].processSamples(out, info.numSamples);

        // Apply user gain and per-sample fade ramp.
        // Each channel starts from the same fadeCurrent so all channels are
        // in sync; fadeCurrent is updated only once (after channel 0).
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

void WhiteNoiseAudioSource::updateFilters()
{
    if (sampleRate <= 0.0) return;

    // Logarithmic map: 0 → 20 Hz, 100 → 20 000 Hz.
    const double freq = 20.0 * std::pow(1000.0, static_cast<double>(lastCutoff) / 100.0);
    const auto   coef = juce::IIRCoefficients::makeLowPass(sampleRate, freq);
    for (auto& f : filters)
        f.setCoefficients(coef);
}
