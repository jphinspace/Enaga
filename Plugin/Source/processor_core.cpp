/**
 * @file   processor_core.cpp
 * @brief  EnagaProcessorCore implementation.
 */

#include "processor_core.h"

#include "audio/noise_audio_source.h"

namespace {

class NoiseAudioSourceAdapter final : public EnagaNoiseSource {
 public:
  void SetCutoff(float normalised_0_to_100) noexcept override {
    source_.SetCutoff(normalised_0_to_100);
  }

  void SetGain(float new_gain) noexcept override { source_.SetGain(new_gain); }

  void SetNoiseType(NoiseType type) noexcept override {
    source_.SetNoiseType(type);
  }

  void SetLfoRate(float rate_hz) noexcept override {
    source_.SetLfoRate(rate_hz);
  }

  void SetLfoIntensity(float intensity) noexcept override {
    source_.SetLfoIntensity(intensity);
  }

  void SetLfoMode(LfoMode mode) noexcept override { source_.SetLfoMode(mode); }

  void StartFadeIn() noexcept override { source_.StartFadeIn(); }

  void StartFadeOut() noexcept override { source_.StartFadeOut(); }

  void PrepareToPlay(int samples_per_block_expected,
                     double new_sample_rate) override {
    source_.prepareToPlay(samples_per_block_expected, new_sample_rate);
  }

  void ReleaseResources() override { source_.releaseResources(); }

  void GetNextAudioBlock(const juce::AudioSourceChannelInfo& info) override {
    source_.getNextAudioBlock(info);
  }

 private:
  NoiseAudioSource source_;
};

}  // namespace

EnagaProcessorCore::EnagaProcessorCore()
    : EnagaProcessorCore(std::make_unique<NoiseAudioSourceAdapter>()) {}

EnagaProcessorCore::EnagaProcessorCore(
    std::unique_ptr<EnagaNoiseSource> noise_source)
    : noise_source_(std::move(noise_source)) {}

void EnagaProcessorCore::SetCutoff(float normalised_0_to_100) noexcept {
  noise_source_->SetCutoff(normalised_0_to_100);
}

void EnagaProcessorCore::SetGain(float new_gain) noexcept {
  noise_source_->SetGain(new_gain);
}

void EnagaProcessorCore::SetNoiseType(NoiseType type) noexcept {
  noise_source_->SetNoiseType(type);
}

void EnagaProcessorCore::SetLfoRate(float rate_hz) noexcept {
  noise_source_->SetLfoRate(rate_hz);
}

void EnagaProcessorCore::SetLfoIntensity(float intensity) noexcept {
  noise_source_->SetLfoIntensity(intensity);
}

void EnagaProcessorCore::SetLfoMode(LfoMode mode) noexcept {
  noise_source_->SetLfoMode(mode);
}

void EnagaProcessorCore::StartFadeIn() noexcept {
  noise_source_->StartFadeIn();
}

void EnagaProcessorCore::StartFadeOut() noexcept {
  noise_source_->StartFadeOut();
}

void EnagaProcessorCore::PrepareToPlay(double sample_rate,
                                       int maximum_expected_samples_per_block) {
  noise_source_->PrepareToPlay(maximum_expected_samples_per_block, sample_rate);
}

void EnagaProcessorCore::ReleaseResources() {
  noise_source_->ReleaseResources();
}

void EnagaProcessorCore::ProcessBlock(juce::AudioBuffer<float>& buffer) {
  juce::AudioSourceChannelInfo info(&buffer, 0, buffer.getNumSamples());
  noise_source_->GetNextAudioBlock(info);
}
