/**
 * @file   entry.cpp
 * @brief  Plugin factory function required by juce_add_plugin.
 *
 * This file is compiled into plugin binary targets (VST3, AU, …).
 */

#include <memory>

#include "processor.h"

// createPluginFilter() is the single entry-point that JUCE's plugin wrapper
// calls to obtain the AudioProcessor instance. The macro JUCE_CALLTYPE
// expands to the correct calling convention on each platform.
juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter() {
  return std::make_unique<EnagaProcessor>().release();
}
