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

#include <atomic>

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
    [[nodiscard]] float tick(int numSamples, double sampleRate) noexcept;

    /**
     * Apply LFO modulation to a user-defined maximum value.
     *
     * @param maxVal   The ceiling (set by the user's slider); never exceeded.
     * @param lfoNorm  Normalised LFO value from tick() in [0, 1].
     * @returns        A value in [maxVal * (1 - intensity/100), maxVal].
     */
    [[nodiscard]] float applyToMax(float maxVal, float lfoNorm) const noexcept;

    /**
     * Set the LFO oscillation rate.
     * @param rateHz  Rate in Hz; clamped to [0.01, 2.0].
     */
    void setRate(float rateHz) noexcept;

    /**
     * Set the LFO modulation depth.
     * @param i  Intensity on a 0–100 scale; 0 = no modulation, 100 = full swing.
     */
    void setIntensity(float i) noexcept;

    /** Set which audio parameter(s) the LFO modulates. */
    void setMode(LfoMode m) noexcept;

    /** Return the current LFO mode. */
    [[nodiscard]] LfoMode getMode() const noexcept;

    /** Return the current intensity in [0, 100]. */
    [[nodiscard]] float getIntensity() const noexcept;

    /** Reset the LFO phase to zero (audio thread). */
    void reset() noexcept;

private:
    std::atomic<float> rate      { 0.1f };  // Hz; clamped to [0.01, 2.0]
    std::atomic<float> intensity { 0.0f };  // 0–100
    std::atomic<int>   mode      { 0 };     // cast to LfoMode; written by UI thread
    double             phase     { 0.0 };   // radians; audio thread only
};
