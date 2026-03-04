/**
 * @file   GreyNoiseGenerator.h
 * @brief  Grey noise generator – perceptually flat (inverse A-weighting).
 */

#pragma once

#include "NoiseGenerator.h"

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
    void prepare(double sampleRate) override
    {
        const auto s0 = juce::IIRCoefficients::makeLowShelf (sampleRate, 100.0,   0.5, 10.0);
        const auto s1 = juce::IIRCoefficients::makePeakFilter(sampleRate, 3500.0, 1.5, 0.71);
        const auto s2 = juce::IIRCoefficients::makeHighShelf (sampleRate, 10000.0, 0.5, 1.5);

        for (auto& ch : filters)
        {
            ch[0].setCoefficients(s0);
            ch[1].setCoefficients(s1);
            ch[2].setCoefficients(s2);
        }

        reset();
    }

    void reset() noexcept override
    {
        for (auto& ch : filters)
            for (auto& f : ch)
                f.reset();
    }

    [[nodiscard]] float nextSample(std::size_t channel) noexcept override
    {
        float s = random.nextFloat() * 2.0f - 1.0f;

        for (auto& f : filters[channel])
            s = f.processSingleSampleRaw(s);

        return s * kAmplitude;
    }

private:
    static constexpr int   kStages    = 3;
    static constexpr float kAmplitude = 0.05f;

    juce::Random random;
    std::array<std::array<juce::IIRFilter, kStages>, 2> filters {};
};
