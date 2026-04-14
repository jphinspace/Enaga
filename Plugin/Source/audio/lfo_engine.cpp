/**
 * @file   lfo_engine.cpp
 * @brief  LfoEngine implementation.
 */

#include "audio/lfo_engine.h"

#include <juce_audio_basics/juce_audio_basics.h>

#include <cmath>

float LfoEngine::Tick(int num_samples, double sample_rate) noexcept {
  if (sample_rate <= 0.0) return 1.0f;

  // Sample at the start of the block, then advance.
  const float result =
      static_cast<float>((1.0 + std::sin(phase_)) * 0.5);

  phase_ +=
      static_cast<double>(rate_.load(std::memory_order_relaxed))
      * juce::MathConstants<double>::twoPi
      * static_cast<double>(num_samples) / sample_rate;

  // Keep phase in [0, 2π) to avoid precision loss.
  while (phase_ >= juce::MathConstants<double>::twoPi)
    phase_ -= juce::MathConstants<double>::twoPi;

  return result;
}

float LfoEngine::ApplyToMax(float max_val,
                            float lfo_norm) const noexcept {
  const float i = intensity_.load(std::memory_order_relaxed) * 0.01f;
  return max_val * (1.0f - i * (1.0f - lfo_norm));
}

void LfoEngine::SetRate(float rate_hz) noexcept {
  rate_.store(juce::jlimit(0.01f, 2.0f, rate_hz),
              std::memory_order_relaxed);
}

void LfoEngine::SetIntensity(float i) noexcept {
  intensity_.store(juce::jlimit(0.0f, 100.0f, i),
                   std::memory_order_relaxed);
}

void LfoEngine::SetMode(LfoMode m) noexcept {
  mode_.store(static_cast<int>(m), std::memory_order_relaxed);
}

LfoMode LfoEngine::GetMode() const noexcept {
  return static_cast<LfoMode>(mode_.load(std::memory_order_relaxed));
}

float LfoEngine::GetIntensity() const noexcept {
  return intensity_.load(std::memory_order_relaxed);
}

void LfoEngine::Reset() noexcept {
  phase_ = 0.0;
}
