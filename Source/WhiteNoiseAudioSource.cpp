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

    // Pre-compute the per-sample fade step (constant while sampleRate is fixed).
    fadeStep = (sampleRate > 0.0)
             ? 1.0f / (fadeDurationSeconds * static_cast<float>(sampleRate))
             : 0.0f;

    lastCutoff  = cutoff.load(std::memory_order_relaxed);
    updateFilters();
    for (auto& f : filters)
        f.reset();

    // Reset per-type state so that noise restarts cleanly on device re-open.
    for (auto& b : pinkState)
        b.fill(0.0f);
    brownState.fill(0.0f);

    updateGreyFilters();
    for (auto& channel : greyFilters)
        for (auto& f : channel)
            f.reset();
}

void WhiteNoiseAudioSource::releaseResources()
{
    for (auto& f : filters)
        f.reset();
    for (auto& channel : greyFilters)
        for (auto& f : channel)
            f.reset();
}

void WhiteNoiseAudioSource::getNextAudioBlock(const juce::AudioSourceChannelInfo& info)
{
    // Re-coefficient the LP filters if the cutoff changed since the last block.
    const float c = cutoff.load(std::memory_order_relaxed);
    if (c != lastCutoff)
    {
        lastCutoff = c;
        updateFilters();
    }

    const float     g      = gain.load(std::memory_order_relaxed);
    const float     target = fadeTarget.load(std::memory_order_relaxed);
    const NoiseType type   = static_cast<NoiseType>(noiseType.load(std::memory_order_relaxed));

    for (int ch = 0; ch < info.buffer->getNumChannels(); ++ch)
    {
        auto* out = info.buffer->getWritePointer(ch, info.startSample);
        const auto chIdx = static_cast<std::size_t>(juce::jmin(ch, 1)); // clamp to [0,1]

        // Generate raw noise samples for this channel.
        switch (type)
        {
            case NoiseType::Pink:
                for (int i = 0; i < info.numSamples; ++i)
                    out[i] = generatePinkSample(chIdx);
                break;

            case NoiseType::Brown:
                for (int i = 0; i < info.numSamples; ++i)
                    out[i] = generateBrownSample(chIdx);
                break;

            case NoiseType::Grey:
            case NoiseType::White:
            default:
                for (int i = 0; i < info.numSamples; ++i)
                    out[i] = random.nextFloat() * 2.0f - 1.0f;
                break;
        }

        // Apply the grey-noise inverse-A-weighting filter chain (white base only).
        if (type == NoiseType::Grey)
        {
            // chIdx is already clamped to [0, 1] above, so greyFilters[chIdx] is always valid.
            for (auto& f : greyFilters[chIdx])
                f.processSamples(out, info.numSamples);
            for (int i = 0; i < info.numSamples; ++i)
                out[i] *= kGreyCompensation;
        }

        // Apply LP filter (cutoff slider) to all noise types.
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

void WhiteNoiseAudioSource::updateGreyFilters()
{
    if (sampleRate <= 0.0) return;

    // Three-stage approximation of the inverse A-weighting curve.
    // Stage 0: low-shelf +20 dB at ~100 Hz (compensates A-weighting bass rolloff).
    const auto stage0 = juce::IIRCoefficients::makeLowShelf(
        sampleRate, 100.0, 0.5, 10.0);

    // Stage 1: peak cut ~−3 dB at ~3500 Hz (compensates A-weighting presence peak).
    const auto stage1 = juce::IIRCoefficients::makePeakFilter(
        sampleRate, 3500.0, 1.5, 0.71);

    // Stage 2: high-shelf +3.5 dB at ~10 kHz (compensates A-weighting HF rolloff).
    const auto stage2 = juce::IIRCoefficients::makeHighShelf(
        sampleRate, 10000.0, 0.5, 1.5);

    for (auto& channel : greyFilters)
    {
        channel[0].setCoefficients(stage0);
        channel[1].setCoefficients(stage1);
        channel[2].setCoefficients(stage2);
    }
}

float WhiteNoiseAudioSource::generatePinkSample(std::size_t ch) noexcept
{
    // Paul Kellett's parallel-filter pink noise algorithm.
    // Sums 7 first-order IIR low-pass filters at geometrically spaced poles
    // to give an approximate 1/f (−3 dB/octave) power spectrum.
    auto& b = pinkState[ch];
    const float w = random.nextFloat() * 2.0f - 1.0f;
    b[0] =  0.99886f * b[0] + w * 0.0555179f;
    b[1] =  0.99332f * b[1] + w * 0.0750759f;
    b[2] =  0.96900f * b[2] + w * 0.1538520f;
    b[3] =  0.86650f * b[3] + w * 0.3104856f;
    b[4] =  0.55000f * b[4] + w * 0.5329522f;
    b[5] = -0.76160f * b[5] - w * 0.0168980f;
    const float pink = (b[0] + b[1] + b[2] + b[3] + b[4] + b[5] + b[6] + w * 0.5362f) * 0.11f;
    b[6] = w * 0.115926f;
    return pink;
}

float WhiteNoiseAudioSource::generateBrownSample(std::size_t ch) noexcept
{
    // First-order IIR integration of white noise gives a 1/f² (−6 dB/octave) spectrum.
    // The leaky integrator prevents unbounded drift; the *3.5 factor normalises amplitude.
    const float w = random.nextFloat() * 2.0f - 1.0f;
    brownState[ch] = (brownState[ch] + 0.02f * w) / 1.02f;
    return juce::jlimit(-1.0f, 1.0f, brownState[ch] * 3.5f);
}
