/**
 * @file   main_component.cpp
 * @brief  MainComponent implementation.
 */

#include "ui/main_component.h"

#include "ui/enaga_look_and_feel.h"

// ============================================================================
//  Constructor / Destructor
// ============================================================================

MainComponent::MainComponent(AudioCallbacks cbs)
    : callbacks_(std::move(cbs))
    , lfo_component_(
        [this](float r)   {
          if (callbacks_.onLfoRate) callbacks_.onLfoRate(r);
        },
        [this](float i)   {
          if (callbacks_.onLfoIntensity) callbacks_.onLfoIntensity(i);
        },
        [this](LfoMode m) {
          if (callbacks_.onLfoMode) callbacks_.onLfoMode(m);
        }) {
  SetupPlayButton();
  SetupDiscreteSlider();
  SetupContinuousSlider();
  SetupValueBox();
  SetupLabels();
  SetupVolumeSlider();
  addAndMakeVisible(lfo_component_);

#if JUCE_IOS || JUCE_ANDROID
  SetupMobileMenuButton();
#else
  menu_bar_.setModel(this);
  addAndMakeVisible(menu_bar_);
#endif
}

MainComponent::~MainComponent() {
#if ! (JUCE_IOS || JUCE_ANDROID)
  menu_bar_.setModel(nullptr);
#endif
}

// ============================================================================
//  juce::MenuBarModel interface
// ============================================================================

juce::StringArray MainComponent::getMenuBarNames() {
  return { "File" };
}

juce::PopupMenu MainComponent::getMenuForIndex(
    int /*top_level_menu_index*/,
    const juce::String& /*menu_name*/) {
  juce::PopupMenu menu;
  menu.addItem(1, "Save Preset");
  menu.addItem(2, "Load Preset");
#if ! (JUCE_IOS || JUCE_ANDROID)
  menu.addSeparator();
  menu.addItem(3, "Quit");
#endif
  return menu;
}

void MainComponent::menuItemSelected(int menu_item_id,
                                     int /*top_level_menu_index*/) {
  switch (menu_item_id) {
    case 1: SavePreset(); break;
    case 2: LoadPreset(); break;
#if ! (JUCE_IOS || JUCE_ANDROID)
    case 3:
      juce::JUCEApplication::getInstance()->systemRequestedQuit();
      break;
#endif
    default: break;
  }
}

// ============================================================================
//  juce::Component interface
// ============================================================================

void MainComponent::resized() {
  auto area = getLocalBounds();
  constexpr int kPad = 10;

#if ! (JUCE_IOS || JUCE_ANDROID)
  menu_bar_.setBounds(area.removeFromTop(kMenuBarHeight));
#endif

  area.reduce(kPad, kPad);

  // Image area: top 38% of remaining space
  const int img_h = juce::roundToInt(area.getHeight() * 0.38f);
  image_area_ = area.removeFromTop(img_h);

#if JUCE_IOS || JUCE_ANDROID
  const int m_btn_sz = juce::jlimit(28, 40, img_h - 8);
  #if JUCE_IOS
  mobile_menu_button_.setBounds(image_area_.getRight() - m_btn_sz - 4,
                                image_area_.getY() + 4,
                                m_btn_sz, m_btn_sz);
  #else
  mobile_menu_button_.setBounds(image_area_.getX() + 4,
                                image_area_.getY() + 4,
                                m_btn_sz, m_btn_sz);
  #endif
#endif

  area.removeFromTop(kPad);

  // Control row height: split remaining space equally across all rows.
  // 3 existing rows (play, cutoff, volume) + 3 LFO rows = 6 total.
  const int num_rows = 6;
  const int ctrl_h =
      (area.getHeight() - (num_rows - 1) * kPad) / num_rows;

  // Row 1: play button + label + discrete slider
  auto row1 = area.removeFromTop(ctrl_h);
  area.removeFromTop(kPad);
  const int btn_sz = juce::jlimit(32, 52, ctrl_h - 4);
  play_button_.setBounds(row1.removeFromLeft(btn_sz + 4)
                             .withSizeKeepingCentre(btn_sz, btn_sz));
  row1.removeFromLeft(6);
  discrete_label_.setBounds(row1.removeFromLeft(64)
                                .withHeight(ctrl_h).withY(row1.getY()));
  discrete_slider_.setBounds(row1);

  // Row 2: Cutoff label + continuous slider + value box
  auto row2 = area.removeFromTop(ctrl_h);
  continuous_label_.setBounds(row2.removeFromLeft(64));
  {
    constexpr int kBoxW = 64;
    continuous_value_box_.setBounds(row2.removeFromRight(kBoxW));
    row2.removeFromRight(4);
    continuous_slider_.setBounds(row2);
  }

#if ! (JUCE_IOS || JUCE_ANDROID)
  area.removeFromTop(kPad);

  // Row 3: Volume label + volume slider
  {
    auto row3 = area.removeFromTop(ctrl_h);
    volume_label_.setBounds(row3.removeFromLeft(64));
    volume_slider_.setBounds(row3);
  }
#else
  area.removeFromTop(kPad);

  // Row 3: Volume label + platform-specific volume control
  {
    auto row3 = area.removeFromTop(ctrl_h);
    volume_label_.setBounds(row3.removeFromLeft(64));
  #if JUCE_IOS
    // MPVolumeView is self-contained; give it all remaining row space.
    mobile_volume_view_.setBounds(row3);
  #else
    // Android: juce::Slider wired to system audio volume
    volume_slider_.setBounds(row3);
  #endif
  }
#endif

  area.removeFromTop(kPad);

  // Rows 4–6: LFO section (mode button, rate, intensity).
  lfo_component_.setBounds(
      area.removeFromTop(3 * ctrl_h + 2 * kPad));
}

