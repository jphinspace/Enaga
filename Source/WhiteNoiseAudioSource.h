/**
 * @file   WhiteNoiseAudioSource.h
 * @brief  Noise generator (white/pink/brown/grey) with first-order Butterworth LP filter
 *         and gain control.
 */

#pragma once

#include <juce_audio_basics/juce_audio_basics.h>

#include <array>
#include <atomic>
#include <cstddef>

/** Spectral colour of the generated noise. */
enum class NoiseType { White = 0, Pink, Brown, Grey };

/**
 * Produces noise of a selectable spectral colour routed through a first-order Butterworth
 * low-pass IIR filter, then scaled by a gain factor.
 *
 * The cutoff (0–100 normalised scale → 20 Hz–20 kHz log), gain (0–1), and noise type are
 * written by the UI thread via lock-free atomics and read on the real-time audio thread
 * using relaxed ordering.  Coefficient updates happen at the start of the first audio block
 * after each cutoff change.
 *
 * Noise algorithms:
 *   - White : flat spectrum (uniform random samples).
 *   - Pink  : 1/f spectrum via Paul Kellett's parallel-filter approximation.
 *   - Brown : 1/f² spectrum via first-order IIR integration of white noise.
 *   - Grey  : perceptually flat (white noise shaped by the inverse A-weighting curve),
 *             approximated with a three-stage IIR shelf/peak filter chain.
 *
 * On desktop the gain is driven by the Volume slider; on iOS/Android the atomic is left at
 * 1.0f (full scale) and loudness is handled by the OS media volume.
 */
class WhiteNoiseAudioSource final : public juce::AudioSource
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
    /**
     * Recalculate LP IIR coefficients from the current @c lastCutoff value.
     * Must only be called on the audio thread (or before streaming starts).
     */
    void updateFilters();

    /**
     * Recalculate the grey-noise IIR filter chain from the current @c sampleRate.
     * Must only be called on the audio thread (or before streaming starts).
     */
    void updateGreyFilters();

    /** Generate one pink-noise sample for the given stereo channel (audio thread only). */
    [[nodiscard]] float generatePinkSample(std::size_t channel) noexcept;

    /** Generate one brown-noise sample for the given stereo channel (audio thread only). */
    [[nodiscard]] float generateBrownSample(std::size_t channel) noexcept;

    juce::Random            random;                // PRNG seeded from system entropy
    std::atomic<float>      cutoff { 100.0f };     // normalised 0–100; written by UI thread
    float                   lastCutoff { 100.0f }; // applied value (audio thread only)
    std::atomic<float>      gain   { 1.0f };       // amplitude multiplier [0,1]
    std::atomic<int>        noiseType { 0 };       // cast to NoiseType; written by UI thread
    std::atomic<float>      fadeTarget { 0.0f };   // 0=silent, 1=full; written by UI thread
    float                   fadeCurrent { 0.0f };  // current fade level (audio thread only)
    float                   fadeStep    { 0.0f };  // per-sample increment (audio thread only)
    double                  sampleRate { 44100.0 };

    std::array<juce::IIRFilter, 2> filters;        // LP filter, one per stereo channel

    // Pink noise: Paul Kellett parallel-filter state (7 state variables × 2 channels).
    std::array<std::array<float, 7>, 2> pinkState {};

    // Brown noise: running integration value, one per stereo channel.
    std::array<float, 2> brownState {};

    // Grey noise: three-stage IIR shelf/peak chain per stereo channel.
    // Stage 0 – low shelf boost  (~100 Hz)  compensates A-weighting bass rolloff.
    // Stage 1 – peak cut         (~3500 Hz) compensates A-weighting presence peak.
    // Stage 2 – high shelf boost (~10 kHz)  compensates A-weighting HF rolloff.
    static constexpr int   kGreyStages       = 3;
    static constexpr float kGreyCompensation = 0.05f; // post-filter amplitude normalisation
    std::array<std::array<juce::IIRFilter, kGreyStages>, 2> greyFilters {};
};
