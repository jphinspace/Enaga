/**
 * @file   NoiseAudioSource.h
 * @brief  JUCE AudioSource that delegates sample generation to a pluggable
 *         NoiseGenerator, then applies a low-pass filter, gain, fade, and LFO.
 */

#pragma once

#include "BrownNoiseGenerator.h"
#include "GreyNoiseGenerator.h"
#include "LfoEngine.h"
#include "PinkNoiseGenerator.h"
#include "WhiteNoiseGenerator.h"

#include <juce_audio_basics/juce_audio_basics.h>

#include <array>
#include <atomic>

/** Spectral colour of the generated noise. */
enum class NoiseType { White = 0, Pink, Brown, Grey };

/**
 * JUCE AudioSource that generates noise of a selectable spectral colour.
 *
 * The noise type, LP-filter cutoff (0–100 normalised → 20 Hz–20 kHz log),
 * gain, and LFO parameters are written by the UI thread via lock-free atomics
 * and applied on the audio thread.  All four generator objects are kept alive
 * for the lifetime of this source so that switching types is always safe.
 *
 * On desktop, gain is driven by the Volume slider.  On iOS/Android the
 * atomic is left at 1.0f and loudness is handled by the OS media volume.
 *
 * The LFO modulates gain and/or cutoff within the bounds set by the user's
 * existing Volume and Cutoff sliders (it never exceeds those values).
 */
class NoiseAudioSource final : public juce::AudioSource
{
public:
    /**
     * Set the low-pass filter cutoff on a normalised 0–100 scale.
     * 0 → 20 Hz (heavy filtering), 100 → 20 kHz (wide open, near full spectrum).
     * Thread-safe: called from the message thread, applied on the audio thread.
     */
    void setCutoff(float normalised0to100) noexcept;

    /**
     * Set output amplitude multiplier in [0, 1].
     * Thread-safe: called from the message thread, read on the audio thread.
     */
    void setGain(float newGain) noexcept;

    /**
     * Set the spectral colour of the generated noise.
     * Thread-safe: called from the message thread, applied on the audio thread.
     */
    void setNoiseType(NoiseType type) noexcept;

    // -----------------------------------------------------------------------
    //  LFO controls
    // -----------------------------------------------------------------------

    /**
     * Set the LFO oscillation rate.
     * @param rateHz  Rate in Hz; clamped to [0.01, 2.0].
     */
    void setLfoRate(float rateHz) noexcept;

    /**
     * Set the LFO modulation depth.
     * @param i  Intensity on a 0–100 scale; 0 = no modulation, 100 = full swing.
     */
    void setLfoIntensity(float i) noexcept;

    /**
     * Set which audio parameter(s) the LFO modulates.
     * The LFO never exceeds the bounds set by the Volume and Cutoff sliders.
     */
    void setLfoMode(LfoMode mode) noexcept;

    /**
     * Begin a linear fade-in from silence to full level over @c fadeDurationSeconds.
     * Thread-safe: written by the message thread, applied on the audio thread.
     */
    void startFadeIn() noexcept;

    /**
     * Begin a linear fade-out from full level to silence over @c fadeDurationSeconds.
     * Thread-safe: written by the message thread, applied on the audio thread.
     */
    void startFadeOut() noexcept;

    /** Duration of the fade-in and fade-out ramp in seconds. */
    static constexpr float fadeDurationSeconds = 0.25f;

    void prepareToPlay(int samplesPerBlockExpected, double newSampleRate) override;
    void releaseResources() override;
    void getNextAudioBlock(const juce::AudioSourceChannelInfo& info) override;

private:
    /** Recompute LP IIR coefficients from @c lastCutoff (audio thread only). */
    void updateLpFilters();

    /** Return a pointer to the currently active generator (audio thread only). */
    [[nodiscard]] NoiseGenerator* activeGenerator() noexcept;

    // One generator per noise type – all prepared simultaneously so that
    // switching between types never requires a re-initialise.
    WhiteNoiseGenerator whiteGen;
    PinkNoiseGenerator  pinkGen;
    BrownNoiseGenerator brownGen;
    GreyNoiseGenerator  greyGen;

    std::atomic<int>   noiseType   { 0 };      // cast to NoiseType; written by UI thread
    std::atomic<float> cutoff      { 100.0f }; // normalised 0–100; written by UI thread
    float              lastCutoff  { 100.0f }; // applied value (audio thread only)
    std::atomic<float> gain        { 1.0f };   // amplitude multiplier [0, 1]
    std::atomic<float> fadeTarget  { 0.0f };   // 0 = silent, 1 = full
    float              fadeCurrent { 0.0f };   // current fade level (audio thread only)
    float              fadeStep    { 0.0f };   // per-sample ramp increment (audio thread only)
    double             sampleRate  { 44100.0 };

    std::array<juce::IIRFilter, 2> lpFilters; // one per stereo channel

    LfoEngine lfo; // LFO oscillator; rate/intensity/mode via atomics, phase audio-thread only
};
