/**
 * @file   LfoComponent.h
 * @brief  Self-contained UI component that exposes LFO controls to the user.
 *
 * Provides:
 *   - A mode button that cycles: Disabled → Volume → Filter → Both → Disabled.
 *   - A "LFO Rate" slider (0.01–2.0 Hz, skewed so 0.2 Hz is at mid-travel).
 *   - An "LFO Intensity" slider (0–100) paired with a numeric text box.
 *
 * All three callbacks fire on the message thread immediately when a control
 * changes value.
 */

#pragma once

#include "LfoEngine.h"

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
                 ModeCallback      onMode)
        : onRate      (std::move(onRate))
        , onIntensity (std::move(onIntensity))
        , onMode      (std::move(onMode))
    {
        setupModeButton();
        setupRateSlider();
        setupIntensitySlider();
        setupIntensityValueBox();
        setupLabels();
    }

    void resized() override
    {
        auto area = getLocalBounds();
        constexpr int pad    = 6;
        constexpr int labelW = 88;
        constexpr int boxW   = 64;

        const int rowH = (area.getHeight() - 2 * pad) / 3;

        // Row 1: mode button (full width).
        modeButton.setBounds(area.removeFromTop(rowH));
        area.removeFromTop(pad);

        // Row 2: rate label + rate slider.
        auto row2 = area.removeFromTop(rowH);
        area.removeFromTop(pad);
        rateLabel.setBounds(row2.removeFromLeft(labelW));
        rateSlider.setBounds(row2);

        // Row 3: intensity label + intensity slider + value box.
        auto row3 = area.removeFromTop(rowH);
        intensityLabel.setBounds(row3.removeFromLeft(labelW));
        intensityValueBox.setBounds(row3.removeFromRight(boxW));
        row3.removeFromRight(4);
        intensitySlider.setBounds(row3);
    }

private:
    // -----------------------------------------------------------------------
    //  Setup helpers
    // -----------------------------------------------------------------------

    void setupModeButton()
    {
        modeButton.setButtonText("LFO: Disabled");
        modeButton.onClick = [this]
        {
            // Cycle: Disabled → Volume → Filter → Both → Disabled
            switch (currentMode)
            {
                case LfoMode::Disabled: currentMode = LfoMode::Volume;   break;
                case LfoMode::Volume:   currentMode = LfoMode::Filter;   break;
                case LfoMode::Filter:   currentMode = LfoMode::Both;     break;
                case LfoMode::Both:     currentMode = LfoMode::Disabled; break;
            }
            updateModeButtonText();
            if (onMode) onMode(currentMode);
        };
        addAndMakeVisible(modeButton);
    }

    void setupRateSlider()
    {
        rateSlider.setSliderStyle(juce::Slider::LinearHorizontal);
        rateSlider.setTextBoxStyle(juce::Slider::NoTextBox, false, 0, 0);
        rateSlider.setRange(0.01, 2.0);
        // Skew so that 0.2 Hz (a typical ocean-wave rate) sits at mid-travel.
        rateSlider.setSkewFactorFromMidPoint(0.2);
        rateSlider.setValue(0.1, juce::dontSendNotification);
        rateSlider.onValueChange = [this]
        {
            if (onRate)
                onRate(static_cast<float>(rateSlider.getValue()));
        };
        addAndMakeVisible(rateSlider);
    }

    void setupIntensitySlider()
    {
        intensitySlider.setSliderStyle(juce::Slider::LinearHorizontal);
        intensitySlider.setTextBoxStyle(juce::Slider::NoTextBox, false, 0, 0);
        intensitySlider.setRange(0.0, 100.0);
        intensitySlider.setValue(0.0, juce::dontSendNotification);
        intensitySlider.onValueChange = [this]
        {
            syncIntensityValueBox();
            if (onIntensity)
                onIntensity(static_cast<float>(intensitySlider.getValue()));
        };
        addAndMakeVisible(intensitySlider);
    }

    void setupIntensityValueBox()
    {
        // Allow only numeric input; getDoubleValue() clamps on ambiguous text.
        intensityValueBox.setInputRestrictions(6, "0123456789.");
        intensityValueBox.setText("0.0", false);
        intensityValueBox.setJustification(juce::Justification::centred);
        intensityValueBox.onReturnKey = [this] { applyIntensityValueBox(); };
        intensityValueBox.onFocusLost = [this] { applyIntensityValueBox(); };
        addAndMakeVisible(intensityValueBox);
    }

    void setupLabels()
    {
        rateLabel.setText("LFO Rate", juce::dontSendNotification);
        rateLabel.setJustificationType(juce::Justification::centredLeft);
        addAndMakeVisible(rateLabel);

        intensityLabel.setText("LFO Intensity", juce::dontSendNotification);
        intensityLabel.setJustificationType(juce::Justification::centredLeft);
        addAndMakeVisible(intensityLabel);
    }

    // -----------------------------------------------------------------------
    //  Mode button
    // -----------------------------------------------------------------------

    void updateModeButtonText()
    {
        const char* text = "LFO: Disabled";
        switch (currentMode)
        {
            case LfoMode::Volume:  text = "LFO: Volume";   break;
            case LfoMode::Filter:  text = "LFO: Filter";   break;
            case LfoMode::Both:    text = "LFO: Both";     break;
            default:                                        break;
        }
        modeButton.setButtonText(text);
    }

    // -----------------------------------------------------------------------
    //  Intensity value-box / slider synchronisation
    // -----------------------------------------------------------------------

    void syncIntensityValueBox()
    {
        intensityValueBox.setText(
            juce::String(intensitySlider.getValue(), 1), false);
    }

    void applyIntensityValueBox()
    {
        const double v = juce::jlimit(0.0, 100.0,
                                      intensityValueBox.getText().getDoubleValue());
        intensitySlider.setValue(v, juce::sendNotificationSync);
        syncIntensityValueBox();
    }

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
    juce::Label      intensityLabel;
    juce::Slider     intensitySlider;
    juce::TextEditor intensityValueBox;
};
