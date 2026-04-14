/**
 * @file   enaga_look_and_feel.cpp
 * @brief  EnagaLookAndFeel implementation.
 */

#include "ui/enaga_look_and_feel.h"

EnagaLookAndFeel::EnagaLookAndFeel() {
  const auto bg = juce::Colour(kBackground);
  const auto panel = juce::Colour(kPanel);
  const auto accent = juce::Colour(kAccent);
  const auto text_col = juce::Colour(kText);

  setColour(juce::ResizableWindow::backgroundColourId, bg);
  setColour(juce::DocumentWindow::backgroundColourId, bg);
  setColour(juce::Slider::backgroundColourId, panel);
  setColour(juce::Slider::thumbColourId, accent);
  setColour(juce::Slider::trackColourId, accent);
  setColour(juce::TextEditor::backgroundColourId, panel);
  setColour(juce::TextEditor::textColourId, text_col);
  setColour(juce::TextEditor::outlineColourId, accent);
  setColour(juce::TextEditor::focusedOutlineColourId, accent.brighter(0.3f));
  setColour(juce::Label::textColourId, text_col);
  setColour(juce::PopupMenu::backgroundColourId, panel);
  setColour(juce::PopupMenu::textColourId, text_col);
  setColour(juce::PopupMenu::highlightedBackgroundColourId, accent);
  setColour(juce::PopupMenu::highlightedTextColourId, bg);
  // TextButton colours are also used by MenuBarComponent rendering.
  setColour(juce::TextButton::buttonColourId, juce::Colour(kButtonBackground));
  setColour(juce::TextButton::buttonOnColourId, accent);
  setColour(juce::TextButton::textColourOffId, text_col);
  setColour(juce::TextButton::textColourOnId, bg);
}

void EnagaLookAndFeel::drawLinearSlider(juce::Graphics& g, int x, int y,
                                        int width, int height, float slider_pos,
                                        float min_slider_pos,
                                        float max_slider_pos,
                                        juce::Slider::SliderStyle style,
                                        juce::Slider& slider) {
  LookAndFeel_V4::drawLinearSlider(g, x, y, width, height, slider_pos,
                                   min_slider_pos, max_slider_pos, style,
                                   slider);

  // Only add tick marks for horizontal sliders with a snap interval.
  if (style != juce::Slider::LinearHorizontal) return;

  const double interval = slider.getInterval();
  if (interval <= 0.0) return;

  const double lo = slider.getMinimum();
  const double hi = slider.getMaximum();
  const double span = hi - lo;

  const auto tick_colour = juce::Colour(kAccent).withAlpha(0.55f);
  const int tick_h = 6;
  const int tick_w = 2;
  // Position ticks just below the slider track centre.
  const int tick_y = y + height / 2 + 5;

  for (double v = lo; v <= hi + interval * 0.01; v += interval) {
    const float proportion = static_cast<float>((v - lo) / span);
    const int tick_x =
        x + juce::roundToInt(proportion * static_cast<float>(width)) -
        tick_w / 2;
    g.setColour(tick_colour);
    g.fillRect(tick_x, tick_y, tick_w, tick_h);
  }
}
