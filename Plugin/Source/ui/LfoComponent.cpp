/**
 * @file   LfoComponent.cpp
 * @brief  LfoComponent implementation.
 */

#include "ui/LfoComponent.h"

LfoComponent::LfoComponent(RateCallback      onRate,
                           IntensityCallback onIntensity,
                           ModeCallback      onMode)
    : onRate      (std::move(onRate))
    , onIntensity (std::move(onIntensity))
    , onMode      (std::move(onMode))
{
    setupModeButton();
    setupRateSlider();
    setupRateValueBox();
    setupIntensitySlider();
    setupIntensityValueBox();
    setupLabels();
}

void LfoComponent::resized()
{
    auto area = getLocalBounds();
    constexpr int pad    = 6;
    constexpr int labelW = 88;
    constexpr int boxW   = 64;

    const int rowH = (area.getHeight() - 2 * pad) / 3;

    // Row 1: mode button (full width).
    modeButton.setBounds(area.removeFromTop(rowH));
    area.removeFromTop(pad);

    // Row 2: rate label + rate slider + rate value box.
    auto row2 = area.removeFromTop(rowH);
    area.removeFromTop(pad);
    rateLabel.setBounds(row2.removeFromLeft(labelW));
    rateValueBox.setBounds(row2.removeFromRight(boxW));
    row2.removeFromRight(4);
    rateSlider.setBounds(row2);

    // Row 3: intensity label + intensity slider + value box.
    auto row3 = area.removeFromTop(rowH);
    intensityLabel.setBounds(row3.removeFromLeft(labelW));
    intensityValueBox.setBounds(row3.removeFromRight(boxW));
    row3.removeFromRight(4);
    intensitySlider.setBounds(row3);
}

void LfoComponent::setupModeButton()
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

void LfoComponent::setupRateSlider()
{
    rateSlider.setSliderStyle(juce::Slider::LinearHorizontal);
    rateSlider.setTextBoxStyle(juce::Slider::NoTextBox, false, 0, 0);
    rateSlider.setRange(0.01, 2.0);
    // Skew so that 0.2 Hz (a typical ocean-wave rate) sits at mid-travel.
    rateSlider.setSkewFactorFromMidPoint(0.2);
    rateSlider.setValue(0.1, juce::dontSendNotification);
    rateSlider.onValueChange = [this]
    {
        syncRateValueBox();
        if (onRate)
            onRate(static_cast<float>(rateSlider.getValue()));
    };
    addAndMakeVisible(rateSlider);
}

void LfoComponent::setupRateValueBox()
{
    // Allow only numeric input; getDoubleValue() clamps on ambiguous text.
    rateValueBox.setInputRestrictions(6, "0123456789.");
    rateValueBox.setText(juce::String(rateSlider.getValue(), 2), false);
    rateValueBox.setJustification(juce::Justification::centred);
    rateValueBox.onReturnKey = [this] { applyRateValueBox(); };
    rateValueBox.onFocusLost = [this] { applyRateValueBox(); };
    addAndMakeVisible(rateValueBox);
}

void LfoComponent::setupIntensitySlider()
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

void LfoComponent::setupIntensityValueBox()
{
    // Allow only numeric input; getDoubleValue() clamps on ambiguous text.
    intensityValueBox.setInputRestrictions(6, "0123456789.");
    intensityValueBox.setText("0.0", false);
    intensityValueBox.setJustification(juce::Justification::centred);
    intensityValueBox.onReturnKey = [this] { applyIntensityValueBox(); };
    intensityValueBox.onFocusLost = [this] { applyIntensityValueBox(); };
    addAndMakeVisible(intensityValueBox);
}

void LfoComponent::setupLabels()
{
    rateLabel.setText("LFO Rate", juce::dontSendNotification);
    rateLabel.setJustificationType(juce::Justification::centredLeft);
    addAndMakeVisible(rateLabel);

    intensityLabel.setText("LFO Intensity", juce::dontSendNotification);
    intensityLabel.setJustificationType(juce::Justification::centredLeft);
    addAndMakeVisible(intensityLabel);
}

void LfoComponent::updateModeButtonText()
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

void LfoComponent::syncRateValueBox()
{
    rateValueBox.setText(
        juce::String(rateSlider.getValue(), 2), false);
}

void LfoComponent::applyRateValueBox()
{
    const double v = juce::jlimit(0.01, 2.0,
                                  rateValueBox.getText().getDoubleValue());
    rateSlider.setValue(v, juce::sendNotificationSync);
    syncRateValueBox();
}

void LfoComponent::syncIntensityValueBox()
{
    intensityValueBox.setText(
        juce::String(intensitySlider.getValue(), 1), false);
}

void LfoComponent::applyIntensityValueBox()
{
    const double v = juce::jlimit(0.0, 100.0,
                                  intensityValueBox.getText().getDoubleValue());
    intensitySlider.setValue(v, juce::sendNotificationSync);
    syncIntensityValueBox();
}
