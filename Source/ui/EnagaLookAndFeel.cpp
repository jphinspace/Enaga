/**
 * @file   EnagaLookAndFeel.cpp
 * @brief  EnagaLookAndFeel implementation.
 */

#include "ui/EnagaLookAndFeel.h"

EnagaLookAndFeel::EnagaLookAndFeel()
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

void EnagaLookAndFeel::drawLinearSlider(juce::Graphics& g, int x, int y, int width, int height,
                                        float sliderPos, float minSliderPos, float maxSliderPos,
                                        juce::Slider::SliderStyle style, juce::Slider& slider)
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
    const int  tickH      = 6;
    const int  tickW      = 2;
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
