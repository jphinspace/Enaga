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
    PlayButton() : juce::Button("play")
    {
        setClickingTogglesState(true);
    }

    void paintButton(juce::Graphics& g,
                     bool isMouseOver,
                     bool /*isButtonDown*/) override
    {
        const auto  bounds = getLocalBounds().toFloat().reduced(2.0f);
        const float dim    = juce::jmin(bounds.getWidth(), bounds.getHeight());
        const auto  sq     = juce::Rectangle<float>(dim, dim).withCentre(bounds.getCentre());
        const float alpha  = isMouseOver ? 0.80f : 1.0f;

        if (getToggleState())
        {
            // ON / playing → red rounded square (stop)
            g.setColour(juce::Colour(0xffe53935).withAlpha(alpha));
            g.fillRoundedRectangle(sq.reduced(dim * 0.20f), 5.0f);
        }
        else
        {
            // OFF / stopped → green triangle (play)
            g.setColour(juce::Colour(0xff43a047).withAlpha(alpha));
            juce::Path tri;
            const auto r = sq.reduced(dim * 0.10f);
            tri.addTriangle(r.getTopLeft(), r.getBottomLeft(),
                            { r.getRight(), r.getCentreY() });
            g.fillPath(tri);
        }
    }
};
