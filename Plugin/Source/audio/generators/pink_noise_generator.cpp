/**
 * @file   pink_noise_generator.cpp
 * @brief  PinkNoiseGenerator implementation.
 */

#include "audio/generators/pink_noise_generator.h"

void PinkNoiseGenerator::Prepare(double /*sample_rate*/) { Reset(); }

void PinkNoiseGenerator::Reset() noexcept {
  for (auto& ch : state_) ch.fill(0.0f);
}

float PinkNoiseGenerator::NextSample(std::size_t channel) noexcept {
  auto& b = state_[channel];
  const float w = random_.nextFloat() * 2.0f - 1.0f;

  b[0] = 0.99886f * b[0] + w * 0.0555179f;
  b[1] = 0.99332f * b[1] + w * 0.0750759f;
  b[2] = 0.96900f * b[2] + w * 0.1538520f;
  b[3] = 0.86650f * b[3] + w * 0.3104856f;
  b[4] = 0.55000f * b[4] + w * 0.5329522f;
  b[5] = -0.76160f * b[5] - w * 0.0168980f;
  const float pink =
      (b[0] + b[1] + b[2] + b[3] + b[4] + b[5] + b[6] + w * 0.5362f) * 0.11f;
  b[6] = w * 0.115926f;

  return pink;
}
