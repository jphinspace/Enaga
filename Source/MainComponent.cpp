/**
 * @file   MainComponent.cpp
 * @brief  MainComponent implementation.
 */

#include "MainComponent.h"

// ============================================================================
//  Constructor / Destructor
// ============================================================================

MainComponent::MainComponent(AudioToggleCallback       onToggle,
                             AudioFilterCallback       onFilter,
                             AudioGainCallback         onGain,
                             AudioNoiseTypeCallback    onNoiseType,
                             AudioLfoRateCallback      onLfoRate,
                             AudioLfoIntensityCallback onLfoIntensity,
                             AudioLfoModeCallback      onLfoMode)
    : audioToggle    (std::move(onToggle))
    , audioFilter    (std::move(onFilter))
    , audioGain      (std::move(onGain))
    , audioNoiseType (std::move(onNoiseType))
    , audioLfoRate   (std::move(onLfoRate))
    , audioLfoIntensity(std::move(onLfoIntensity))
    , audioLfoMode   (std::move(onLfoMode))
    , lfoComponent(
        [this](float r)    { if (audioLfoRate)      audioLfoRate(r);      },
        [this](float i)    { if (audioLfoIntensity) audioLfoIntensity(i); },
        [this](LfoMode m)  { if (audioLfoMode)      audioLfoMode(m);      })
{
    setupPlayButton();
    setupDiscreteSlider();
    setupContinuousSlider();
    setupValueBox();
    setupLabels();
    setupVolumeSlider();
    addAndMakeVisible(lfoComponent);

#if JUCE_IOS || JUCE_ANDROID
    setupMobileMenuButton();
#else
    menuBar.setModel(this);
    addAndMakeVisible(menuBar);
#endif
}

MainComponent::~MainComponent()
{
#if ! (JUCE_IOS || JUCE_ANDROID)
    menuBar.setModel(nullptr);
#endif
}

// ============================================================================
//  juce::MenuBarModel interface
// ============================================================================

juce::StringArray MainComponent::getMenuBarNames()
{
    return { "File" };
}

juce::PopupMenu MainComponent::getMenuForIndex(int /*topLevelMenuIndex*/,
                                               const juce::String& /*menuName*/)
{
    juce::PopupMenu menu;
    menu.addItem(1, "Save Preset");
    menu.addItem(2, "Load Preset");
#if ! (JUCE_IOS || JUCE_ANDROID)
    menu.addSeparator();
    menu.addItem(3, "Quit");
#endif
    return menu;
}

void MainComponent::menuItemSelected(int menuItemID, int /*topLevelMenuIndex*/)
{
    switch (menuItemID)
    {
        case 1: savePreset(); break;
        case 2: loadPreset(); break;
#if ! (JUCE_IOS || JUCE_ANDROID)
        case 3: juce::JUCEApplication::getInstance()->systemRequestedQuit(); break;
#endif
        default: break;
    }
}

// ============================================================================
//  juce::Component interface
// ============================================================================

