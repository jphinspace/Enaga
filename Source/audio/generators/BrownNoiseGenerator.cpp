/**
 * @file   BrownNoiseGenerator.cpp
 * @brief  BrownNoiseGenerator implementation.
 */

#include "audio/generators/BrownNoiseGenerator.h"

void BrownNoiseGenerator::prepare(double /*sampleRate*/) { reset(); }

void BrownNoiseGenerator::reset() noexcept { state.fill(0.0f); }

float BrownNoiseGenerator::nextSample(std::size_t channel) noexcept
{
    const float w  = random.nextFloat() * 2.0f - 1.0f;
    state[channel] = (state[channel] + 0.02f * w) / 1.02f;
    return juce::jlimit(-1.0f, 1.0f, state[channel] * 3.5f);
}
