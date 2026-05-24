/**
 * @file   lfo_mode.h
 * @brief  LFO modulation target enumeration.
 */

#ifndef ENAGA_AUDIO_LFO_MODE_H_
#define ENAGA_AUDIO_LFO_MODE_H_

/** Which audio parameter(s) the LFO modulates. */
enum class LfoMode {
  kDisabled = 0,  ///< No modulation.
  kVolume,        ///< Modulates output gain within the Volume-slider bound.
  kFilter,  ///< Modulates LP filter cutoff within the Cutoff-slider bound.
  kBoth     ///< Modulates both gain and cutoff simultaneously.
};

#endif  // ENAGA_AUDIO_LFO_MODE_H_