void MainComponent::paint(juce::Graphics& g) {
  g.fillAll(juce::Colour(EnagaLookAndFeel::kBackground));
  PaintImageArea(g);
}

// ============================================================================
//  Setup helpers
// ============================================================================

void MainComponent::SetupPlayButton() {
  play_button_.onClick = [this] {
    if (callbacks_.onToggle)
      callbacks_.onToggle(play_button_.getToggleState());
  };
  addAndMakeVisible(play_button_);
}

void MainComponent::SetupDiscreteSlider() {
  discrete_slider_.setSliderStyle(juce::Slider::LinearHorizontal);
  discrete_slider_.setTextBoxStyle(
      juce::Slider::TextBoxLeft, false, 56, 20);
  discrete_slider_.setRange(1.0, 4.0, 1.0);
  discrete_slider_.setValue(kDefaultDiscreteValue);

  // Show the noise-type name instead of a raw number.
  discrete_slider_.textFromValueFunction = [](double v) -> juce::String {
    switch (static_cast<int>(std::round(v))) {
      case 1:  return "White";
      case 2:  return "Pink";
      case 3:  return "Brown";
      case 4:  return "Grey";
      default: return {};
    }
  };
  discrete_slider_.valueFromTextFunction =
      [](const juce::String& text) -> double {
    const auto lower = text.toLowerCase();
    if (lower == "pink")  return 2.0;
    if (lower == "brown") return 3.0;
    if (lower == "grey")  return 4.0;
    return 1.0;  // default to White
  };

  discrete_slider_.onValueChange = [this] {
    if (callbacks_.onNoiseType)
      callbacks_.onNoiseType(
          static_cast<float>(discrete_slider_.getValue()));
  };
  addAndMakeVisible(discrete_slider_);
}

void MainComponent::SetupContinuousSlider() {
  continuous_slider_.setSliderStyle(juce::Slider::LinearHorizontal);
  continuous_slider_.setTextBoxStyle(
      juce::Slider::NoTextBox, false, 0, 0);
  continuous_slider_.setRange(0.0, 100.0);
  continuous_slider_.setValue(kDefaultContinuousValue);
  continuous_slider_.onValueChange = [this] {
    SyncValueBox();
    if (callbacks_.onFilter)
      callbacks_.onFilter(
          static_cast<float>(continuous_slider_.getValue()));
  };
  addAndMakeVisible(continuous_slider_);
}

void MainComponent::SetupValueBox() {
  // Character restriction prevents non-numeric input.
  continuous_value_box_.setInputRestrictions(6, "0123456789.");
  continuous_value_box_.setText(
      juce::String(kDefaultContinuousValue, 1), false);
  continuous_value_box_.setJustification(
      juce::Justification::centred);
  continuous_value_box_.onReturnKey = [this] { ApplyValueBox(); };
  continuous_value_box_.onFocusLost = [this] { ApplyValueBox(); };
  addAndMakeVisible(continuous_value_box_);
}

