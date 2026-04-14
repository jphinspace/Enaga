/**
 * @file   plugin_entry.cpp
 * @brief  Plugin factory function required by juce_add_plugin.
 *
 * This file is compiled only into the plugin binary targets (VST3, AU, …).
 * It is NOT included in the Host build because the host instantiates
 * EnagaProcessor directly via new.
 */

#include "plugin_processor.h"

// createPluginFilter() is the single entry-point that JUCE's plugin wrapper
// calls to obtain the AudioProcessor instance. The macro JUCE_CALLTYPE
// expands to the correct calling convention on each platform.
juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter() {
  return new EnagaProcessor();
}
