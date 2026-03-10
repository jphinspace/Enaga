/**
 * @file   LfoMode.h
 * @brief  LFO modulation target enumeration.
 */

#pragma once

/** Which audio parameter(s) the LFO modulates. */
enum class LfoMode
{
    Disabled = 0, ///< No modulation.
    Volume,       ///< Modulates output gain within the Volume-slider bound.
    Filter,       ///< Modulates LP filter cutoff within the Cutoff-slider bound.
    Both          ///< Modulates both gain and cutoff simultaneously.
};
