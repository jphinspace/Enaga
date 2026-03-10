/**
 * @file   NoiseType.h
 * @brief  Spectral colour enumeration for the noise generator.
 */

#pragma once

/** Spectral colour of the generated noise. */
enum class NoiseType
{
    White = 0, ///< Flat frequency spectrum (white noise).
    Pink,      ///< 1/f spectrum (−3 dB/octave).
    Brown,     ///< 1/f² spectrum (−6 dB/octave, Brownian noise).
    Grey       ///< Perceptually flat (inverse A-weighting shaped).
};