void MainComponent::SetupLabels() {
  discrete_label_.setText("Noise", juce::dontSendNotification);
  discrete_label_.setJustificationType(
      juce::Justification::centredLeft);
  addAndMakeVisible(discrete_label_);

  continuous_label_.setText("Cutoff", juce::dontSendNotification);
  continuous_label_.setJustificationType(
      juce::Justification::centredLeft);
  addAndMakeVisible(continuous_label_);

  volume_label_.setText("Volume", juce::dontSendNotification);
  volume_label_.setJustificationType(
      juce::Justification::centredLeft);
  addAndMakeVisible(volume_label_);
}

void MainComponent::SetupMobileMenuButton() {
#if JUCE_IOS
  // UTF-8 encoding of U+2699 (⚙) followed by U+FE0F (variation selector)
  mobile_menu_button_.setButtonText(
      juce::CharPointer_UTF8("\xe2\x9a\x99\xef\xb8\x8f"));
#elif JUCE_ANDROID
  // UTF-8 encoding of U+2630 (☰)
  mobile_menu_button_.setButtonText(
      juce::CharPointer_UTF8("\xe2\x98\xb0"));
#endif
#if JUCE_IOS || JUCE_ANDROID
  mobile_menu_button_.onClick = [this] { ShowMobileMenu(); };
  addAndMakeVisible(mobile_menu_button_);
#endif
}

#if ! (JUCE_IOS || JUCE_ANDROID)
void MainComponent::SetupVolumeSlider() {
  volume_slider_.setSliderStyle(juce::Slider::LinearHorizontal);
  volume_slider_.setTextBoxStyle(
      juce::Slider::NoTextBox, false, 0, 0);
  volume_slider_.setRange(0.0, 100.0);
  volume_slider_.setValue(kDefaultVolumeValue);
  volume_slider_.onValueChange = [this] {
    if (callbacks_.onGain)
      callbacks_.onGain(
          static_cast<float>(volume_slider_.getValue()) / 100.0f);
  };
  addAndMakeVisible(volume_slider_);
}
#elif JUCE_IOS
void MainComponent::SetupVolumeSlider() {
  // mobile_volume_view_ wraps MPVolumeView; no extra configuration needed
  // — UIKit owns the appearance and directly manipulates device media
  // volume without any application-layer callbacks.
  addAndMakeVisible(mobile_volume_view_);
}
#else  // JUCE_ANDROID
void MainComponent::SetupVolumeSlider() {
  volume_slider_.setSliderStyle(juce::Slider::LinearHorizontal);
  volume_slider_.setTextBoxStyle(
      juce::Slider::NoTextBox, false, 0, 0);
  volume_slider_.setRange(0.0, 100.0);

  // Initialise from the current system media volume.
  const float sys_gain = juce::jlimit(
      0.0f, 1.0f, juce::SystemAudioVolume::getGain());
  volume_slider_.setValue(static_cast<double>(sys_gain) * 100.0,
                          juce::dontSendNotification);

  volume_slider_.onValueChange = [this] {
    juce::SystemAudioVolume::setGain(
        static_cast<float>(volume_slider_.getValue()) / 100.0f);
  };
  addAndMakeVisible(volume_slider_);
}
#endif

// ============================================================================
//  Painting
// ============================================================================

void MainComponent::PaintImageArea(juce::Graphics& g) {
  g.setColour(juce::Colour(EnagaLookAndFeel::kImageArea));
  g.fillRoundedRectangle(image_area_.toFloat(), 8.0f);

  // Decorative static-noise waveform.
  // Fixed seed ensures identical decorative pattern on every paint.
  static constexpr int kWaveformSampleCount = 80;
  static constexpr int kWaveformSeed        = 12345;
  g.setColour(juce::Colour(EnagaLookAndFeel::kAccent).withAlpha(0.45f));
  juce::Path wave;
  const float step =
      image_area_.getWidth() /
      static_cast<float>(kWaveformSampleCount);
  const float cy  = static_cast<float>(image_area_.getCentreY());
  const float amp = image_area_.getHeight() * 0.28f;
  juce::Random rng(kWaveformSeed);
  wave.startNewSubPath(static_cast<float>(image_area_.getX()), cy);
  for (int i = 1; i <= kWaveformSampleCount; ++i)
    wave.lineTo(image_area_.getX() + i * step,
                cy + (rng.nextFloat() * 2.0f - 1.0f) * amp);
  g.strokePath(wave, juce::PathStrokeType(1.5f));

  // App name
  g.setColour(
      juce::Colour(EnagaLookAndFeel::kText).withAlpha(0.8f));
  g.setFont(juce::Font(
      juce::FontOptions{}.withHeight(24.0f).withStyle("Bold")));
  g.drawText("ENAGA", image_area_, juce::Justification::centred,
             false);
}

