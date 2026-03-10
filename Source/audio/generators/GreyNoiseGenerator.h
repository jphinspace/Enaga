/**
 * @file   GreyNoiseGenerator.h
 * @brief  Grey noise generator – perceptually flat (inverse A-weighting).
 */

#pragma once

#include "audio/generators/NoiseGenerator.h"

#include <juce_audio_basics/juce_audio_basics.h>

#include <array>

/**
 * Produces grey noise: white noise shaped by the inverse A-weighting curve
 * so that it sounds perceptually flat to human hearing.
 *
 * Approximated with a three-stage IIR filter chain per stereo channel:
 *   Stage 0 – low-shelf  +20 dB  @ ~100 Hz  (compensates bass rolloff)
 *   Stage 1 – peak cut   −3 dB   @ ~3.5 kHz (compensates presence peak)
 *   Stage 2 – high-shelf +3.5 dB @ ~10 kHz  (compensates HF rolloff)
 *
 * A ×0.05 post-scale normalises the boosted output back to [−1, 1].
 * Independent filter state is maintained for each stereo channel.
 */
class GreyNoiseGenerator final : public NoiseGenerator
{
public:
    void prepare(double sampleRate) override;
    void reset() noexcept override;
    [[nodiscard]] float nextSample(std::size_t channel) noexcept override;

private:
    static constexpr int   kStages    = 3;
    static constexpr float kAmplitude = 0.05f;

    juce::Random random;
    std::array<std::array<juce::IIRFilter, kStages>, 2> filters {};
};
