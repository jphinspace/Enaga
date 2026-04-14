/**
 * @file   lfo_mode.h
 * @brief  LFO modulation target enumeration.
 */

#pragma once

/** Which audio parameter(s) the LFO modulates. */
enum class LfoMode {
  kDisabled = 0,  ///< No modulation.
  kVolume,        ///< Modulates output gain within the Volume-slider bound.
  kFilter,        ///< Modulates LP filter cutoff within the Cutoff-slider bound.
  kBoth           ///< Modulates both gain and cutoff simultaneously.
};
