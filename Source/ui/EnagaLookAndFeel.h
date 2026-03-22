/**
 * @file   EnagaLookAndFeel.h
 * @brief  Application-wide dark look and feel for Enaga.
 */

#pragma once

#include <juce_gui_basics/juce_gui_basics.h>

/**
 * Custom dark-mode look and feel applied globally to all Enaga controls.
 * Near-black background with a light-blue accent and high-contrast text.
 *
 * TextButton colour IDs are set here because LookAndFeel_V4::drawMenuBarItem
 * resolves menu bar item colours through them.
 *
 * All five theme colours are exposed as public static constexpr members so
 * that Main.cpp, MainComponent.cpp, and any future code can reference them
 * by name rather than repeating raw hex literals.
 */
class EnagaLookAndFeel final : public juce::LookAndFeel_V4
{
public:
    // -----------------------------------------------------------------------
    //  Theme colour palette (single source of truth for all hex literals)
    // -----------------------------------------------------------------------

    /** Main background: near-black. */
    static constexpr juce::uint32 kBackground = 0xff1a1a1a;

    /** Control panel backgrounds: dark grey. */
    static constexpr juce::uint32 kPanel = 0xff2d2d2d;

    /** Highlight / slider thumb: light blue. */
    static constexpr juce::uint32 kAccent = 0xff4fc3f7;

    /** Primary text: off-white. */
    static constexpr juce::uint32 kText = 0xffe0e0e0;

    /** Image area fill: slightly lighter than the main background. */
    static constexpr juce::uint32 kImageArea = 0xff222222;

    /** TextButton background (also used by MenuBarComponent rendering). */
    static constexpr juce::uint32 kButtonBackground = 0xff212121;

    // -----------------------------------------------------------------------

    EnagaLookAndFeel();

    /** Draws step-position tick marks below the track for sliders with a
     *  non-zero snap interval (i.e. the discrete Steps slider). */
    void drawLinearSlider(juce::Graphics& g, int x, int y, int width, int height,
                          float sliderPos, float minSliderPos, float maxSliderPos,
                          juce::Slider::SliderStyle style, juce::Slider& slider) override;
};
