/**
 * @file   LfoEngine.h
 * @brief  Thread-safe, sine-wave low-frequency oscillator used to modulate
 *         volume and/or low-pass filter cutoff within user-set bounds.
 *
 * Rate, intensity, and mode are written by the UI thread via lock-free atomics.
 * Phase is maintained exclusively on the audio thread.
 *
 * Usage (audio thread):
 * @code
 *   // Once per buffer:
 *   const float lfoNorm = engine.tick(numSamples, sampleRate); // [0, 1]
 *   const float modulated = engine.applyToMax(maxValue, lfoNorm);
 * @endcode
 */

#pragma once

#include "audio/LfoMode.h"

#include <juce_audio_basics/juce_audio_basics.h>

#include <atomic>
#include <cmath>

/**
 * Sine-wave LFO oscillator with lock-free parameter updates.
 *
 * The LFO produces a normalised sine value in [0, 1] where 1 is the peak
 * (full value) and 0 is the trough (most reduced value).  Intensity maps
 * 0–100 → no modulation–full modulation: at intensity 100 the output swings
 * from the user's maximum value all the way down to zero.
 */
class LfoEngine final
{
public:
    /**
     * Advance the LFO phase by @p numSamples and return the LFO value at
     * the start of the block, normalised to [0, 1].
     *
     * Must be called exclusively from the audio thread.
     *
     * @param numSamples  Number of samples in the current audio buffer.
     * @param sampleRate  Current hardware sample rate in Hz.
     * @returns           Normalised LFO value: 1 = peak, 0 = trough.
     */
    [[nodiscard]] float tick(int numSamples, double sampleRate) noexcept
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

    /**
     * Apply LFO modulation to a user-defined maximum value.
     *
     * @param maxVal   The ceiling (set by the user's slider); never exceeded.
     * @param lfoNorm  Normalised LFO value from tick() in [0, 1].
     * @returns        A value in [maxVal * (1 - intensity/100), maxVal].
     */
    [[nodiscard]] float applyToMax(float maxVal, float lfoNorm) const noexcept
    {
        const float i = intensity.load(std::memory_order_relaxed) * 0.01f;
        return maxVal * (1.0f - i * (1.0f - lfoNorm));
    }

    /**
     * Set the LFO oscillation rate.
     * @param rateHz  Rate in Hz; clamped to [0.01, 2.0].
     */
    void setRate(float rateHz) noexcept
    {
        rate.store(juce::jlimit(0.01f, 2.0f, rateHz), std::memory_order_relaxed);
    }

    /**
     * Set the LFO modulation depth.
     * @param i  Intensity on a 0–100 scale; 0 = no modulation, 100 = full swing.
     */
    void setIntensity(float i) noexcept
    {
        intensity.store(juce::jlimit(0.0f, 100.0f, i), std::memory_order_relaxed);
    }

    /** Set which audio parameter(s) the LFO modulates. */
    void setMode(LfoMode m) noexcept
    {
        mode.store(static_cast<int>(m), std::memory_order_relaxed);
    }

    /** Return the current LFO mode. */
    [[nodiscard]] LfoMode getMode() const noexcept
    {
        return static_cast<LfoMode>(mode.load(std::memory_order_relaxed));
    }

    /** Return the current intensity in [0, 100]. */
    [[nodiscard]] float getIntensity() const noexcept
    {
        return intensity.load(std::memory_order_relaxed);
    }

    /** Reset the LFO phase to zero (audio thread). */
    void reset() noexcept { phase = 0.0; }

private:
    std::atomic<float> rate      { 0.1f };  // Hz; clamped to [0.01, 2.0]
    std::atomic<float> intensity { 0.0f };  // 0–100
    std::atomic<int>   mode      { 0 };     // cast to LfoMode; written by UI thread
    double             phase     { 0.0 };   // radians; audio thread only
};
