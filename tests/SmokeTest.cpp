/**
 * @file   SmokeTest.cpp
 * @brief  Offline smoke test for NoiseAudioSource.
 *
 * Constructs a NoiseAudioSource, calls prepareToPlay(), then exercises
 * getNextAudioBlock() for all four noise types to verify that:
 *   - No assertions or exceptions are thrown.
 *   - Output samples are finite (not NaN / Inf).
 *
 * Runs entirely offline (no audio device required) so it can execute in CI
 * without any audio hardware.
 */

// TODO:C++26  Replace #include with module imports once JUCE ships modules.

#include "audio/NoiseAudioSource.h"

#include <cassert>
#include <cmath>
#include <cstdio>
#include <vector>

// ---------------------------------------------------------------------------
//  Minimal JUCE stub: include the JUCE headers we need for an offline test.
// ---------------------------------------------------------------------------
#include <juce_audio_basics/juce_audio_basics.h>

static bool runSmokeTest()
{
    constexpr double kSampleRate        = 44100.0;
    constexpr int    kSamplesPerBlock   = 512;
    constexpr int    kNumChannels       = 2;
    constexpr int    kBlocksPerNoiseType = 4;

    NoiseAudioSource source;
    source.prepareToPlay(kSamplesPerBlock, kSampleRate);

    // Exercise all four noise types.
    const NoiseType noiseTypes[] = {
        NoiseType::White,
        NoiseType::Pink,
        NoiseType::Brown,
        NoiseType::Grey
    };

    juce::AudioBuffer<float> buffer(kNumChannels, kSamplesPerBlock);

    for (const auto type : noiseTypes)
    {
        source.setNoiseType(type);
        source.startFadeIn();

        for (int block = 0; block < kBlocksPerNoiseType; ++block)
        {
            buffer.clear();
            juce::AudioSourceChannelInfo info(&buffer, 0, kSamplesPerBlock);
            source.getNextAudioBlock(info);

            // Verify all samples are finite.
            for (int ch = 0; ch < kNumChannels; ++ch)
            {
                const float* data = buffer.getReadPointer(ch);
                for (int i = 0; i < kSamplesPerBlock; ++i)
                {
                    if (!std::isfinite(data[i]))
                    {
                        std::fprintf(stderr,
                            "FAIL: non-finite sample at NoiseType=%d ch=%d i=%d value=%f\n",
                            static_cast<int>(type), ch, i, static_cast<double>(data[i]));
                        return false;
                    }
                }
            }
        }

        source.startFadeOut();
    }

    source.releaseResources();
    std::puts("PASS: NoiseAudioSource smoke test");
    return true;
}

int main()
{
    return runSmokeTest() ? 0 : 1;
}
