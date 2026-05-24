/**
 * @file   white_noise_generator.cpp
 * @brief  WhiteNoiseGenerator implementation.
 */

#include "audio/generators/white_noise_generator.h"

void WhiteNoiseGenerator::Prepare(double /*sample_rate*/) {}

void WhiteNoiseGenerator::Reset() noexcept {}

float WhiteNoiseGenerator::NextSample(std::size_t /*channel*/) noexcept {
  return random_.nextFloat() * 2.0f - 1.0f;
}
