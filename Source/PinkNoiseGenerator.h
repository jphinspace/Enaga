/**
 * @file   PinkNoiseGenerator.h
 * @brief  Pink noise generator – 1/f spectrum (−3 dB/octave).
 */

#pragma once

#include "NoiseGenerator.h"

#include <juce_audio_basics/juce_audio_basics.h>

#include <array>

/**
 * Produces pink (1/f) noise using Paul Kellett's parallel-filter algorithm.
 *
 * Seven first-order IIR low-pass filters with geometrically-spaced poles are
 * summed to give an approximate −3 dB/octave power spectrum.  Independent
 * state is maintained for each stereo channel so left and right are
 * decorrelated.
 */
class PinkNoiseGenerator final : public NoiseGenerator
{
public:
    void prepare(double /*sampleRate*/) override { reset(); }

    void reset() noexcept override
    {
        for (auto& ch : state)
            ch.fill(0.0f);
    }

    [[nodiscard]] float nextSample(std::size_t channel) noexcept override
    {
        auto& b       = state[channel];
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

private:
    juce::Random random;
    // 7 filter-state variables per stereo channel (Paul Kellett algorithm).
    std::array<std::array<float, 7>, 2> state {};
};
