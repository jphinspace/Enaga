/**
 * @file   noise_type.h
 * @brief  Spectral colour enumeration for the noise generator.
 */

#pragma once

/** Spectral colour of the generated noise. */
enum class NoiseType {
  kWhite = 0,  ///< Flat frequency spectrum (white noise).
  kPink,       ///< 1/f spectrum (−3 dB/octave).
  kBrown,      ///< 1/f² spectrum (−6 dB/octave, Brownian noise).
  kGrey        ///< Perceptually flat (inverse A-weighting shaped).
};
