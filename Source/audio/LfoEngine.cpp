/**
 * @file   LfoEngine.cpp
 * @brief  LfoEngine implementation.
 */

#include "audio/LfoEngine.h"

#include <juce_audio_basics/juce_audio_basics.h>

#include <cmath>

float LfoEngine::tick(int numSamples, double sampleRate) noexcept
{
    if (sampleRate <= 0.0) return 1.0f;

    // Sample at the start of the block, then advance.
    const float result = static_cast<float>((1.0 + std::sin(phase)) * 0.5);

    phase += static_cast<double>(rate.load(std::memory_order_relaxed))
             * juce::MathConstants<double>::twoPi
             * static_cast<double>(numSamples) / sampleRate;

    // Keep phase in [0, 2π) to avoid precision loss.
    while (phase >= juce::MathConstants<double>::twoPi)
        phase -= juce::MathConstants<double>::twoPi;

    return result;
}

float LfoEngine::applyToMax(float maxVal, float lfoNorm) const noexcept
{
    const float i = intensity.load(std::memory_order_relaxed) * 0.01f;
    return maxVal * (1.0f - i * (1.0f - lfoNorm));
}

void LfoEngine::setRate(float rateHz) noexcept
{
    rate.store(juce::jlimit(0.01f, 2.0f, rateHz), std::memory_order_relaxed);
}

void LfoEngine::setIntensity(float i) noexcept
{
    intensity.store(juce::jlimit(0.0f, 100.0f, i), std::memory_order_relaxed);
}

void LfoEngine::setMode(LfoMode m) noexcept
{
    mode.store(static_cast<int>(m), std::memory_order_relaxed);
}

LfoMode LfoEngine::getMode() const noexcept
{
    return static_cast<LfoMode>(mode.load(std::memory_order_relaxed));
}

float LfoEngine::getIntensity() const noexcept
{
    return intensity.load(std::memory_order_relaxed);
}

void LfoEngine::reset() noexcept
{
    phase = 0.0;
}
