/**
 * @file   play_button.cpp
 * @brief  PlayButton implementation.
 */

#include "ui/play_button.h"

PlayButton::PlayButton() : juce::Button("play") {
  setClickingTogglesState(true);
}

void PlayButton::paintButton(juce::Graphics& g,
                             bool is_mouse_over,
                             bool /*is_button_down*/) {
  const auto  bounds = getLocalBounds().toFloat().reduced(2.0f);
  const float dim    = juce::jmin(bounds.getWidth(),
                                  bounds.getHeight());
  const auto  sq     = juce::Rectangle<float>(dim, dim)
                           .withCentre(bounds.getCentre());
  const float alpha  = is_mouse_over ? 0.80f : 1.0f;

  if (getToggleState()) {
    // ON / playing → red rounded square (stop)
    g.setColour(juce::Colour(kStopColour).withAlpha(alpha));
    g.fillRoundedRectangle(sq.reduced(dim * 0.20f), 5.0f);
  } else {
    // OFF / stopped → green triangle (play)
    g.setColour(juce::Colour(kPlayColour).withAlpha(alpha));
    juce::Path tri;
    const auto r = sq.reduced(dim * 0.10f);
    tri.addTriangle(r.getTopLeft(), r.getBottomLeft(),
                    { r.getRight(), r.getCentreY() });
    g.fillPath(tri);
  }
}