void MainComponent::resized()
{
    auto area = getLocalBounds();
    constexpr int pad = 10;

#if ! (JUCE_IOS || JUCE_ANDROID)
    menuBar.setBounds(area.removeFromTop(menuBarHeight));
#endif

    area.reduce(pad, pad);

    // Image area: top 38% of remaining space
    const int imgH = juce::roundToInt(area.getHeight() * 0.38f);
    imageArea = area.removeFromTop(imgH);

#if JUCE_IOS || JUCE_ANDROID
    const int mBtnSz = juce::jlimit(28, 40, imgH - 8);
  #if JUCE_IOS
    mobileMenuButton.setBounds(imageArea.getRight() - mBtnSz - 4,
                               imageArea.getY() + 4, mBtnSz, mBtnSz);
  #else
    mobileMenuButton.setBounds(imageArea.getX() + 4,
                               imageArea.getY() + 4, mBtnSz, mBtnSz);
  #endif
#endif

    area.removeFromTop(pad);

    // Control row height: split remaining space equally across all rows.
    // 3 existing rows (play, cutoff, volume) + 3 LFO rows = 6 total.
    const int numRows = 6;
    const int ctrlH = (area.getHeight() - (numRows - 1) * pad) / numRows;

    // Row 1: play button + label + discrete slider
    auto row1 = area.removeFromTop(ctrlH);
    area.removeFromTop(pad);
    const int btnSz = juce::jlimit(32, 52, ctrlH - 4);
    playButton.setBounds(row1.removeFromLeft(btnSz + 4)
                             .withSizeKeepingCentre(btnSz, btnSz));
    row1.removeFromLeft(6);
    discreteLabel.setBounds(row1.removeFromLeft(64)
                                .withHeight(ctrlH).withY(row1.getY()));
    discreteSlider.setBounds(row1);

    // Row 2: Cutoff label + continuous slider + value box
    auto row2 = area.removeFromTop(ctrlH);
    continuousLabel.setBounds(row2.removeFromLeft(64));
    {
        const int boxW = juce::jlimit(56, 76, row2.getWidth() / 5);
        continuousValueBox.setBounds(row2.removeFromRight(boxW));
        row2.removeFromRight(4);
        continuousSlider.setBounds(row2);
    }

#if ! (JUCE_IOS || JUCE_ANDROID)
    area.removeFromTop(pad);

    // Row 3: Volume label + volume slider
    {
        auto row3 = area.removeFromTop(ctrlH);
        volumeLabel.setBounds(row3.removeFromLeft(64));
        volumeSlider.setBounds(row3);
    }
#else
    area.removeFromTop(pad);

    // Row 3: Volume label + platform-specific volume control
    {
        auto row3 = area.removeFromTop(ctrlH);
        volumeLabel.setBounds(row3.removeFromLeft(64));
  #if JUCE_IOS
        // MPVolumeView is self-contained (includes speaker icons); give it all
        // remaining row space so it can render at its natural size.
        mobileVolumeView.setBounds(row3);
  #else
        // Android: juce::Slider wired to system audio volume
        volumeSlider.setBounds(row3);
  #endif
    }
#endif

    area.removeFromTop(pad);

    // Rows 4–6: LFO section (mode button, rate slider, intensity slider + value box).
    lfoComponent.setBounds(area.removeFromTop(3 * ctrlH + 2 * pad));
}

void MainComponent::paint(juce::Graphics& g)
{
    g.fillAll(juce::Colour(0xff1a1a1a));
    paintImageArea(g);
}

// ============================================================================
//  Setup helpers
// ============================================================================

void MainComponent::setupPlayButton()
{
    playButton.onClick = [this]
    {
        if (audioToggle) audioToggle(playButton.getToggleState());
    };
    addAndMakeVisible(playButton);
}

void MainComponent::setupDiscreteSlider()
{
    discreteSlider.setSliderStyle(juce::Slider::LinearHorizontal);
    discreteSlider.setTextBoxStyle(juce::Slider::TextBoxLeft, false, 56, 20);
    discreteSlider.setRange(1.0, 4.0, 1.0);  // 4 noise types: White, Pink, Brown, Grey
    discreteSlider.setValue(defaultDiscreteValue);

    // Show the noise-type name instead of a raw number.
    discreteSlider.textFromValueFunction = [](double v) -> juce::String
    {
        switch (static_cast<int>(std::round(v)))
        {
            case 1:  return "White";
            case 2:  return "Pink";
            case 3:  return "Brown";
            case 4:  return "Grey";
            default: return {};
        }
    };
    discreteSlider.valueFromTextFunction = [](const juce::String& text) -> double
    {
        const auto lower = text.toLowerCase();
        if (lower == "pink")  return 2.0;
        if (lower == "brown") return 3.0;
        if (lower == "grey")  return 4.0;
        return 1.0; // default to White
    };

    discreteSlider.onValueChange = [this]
    {
        if (audioNoiseType)
            audioNoiseType(static_cast<float>(discreteSlider.getValue()));
    };
    addAndMakeVisible(discreteSlider);
}

void MainComponent::setupContinuousSlider()
{
    continuousSlider.setSliderStyle(juce::Slider::LinearHorizontal);
    continuousSlider.setTextBoxStyle(juce::Slider::NoTextBox, false, 0, 0);
    continuousSlider.setRange(0.0, 100.0);
    continuousSlider.setValue(defaultContinuousValue);
    continuousSlider.onValueChange = [this]
    {
        syncValueBox();
        if (audioFilter)
            audioFilter(static_cast<float>(continuousSlider.getValue()));
    };
    addAndMakeVisible(continuousSlider);
}

