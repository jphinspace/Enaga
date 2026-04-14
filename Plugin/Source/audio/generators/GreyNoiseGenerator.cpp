/**
 * @file   GreyNoiseGenerator.cpp
 * @brief  GreyNoiseGenerator implementation.
 */

#include "audio/generators/GreyNoiseGenerator.h"

void GreyNoiseGenerator::prepare(double sampleRate)
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

void GreyNoiseGenerator::reset() noexcept
{
    for (auto& ch : filters)
        for (auto& f : ch)
            f.reset();
}

float GreyNoiseGenerator::nextSample(std::size_t channel) noexcept
{
    float s = random.nextFloat() * 2.0f - 1.0f;

    for (auto& f : filters[channel])
        s = f.processSingleSampleRaw(s);

    return s * kAmplitude;
}
