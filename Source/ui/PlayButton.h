/**
 * @file   PlayButton.h
 * @brief  Custom toggle button drawn as a play triangle or stop square.
 */

#pragma once

#include <juce_gui_basics/juce_gui_basics.h>

/**
 * Custom toggle button rendered as:
 *   - A green right-pointing triangle when toggle state is false (noise off).
 *   - A red rounded square when toggle state is true  (noise on).
 *
 * No text is displayed.
 */
class PlayButton final : public juce::Button
{
public:
    PlayButton();

    void paintButton(juce::Graphics& g,
                     bool isMouseOver,
                     bool isButtonDown) override;
};