void MainComponent::setupValueBox()
{
    // Character restriction prevents non-numeric input; getDoubleValue() in
    // applyValueBox() stops at the first ambiguous character (e.g. a second
    // decimal point) so multi-point strings like "1.2.3" resolve to 1.2.
    continuousValueBox.setInputRestrictions(6, "0123456789.");
    continuousValueBox.setText(juce::String(defaultContinuousValue, 1), false);
    continuousValueBox.setJustification(juce::Justification::centred);
    continuousValueBox.onReturnKey = [this] { applyValueBox(); };
    continuousValueBox.onFocusLost = [this] { applyValueBox(); };
    addAndMakeVisible(continuousValueBox);
}

void MainComponent::setupLabels()
{
    discreteLabel.setText("Noise", juce::dontSendNotification);
    discreteLabel.setJustificationType(juce::Justification::centredLeft);
    addAndMakeVisible(discreteLabel);

    continuousLabel.setText("Cutoff", juce::dontSendNotification);
    continuousLabel.setJustificationType(juce::Justification::centredLeft);
    addAndMakeVisible(continuousLabel);

    volumeLabel.setText("Volume", juce::dontSendNotification);
    volumeLabel.setJustificationType(juce::Justification::centredLeft);
    addAndMakeVisible(volumeLabel);
}

void MainComponent::setupMobileMenuButton()
{
#if JUCE_IOS
    // UTF-8 encoding of U+2699 (⚙) followed by U+FE0F (variation selector)
    mobileMenuButton.setButtonText(
        juce::CharPointer_UTF8("\xe2\x9a\x99\xef\xb8\x8f"));
#elif JUCE_ANDROID
    // UTF-8 encoding of U+2630 (☰)
    mobileMenuButton.setButtonText(
        juce::CharPointer_UTF8("\xe2\x98\xb0"));
#endif
#if JUCE_IOS || JUCE_ANDROID
    mobileMenuButton.onClick = [this] { showMobileMenu(); };
    addAndMakeVisible(mobileMenuButton);
#endif
}

#if ! (JUCE_IOS || JUCE_ANDROID)
void MainComponent::setupVolumeSlider()
{
    volumeSlider.setSliderStyle(juce::Slider::LinearHorizontal);
    volumeSlider.setTextBoxStyle(juce::Slider::NoTextBox, false, 0, 0);
    volumeSlider.setRange(0.0, 100.0);
    volumeSlider.setValue(defaultVolumeValue);
    volumeSlider.onValueChange = [this]
    {
        if (audioGain)
            audioGain(static_cast<float>(volumeSlider.getValue()) / 100.0f);
    };
    addAndMakeVisible(volumeSlider);
}
#elif JUCE_IOS
void MainComponent::setupVolumeSlider()
{
    // mobileVolumeView wraps MPVolumeView; no extra configuration needed —
    // UIKit owns the appearance and the control directly manipulates device
    // media volume without any application-layer callbacks.
    addAndMakeVisible(mobileVolumeView);
}
#else // JUCE_ANDROID
void MainComponent::setupVolumeSlider()
{
    volumeSlider.setSliderStyle(juce::Slider::LinearHorizontal);
    volumeSlider.setTextBoxStyle(juce::Slider::NoTextBox, false, 0, 0);
    volumeSlider.setRange(0.0, 100.0);

    // Initialise from the current system media volume; clamp to [0, 1] in
    // case the API returns an unexpected value on some devices.
    const float sysGain = juce::jlimit(0.0f, 1.0f,
                                        juce::SystemAudioVolume::getGain());
    volumeSlider.setValue(static_cast<double>(sysGain) * 100.0,
                          juce::dontSendNotification);

    volumeSlider.onValueChange = [this]
    {
        juce::SystemAudioVolume::setGain(
            static_cast<float>(volumeSlider.getValue()) / 100.0f);
    };
    addAndMakeVisible(volumeSlider);
}
#endif

// ============================================================================
//  Painting
// ============================================================================