// ============================================================================
//  Value-box / slider synchronisation
// ============================================================================

void MainComponent::SyncValueBox() {
  continuous_value_box_.setText(
      juce::String(continuous_slider_.getValue(), 1), false);
}

void MainComponent::ApplyValueBox() {
  const double v = juce::jlimit(
      0.0, 100.0,
      continuous_value_box_.getText().getDoubleValue());
  continuous_slider_.setValue(v, juce::sendNotificationSync);
  SyncValueBox();
}

// ============================================================================
//  Preset I/O
// ============================================================================

void MainComponent::SavePreset() {
  file_chooser_ = std::make_unique<juce::FileChooser>(
      "Save Preset",
      juce::File::getSpecialLocation(
          juce::File::userDocumentsDirectory),
      "*.xml");

  constexpr int flags = juce::FileBrowserComponent::saveMode
                      | juce::FileBrowserComponent::canSelectFiles;

  file_chooser_->launchAsync(flags,
      [this](const juce::FileChooser& fc) {
        auto file = fc.getResult();
        if (file == juce::File{}) return;

        if (file.getFileExtension().isEmpty())
          file = file.withFileExtension("xml");

        juce::XmlElement preset("EnagaPreset");
        preset.setAttribute("discreteValue",
                            discrete_slider_.getValue());
        preset.setAttribute("continuousValue",
                            continuous_slider_.getValue());
        preset.setAttribute(
            "isPlaying",
            static_cast<int>(play_button_.getToggleState()));
#if ! (JUCE_IOS || JUCE_ANDROID)
        preset.setAttribute("volumeValue",
                            volume_slider_.getValue());
#endif

        if (! preset.writeTo(file))
          juce::Logger::writeToLog(
              "Preset save failed: " + file.getFullPathName());
      });
}

void MainComponent::LoadPreset() {
  file_chooser_ = std::make_unique<juce::FileChooser>(
      "Load Preset",
      juce::File::getSpecialLocation(
          juce::File::userDocumentsDirectory),
      "*.xml");

  constexpr int flags = juce::FileBrowserComponent::openMode
                      | juce::FileBrowserComponent::canSelectFiles;

  file_chooser_->launchAsync(flags,
      [this](const juce::FileChooser& fc) {
        const auto file = fc.getResult();
        if (file == juce::File{}) return;

        if (const auto xml = juce::XmlDocument::parse(file)) {
          const double disc_val = xml->getDoubleAttribute(
              "discreteValue", kDefaultDiscreteValue);
          const double cont_val = xml->getDoubleAttribute(
              "continuousValue", kDefaultContinuousValue);
          const bool playing =
              xml->getIntAttribute("isPlaying", 0) != 0;

          discrete_slider_.setValue(disc_val);
          continuous_slider_.setValue(cont_val);
          SyncValueBox();

#if ! (JUCE_IOS || JUCE_ANDROID)
          const double vol_val = xml->getDoubleAttribute(
              "volumeValue", kDefaultVolumeValue);
          volume_slider_.setValue(vol_val);
#endif

          if (playing != play_button_.getToggleState())
            play_button_.setToggleState(
                playing, juce::sendNotification);
        }
      });
}

#if JUCE_IOS || JUCE_ANDROID
void MainComponent::ShowMobileMenu() {
  juce::PopupMenu menu;
  menu.addItem(1, "Save Preset");
  menu.addItem(2, "Load Preset");
  menu.showMenuAsync(
      juce::PopupMenu::Options{}.withTargetComponent(
          &mobile_menu_button_),
      [this](int id) {
        switch (id) {
          case 1: SavePreset(); break;
          case 2: LoadPreset(); break;
          default: break;
        }
      });
}
#endif
