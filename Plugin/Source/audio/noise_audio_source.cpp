/**
 * @file   noise_audio_source.cpp
 * @brief  NoiseAudioSource implementation.
 */

#include "audio/noise_audio_source.h"

#include <cmath>

void NoiseAudioSource::SetCutoff(float normalised_0_to_100) noexcept {
  cutoff_.store(juce::jlimit(0.0f, 100.0f, normalised_0_to_100),
                std::memory_order_relaxed);
}

void NoiseAudioSource::SetGain(float new_gain) noexcept {
  gain_.store(juce::jlimit(0.0f, 1.0f, new_gain),
              std::memory_order_relaxed);
}

void NoiseAudioSource::SetNoiseType(NoiseType type) noexcept {
  noise_type_.store(static_cast<int>(type), std::memory_order_relaxed);
}

void NoiseAudioSource::SetLfoRate(float rate_hz) noexcept {
  lfo_.SetRate(rate_hz);
}

void NoiseAudioSource::SetLfoIntensity(float i) noexcept {
  lfo_.SetIntensity(i);
}

void NoiseAudioSource::SetLfoMode(LfoMode mode) noexcept {
  lfo_.SetMode(mode);
}

void NoiseAudioSource::StartFadeIn() noexcept {
  fade_target_.store(1.0f, std::memory_order_relaxed);
}

void NoiseAudioSource::StartFadeOut() noexcept {
  fade_target_.store(0.0f, std::memory_order_relaxed);
}

void NoiseAudioSource::prepareToPlay(
    int /*samples_per_block_expected*/, double new_sample_rate) {
  sample_rate_  = new_sample_rate;
  fade_current_ = fade_target_.load(std::memory_order_relaxed);
  fade_step_    = (sample_rate_ > 0.0)
      ? 1.0f / (kFadeDurationSeconds *
                static_cast<float>(sample_rate_))
      : 0.0f;

  last_cutoff_ = cutoff_.load(std::memory_order_relaxed);
  UpdateLpFilters();
  for (auto& f : lp_filters_)
    f.reset();

  lfo_.Reset();

  white_gen_.Prepare(sample_rate_);
  pink_gen_.Prepare(sample_rate_);
  brown_gen_.Prepare(sample_rate_);
  grey_gen_.Prepare(sample_rate_);
}

void NoiseAudioSource::releaseResources() {
  for (auto& f : lp_filters_)
    f.reset();

  white_gen_.Reset();
  pink_gen_.Reset();
  brown_gen_.Reset();
  grey_gen_.Reset();
}

NoiseGenerator* NoiseAudioSource::ActiveGenerator() noexcept {
  switch (static_cast<NoiseType>(
      noise_type_.load(std::memory_order_relaxed))) {
    case NoiseType::kPink:  return &pink_gen_;
    case NoiseType::kBrown: return &brown_gen_;
    case NoiseType::kGrey:  return &grey_gen_;
    default:                return &white_gen_;
  }
}

void NoiseAudioSource::getNextAudioBlock(
    const juce::AudioSourceChannelInfo& info) {
  // Advance the LFO once per block and obtain its normalised value [0,1].
  const float lfo_norm       = lfo_.Tick(info.numSamples, sample_rate_);
  const LfoMode lfo_mode_val = lfo_.GetMode();

  // Effective cutoff: apply LFO modulation when mode includes Filter.
  const float user_cutoff = cutoff_.load(std::memory_order_relaxed);
  const float effective_cutoff =
      (lfo_mode_val == LfoMode::kFilter ||
       lfo_mode_val == LfoMode::kBoth)
      ? lfo_.ApplyToMax(user_cutoff, lfo_norm)
      : user_cutoff;

  // Re-coefficient the LP filter when the effective cutoff changes.
  if (effective_cutoff != last_cutoff_) {
    last_cutoff_ = effective_cutoff;
    UpdateLpFilters();
  }

  const float g      = gain_.load(std::memory_order_relaxed);
  const float target = fade_target_.load(std::memory_order_relaxed);

  // LFO gain multiplier: 1.0 when not modulating volume.
  const float lfo_gain_mod =
      (lfo_mode_val == LfoMode::kVolume ||
       lfo_mode_val == LfoMode::kBoth)
      ? lfo_.ApplyToMax(1.0f, lfo_norm)
      : 1.0f;

  // Read the noise type once so the whole block uses the same generator.
  NoiseGenerator* gen = ActiveGenerator();

  for (int ch = 0; ch < info.buffer->getNumChannels(); ++ch) {
    auto* out = info.buffer->getWritePointer(ch, info.startSample);
    const auto ch_idx =
        static_cast<std::size_t>(juce::jmin(ch, 1));

    for (int i = 0; i < info.numSamples; ++i)
      out[i] = gen->NextSample(ch_idx);

    lp_filters_[ch_idx].processSamples(out, info.numSamples);

    // Apply user gain, LFO gain modifier, and per-sample fade ramp.
    // All channels start from the same fade_current_ so they stay in sync;
    // fade_current_ is updated only once (after channel 0).
    float fc = fade_current_;
    for (int i = 0; i < info.numSamples; ++i) {
      out[i] *= g * lfo_gain_mod * fc;
      if (fc != target)
        fc = (target > fc) ? juce::jmin(fc + fade_step_, target)
                           : juce::jmax(fc - fade_step_, target);
    }

    if (ch == 0)
      fade_current_ = fc;
  }
}

void NoiseAudioSource::UpdateLpFilters() {
  if (sample_rate_ <= 0.0) return;

  // Logarithmic map: 0 → 20 Hz, 100 → 20 000 Hz.
  const double freq = 20.0 * std::pow(
      1000.0, static_cast<double>(last_cutoff_) / 100.0);
  const auto coef =
      juce::IIRCoefficients::makeLowPass(sample_rate_, freq);
  for (auto& f : lp_filters_)
    f.setCoefficients(coef);
}
