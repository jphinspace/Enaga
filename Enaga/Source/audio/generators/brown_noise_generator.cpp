/**
 * @file   brown_noise_generator.cpp
 * @brief  BrownNoiseGenerator implementation.
 */

#include "audio/generators/brown_noise_generator.h"

void BrownNoiseGenerator::Prepare(double /*sample_rate*/) { Reset(); }

void BrownNoiseGenerator::Reset() noexcept { state_.fill(0.0f); }

float BrownNoiseGenerator::NextSample(std::size_t channel) noexcept {
  const float w = random_.nextFloat() * 2.0f - 1.0f;
  state_[channel] = (state_[channel] + 0.02f * w) / 1.02f;
  return juce::jlimit(-1.0f, 1.0f, state_[channel] * 3.5f);
}
