/**
 * @file   BrownNoiseGenerator.h
 * @brief  Brown (Brownian / red) noise generator – 1/f² spectrum (−6 dB/octave).
 */

#pragma once

#include "audio/generators/NoiseGenerator.h"

#include <juce_audio_basics/juce_audio_basics.h>

#include <array>

/**
 * Produces brown noise by leaky first-order IIR integration of white noise.
 *
 * The leaky integrator prevents DC drift while preserving the −6 dB/octave
 * power spectrum.  Output is normalised with ×3.5 and hard-clipped to [−1, 1].
 * Independent state is maintained for each stereo channel.
 */
class BrownNoiseGenerator final : public NoiseGenerator
{
public:
    void prepare(double sampleRate) override;
    void reset() noexcept override;
    [[nodiscard]] float nextSample(std::size_t channel) noexcept override;

private:
    juce::Random random;
    std::array<float, 2> state {};  // running integration value per channel
};
