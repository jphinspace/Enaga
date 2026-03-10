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

    // -----------------------------------------------------------------------

    EnagaLookAndFeel()
    {
        const auto bg      = juce::Colour(kBackground);
        const auto panel   = juce::Colour(kPanel);
        const auto accent  = juce::Colour(kAccent);
        const auto textCol = juce::Colour(kText);

        setColour(juce::ResizableWindow::backgroundColourId,          bg);
        setColour(juce::DocumentWindow::backgroundColourId,           bg);
        setColour(juce::Slider::backgroundColourId,                   panel);
        setColour(juce::Slider::thumbColourId,                        accent);
        setColour(juce::Slider::trackColourId,                        accent);
        setColour(juce::TextEditor::backgroundColourId,               panel);
        setColour(juce::TextEditor::textColourId,                     textCol);
        setColour(juce::TextEditor::outlineColourId,                  accent);
        setColour(juce::TextEditor::focusedOutlineColourId,           accent.brighter(0.3f));
        setColour(juce::Label::textColourId,                          textCol);
        setColour(juce::PopupMenu::backgroundColourId,                panel);
        setColour(juce::PopupMenu::textColourId,                      textCol);
        setColour(juce::PopupMenu::highlightedBackgroundColourId,     accent);
        setColour(juce::PopupMenu::highlightedTextColourId,           bg);
        // TextButton colours are also used by MenuBarComponent rendering
        // (see LookAndFeel_V4::drawMenuBarBackground / drawMenuBarItem).
        setColour(juce::TextButton::buttonColourId,                   juce::Colour(0xff212121));
        setColour(juce::TextButton::buttonOnColourId,                 accent);
        setColour(juce::TextButton::textColourOffId,                  textCol);
        setColour(juce::TextButton::textColourOnId,                   bg);
    }

    /** Draws step-position tick marks below the track for sliders with a
     *  non-zero snap interval (i.e. the discrete Steps slider). */
    void drawLinearSlider(juce::Graphics& g, int x, int y, int width, int height,
                          float sliderPos, float minSliderPos, float maxSliderPos,
                          juce::Slider::SliderStyle style, juce::Slider& slider) override
    {
        LookAndFeel_V4::drawLinearSlider(g, x, y, width, height,
                                         sliderPos, minSliderPos, maxSliderPos,
                                         style, slider);

        // Only add tick marks for horizontal sliders with a visible snap interval.
        if (style != juce::Slider::LinearHorizontal)
            return;

        const double interval = slider.getInterval();
        if (interval <= 0.0)
            return;

        const double lo   = slider.getMinimum();
        const double hi   = slider.getMaximum();
        const double span = hi - lo;

        const auto tickColour = juce::Colour(kAccent).withAlpha(0.55f);
        const int  tickH      = 6;   // height of each tick mark (pixels)
        const int  tickW      = 2;   // width  of each tick mark (pixels)
        // Position ticks just below the slider track centre.
        const int  tickY      = y + height / 2 + 5;

        for (double v = lo; v <= hi + interval * 0.01; v += interval)
        {
            const float proportion = static_cast<float>((v - lo) / span);
            const int   tickX      = x + juce::roundToInt(proportion * static_cast<float>(width))
                                       - tickW / 2;
            g.setColour(tickColour);
            g.fillRect(tickX, tickY, tickW, tickH);
        }
    }
};
