/**
 * @file   WhiteNoiseGenerator.h
 * @brief  White noise generator – flat frequency spectrum.
 */

#pragma once

#include "NoiseGenerator.h"

#include <juce_audio_basics/juce_audio_basics.h>

/**
 * Produces uniformly-distributed white noise with a flat power spectrum.
 * No per-channel DSP state is needed; the PRNG feeds all channels equally.
 */
class WhiteNoiseGenerator final : public NoiseGenerator
{
public:
    void prepare(double /*sampleRate*/) override {}
    void reset() noexcept override {}

    [[nodiscard]] float nextSample(std::size_t /*channel*/) noexcept override
    {
        return random.nextFloat() * 2.0f - 1.0f;
    }

private:
    juce::Random random;
};
