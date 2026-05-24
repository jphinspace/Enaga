/**
 * @file   play_button.h
 * @brief  Custom toggle button drawn as a play triangle or stop square.
 */

#ifndef ENAGA_UI_PLAY_BUTTON_H_
#define ENAGA_UI_PLAY_BUTTON_H_

#include <juce_gui_basics/juce_gui_basics.h>

/**
 * Custom toggle button rendered as:
 *   - A green right-pointing triangle when toggle state is false (noise off).
 *   - A red rounded square when toggle state is true (noise on).
 *
 * No text is displayed.
 */
class PlayButton final : public juce::Button {
 public:
  /** Colour used to paint the play (triangle) state. */
  static constexpr juce::uint32 kPlayColour = 0xff43a047;

  /** Colour used to paint the stop (square) state. */
  static constexpr juce::uint32 kStopColour = 0xffe53935;

  PlayButton();

  void paintButton(juce::Graphics& g, bool is_mouse_over,
                   bool is_button_down) override;
};

#endif  // ENAGA_UI_PLAY_BUTTON_H_
