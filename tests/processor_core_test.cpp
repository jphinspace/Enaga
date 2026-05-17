/**
 * @file   processor_core_test.cpp
 * @brief  Unit tests for EnagaProcessorCore.
 */

#include "processor_core.h"

#include <cmath>
#include <memory>

namespace {

class FakeNoiseSource final : public EnagaNoiseSource {
 public:
  void SetCutoff(float normalised_0_to_100) noexcept override {
    last_cutoff = normalised_0_to_100;
    set_cutoff_calls++;
  }

  void SetGain(float new_gain) noexcept override {
    last_gain = new_gain;
    set_gain_calls++;
  }

  void SetNoiseType(NoiseType type) noexcept override {
    last_noise_type = type;
    set_noise_type_calls++;
  }

  void SetLfoRate(float rate_hz) noexcept override {
    last_lfo_rate = rate_hz;
    set_lfo_rate_calls++;
  }

  void SetLfoIntensity(float intensity) noexcept override {
    last_lfo_intensity = intensity;
    set_lfo_intensity_calls++;
  }

  void SetLfoMode(LfoMode mode) noexcept override {
    last_lfo_mode = mode;
    set_lfo_mode_calls++;
  }

  void StartFadeIn() noexcept override { fade_in_calls++; }

  void StartFadeOut() noexcept override { fade_out_calls++; }

  void PrepareToPlay(int samples_per_block_expected,
                     double new_sample_rate) override {
    last_prepare_block_size = samples_per_block_expected;
    last_prepare_sample_rate = new_sample_rate;
    prepare_calls++;
  }

  void ReleaseResources() override { release_calls++; }

  void GetNextAudioBlock(const juce::AudioSourceChannelInfo& info) override {
    get_block_calls++;
    last_num_samples = info.numSamples;
    last_start_sample = info.startSample;
  }

  float last_cutoff = 0.0f;
  float last_gain = 0.0f;
  NoiseType last_noise_type = NoiseType::kWhite;
  float last_lfo_rate = 0.0f;
  float last_lfo_intensity = 0.0f;
  LfoMode last_lfo_mode = LfoMode::kDisabled;
  int last_prepare_block_size = 0;
  double last_prepare_sample_rate = 0.0;
  int last_num_samples = 0;
  int last_start_sample = -1;

  int set_cutoff_calls = 0;
  int set_gain_calls = 0;
  int set_noise_type_calls = 0;
  int set_lfo_rate_calls = 0;
  int set_lfo_intensity_calls = 0;
  int set_lfo_mode_calls = 0;
  int fade_in_calls = 0;
  int fade_out_calls = 0;
  int prepare_calls = 0;
  int release_calls = 0;
  int get_block_calls = 0;
};

bool RunProcessorCoreDelegationTest() {
  auto fake = std::make_unique<FakeNoiseSource>();
  FakeNoiseSource* raw = fake.get();

  EnagaProcessorCore core(std::move(fake));
  core.SetCutoff(42.0f);
  core.SetGain(0.4f);
  core.SetNoiseType(NoiseType::kBrown);
  core.SetLfoRate(1.5f);
  core.SetLfoIntensity(80.0f);
  core.SetLfoMode(LfoMode::kBoth);
  core.StartFadeIn();
  core.StartFadeOut();
  core.PrepareToPlay(48000.0, 256);

  juce::AudioBuffer<float> buffer(2, 256);
  core.ProcessBlock(buffer);
  core.ReleaseResources();

  return raw->set_cutoff_calls == 1 &&
         std::abs(raw->last_cutoff - 42.0f) < 0.001f &&
         raw->set_gain_calls == 1 && std::abs(raw->last_gain - 0.4f) < 0.001f &&
         raw->set_noise_type_calls == 1 &&
         raw->last_noise_type == NoiseType::kBrown &&
         raw->set_lfo_rate_calls == 1 &&
         std::abs(raw->last_lfo_rate - 1.5f) < 0.001f &&
         raw->set_lfo_intensity_calls == 1 &&
         std::abs(raw->last_lfo_intensity - 80.0f) < 0.001f &&
         raw->set_lfo_mode_calls == 1 && raw->last_lfo_mode == LfoMode::kBoth &&
         raw->fade_in_calls == 1 && raw->fade_out_calls == 1 &&
         raw->prepare_calls == 1 && raw->last_prepare_block_size == 256 &&
         std::abs(raw->last_prepare_sample_rate - 48000.0) < 0.001 &&
         raw->get_block_calls == 1 && raw->last_start_sample == 0 &&
         raw->last_num_samples == 256 && raw->release_calls == 1;
}

}  // namespace

bool RunProcessorCoreTests() { return RunProcessorCoreDelegationTest(); }
