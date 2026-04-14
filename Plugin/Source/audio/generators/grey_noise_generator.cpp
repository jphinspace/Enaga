/**
 * @file   grey_noise_generator.cpp
 * @brief  GreyNoiseGenerator implementation.
 */

#include "audio/generators/grey_noise_generator.h"

void GreyNoiseGenerator::Prepare(double sample_rate) {
  const auto s0 = juce::IIRCoefficients::makeLowShelf(
      sample_rate, 100.0, 0.5, 10.0);
  const auto s1 = juce::IIRCoefficients::makePeakFilter(
      sample_rate, 3500.0, 1.5, 0.71);
  const auto s2 = juce::IIRCoefficients::makeHighShelf(
      sample_rate, 10000.0, 0.5, 1.5);

  for (auto& ch : filters_) {
    ch[0].setCoefficients(s0);
    ch[1].setCoefficients(s1);
    ch[2].setCoefficients(s2);
  }

  Reset();
}

void GreyNoiseGenerator::Reset() noexcept {
  for (auto& ch : filters_)
    for (auto& f : ch)
      f.reset();
}

float GreyNoiseGenerator::NextSample(std::size_t channel) noexcept {
  float s = random_.nextFloat() * 2.0f - 1.0f;

  for (auto& f : filters_[channel])
    s = f.processSingleSampleRaw(s);

  return s * kAmplitude;
}
