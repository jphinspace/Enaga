/**
 * @file   WhiteNoiseGenerator.cpp
 * @brief  WhiteNoiseGenerator implementation.
 */

#include "audio/generators/WhiteNoiseGenerator.h"

void WhiteNoiseGenerator::prepare(double /*sampleRate*/) {}

void WhiteNoiseGenerator::reset() noexcept {}

float WhiteNoiseGenerator::nextSample(std::size_t /*channel*/) noexcept
{
    return random.nextFloat() * 2.0f - 1.0f;
}
