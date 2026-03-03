/**
 * @file   WhiteNoiseAudioSource.h
 * @brief  White noise generator with first-order Butterworth LP filter and gain control.
 */

#pragma once

#include <juce_audio_basics/juce_audio_basics.h>

#include <array>
#include <atomic>

/**
 * Produces full-scale white noise routed through a first-order Butterworth
 * low-pass IIR filter, then scaled by a gain factor.
 *
 * Both the cutoff (0–100 normalised scale → 20 Hz–20 kHz log) and gain
 * (0–1) are written by the UI thread via lock-free atomics and read on the
 * real-time audio thread using relaxed ordering.  Coefficient updates happen
 * at the start of the first audio block after each cutoff change.
 *
 * On desktop the gain is driven by the Volume slider; on iOS/Android the
 * atomic is left at 1.0f (full scale) and loudness is handled by the OS
 * media volume.
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

    void prepareToPlay(int samplesPerBlockExpected, double newSampleRate) override;
    void releaseResources() override;
    void getNextAudioBlock(const juce::AudioSourceChannelInfo& info) override;

private:
    /**
     * Recalculate IIR coefficients from the current @c lastCutoff value.
     * Must only be called on the audio thread (or before streaming starts).
     */
    void updateFilters();

    juce::Random            random;                // PRNG seeded from system entropy
    std::atomic<float>      cutoff { 100.0f };     // normalised 0–100; written by UI thread
    float                   lastCutoff { 100.0f }; // applied value (audio thread only)
    std::atomic<float>      gain   { 1.0f };       // amplitude multiplier [0,1]
    double                  sampleRate { 44100.0 };
    std::array<juce::IIRFilter, 2> filters;        // one per stereo channel
};
