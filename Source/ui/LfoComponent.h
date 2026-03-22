/**
 * @file   LfoComponent.h
 * @brief  Self-contained UI component that exposes LFO controls to the user.
 *
 * Provides:
 *   - A mode button that cycles: Disabled → Volume → Filter → Both → Disabled.
 *   - A "LFO Rate" slider (0.01–2.0 Hz, skewed so 0.2 Hz is at mid-travel) with a numeric text box.
 *   - An "LFO Intensity" slider (0–100) paired with a numeric text box.
 *
 * All three callbacks fire on the message thread immediately when a control
 * changes value.
 */

#pragma once

#include "audio/LfoEngine.h"

#include <juce_gui_basics/juce_gui_basics.h>

#include <functional>

/**
 * UI panel for the LFO section.
 *
 * Place this component inside a parent and call setBounds() from the parent's
 * resized() override.  The component manages its own internal layout.
 */
class LfoComponent final : public juce::Component
{
public:
    using RateCallback      = std::function<void(float /*rateHz*/)>;
    using IntensityCallback = std::function<void(float /*0-100*/)>;
    using ModeCallback      = std::function<void(LfoMode)>;

    LfoComponent(RateCallback      onRate,
                 IntensityCallback onIntensity,
                 ModeCallback      onMode);

    void resized() override;

private:
    // -----------------------------------------------------------------------
    //  Setup helpers
    // -----------------------------------------------------------------------

    void setupModeButton();
    void setupRateSlider();
    void setupRateValueBox();
    void setupIntensitySlider();
    void setupIntensityValueBox();
    void setupLabels();

    // -----------------------------------------------------------------------
    //  Mode button
    // -----------------------------------------------------------------------

    void updateModeButtonText();

    // -----------------------------------------------------------------------
    //  Rate value-box / slider synchronisation
    // -----------------------------------------------------------------------

    void syncRateValueBox();
    void applyRateValueBox();

    // -----------------------------------------------------------------------
    //  Intensity value-box / slider synchronisation
    // -----------------------------------------------------------------------

    void syncIntensityValueBox();
    void applyIntensityValueBox();

    // -----------------------------------------------------------------------
    //  Callbacks
    // -----------------------------------------------------------------------

    RateCallback      onRate;
    IntensityCallback onIntensity;
    ModeCallback      onMode;

    LfoMode currentMode { LfoMode::Disabled };

    // -----------------------------------------------------------------------
    //  Child controls (declaration order matches dependency / destruction)
    // -----------------------------------------------------------------------

    juce::TextButton modeButton;
    juce::Label      rateLabel;
    juce::Slider     rateSlider;
    juce::TextEditor rateValueBox;
    juce::Label      intensityLabel;
    juce::Slider     intensitySlider;
    juce::TextEditor intensityValueBox;
};