void MainComponent::paintImageArea(juce::Graphics& g)
{
    g.setColour(juce::Colour(0xff222222));
    g.fillRoundedRectangle(imageArea.toFloat(), 8.0f);

    // Decorative static-noise waveform.
    // Fixed seed ensures identical decorative pattern on every paint.
    static constexpr int   waveformSampleCount = 80;    // line segments
    static constexpr int   waveformSeed        = 12345; // fixed for visual consistency
    g.setColour(juce::Colour(0xff4fc3f7).withAlpha(0.45f));
    juce::Path wave;
    const float   step = imageArea.getWidth() / static_cast<float>(waveformSampleCount);
    const float   cy   = static_cast<float>(imageArea.getCentreY());
    const float   amp  = imageArea.getHeight() * 0.28f;
    juce::Random  r(waveformSeed);
    wave.startNewSubPath(static_cast<float>(imageArea.getX()), cy);
    for (int i = 1; i <= waveformSampleCount; ++i)
        wave.lineTo(imageArea.getX() + i * step,
                    cy + (r.nextFloat() * 2.0f - 1.0f) * amp);
    g.strokePath(wave, juce::PathStrokeType(1.5f));

    // App name
    g.setColour(juce::Colour(0xffe0e0e0).withAlpha(0.8f));
    g.setFont(juce::Font(juce::FontOptions{}.withHeight(24.0f).withStyle("Bold")));
    g.drawText("ENAGA", imageArea, juce::Justification::centred, false);
}

// ============================================================================
//  Value-box / slider synchronisation
// ============================================================================

void MainComponent::syncValueBox()
{
    continuousValueBox.setText(
        juce::String(continuousSlider.getValue(), 1), false);
}

void MainComponent::applyValueBox()
{
    const double v = juce::jlimit(0.0, 100.0,
                                  continuousValueBox.getText().getDoubleValue());
    continuousSlider.setValue(v, juce::sendNotificationSync);
    syncValueBox();
}

// ============================================================================
//  Preset I/O
// ============================================================================

void MainComponent::savePreset()
{
    fileChooser = std::make_unique<juce::FileChooser>(
        "Save Preset",
        juce::File::getSpecialLocation(juce::File::userDocumentsDirectory),
        "*.xml");

    constexpr int flags = juce::FileBrowserComponent::saveMode
                        | juce::FileBrowserComponent::canSelectFiles;

    fileChooser->launchAsync(flags, [this](const juce::FileChooser& fc)
    {
        auto file = fc.getResult();
        if (file == juce::File{}) return;

        if (file.getFileExtension().isEmpty())
            file = file.withFileExtension("xml");

        juce::XmlElement preset("EnagaPreset");
        preset.setAttribute("discreteValue",   discreteSlider.getValue());
        preset.setAttribute("continuousValue", continuousSlider.getValue());
        preset.setAttribute("isPlaying",
                            static_cast<int>(playButton.getToggleState()));
#if ! (JUCE_IOS || JUCE_ANDROID)
        preset.setAttribute("volumeValue", volumeSlider.getValue());
#endif

        if (! preset.writeTo(file))
            juce::Logger::writeToLog("Preset save failed: " + file.getFullPathName());
    });
}

void MainComponent::loadPreset()
{
    fileChooser = std::make_unique<juce::FileChooser>(
        "Load Preset",
        juce::File::getSpecialLocation(juce::File::userDocumentsDirectory),
        "*.xml");

    constexpr int flags = juce::FileBrowserComponent::openMode
                        | juce::FileBrowserComponent::canSelectFiles;

    fileChooser->launchAsync(flags, [this](const juce::FileChooser& fc)
    {
        const auto file = fc.getResult();
        if (file == juce::File{}) return;

        if (const auto xml = juce::XmlDocument::parse(file))
        {
            const double discVal = xml->getDoubleAttribute("discreteValue",
                                                           defaultDiscreteValue);
            const double contVal = xml->getDoubleAttribute("continuousValue",
                                                           defaultContinuousValue);
            const bool   playing = xml->getIntAttribute("isPlaying", 0) != 0;

            discreteSlider.setValue(discVal);
            continuousSlider.setValue(contVal);
            syncValueBox();

#if ! (JUCE_IOS || JUCE_ANDROID)
            const double volVal = xml->getDoubleAttribute("volumeValue",
                                                          defaultVolumeValue);
            volumeSlider.setValue(volVal);
#endif

            if (playing != playButton.getToggleState())
                playButton.setToggleState(playing, juce::sendNotification);
        }
    });
}

#if JUCE_IOS || JUCE_ANDROID
void MainComponent::showMobileMenu()
{
    juce::PopupMenu menu;
    menu.addItem(1, "Save Preset");
    menu.addItem(2, "Load Preset");
    menu.showMenuAsync(
        juce::PopupMenu::Options{}.withTargetComponent(&mobileMenuButton),
        [this](int id)
        {
            switch (id)
            {
                case 1: savePreset(); break;
                case 2: loadPreset(); break;
                default: break;
            }
        });
}
#endif
