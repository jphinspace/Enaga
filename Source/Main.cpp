/**
 * @file   Main.cpp
 * @brief  Enaga – Relaxing White Noise Generator
 *
 * Generates white noise via JUCE's audio-device API while the application is
 * running.  Audio stops when the user presses the on/off button and resumes
 * when they press it again.  The continuous Cutoff slider controls the cutoff
 * frequency of a low-pass filter (0–100 → 20 Hz–20 kHz, log scale).  On
 * desktop a Volume slider provides application-level gain control; on iOS and
 * Android the system media volume (hardware buttons) is used instead.
 *
 * Built with C++23.  All areas that benefit from C++26 features are marked
 * with a "TODO:C++26" comment so the eventual migration is straightforward.
 *
 * Supported platforms (via JUCE):
 *   - macOS, Windows, Linux  (desktop)
 *   - iOS, Android           (mobile – handled by JUCE's platform glue)
 *
 * Compile-time prerequisites:
 *   cmake --preset <platform>    (see CMakePresets.json)
 */

// TODO:C++26  Replace the #include directives below with named module imports
//             once JUCE and the standard library ship module interfaces:
//
//   import std;               // replaces <iostream>
//   import juce;              // replaces juce_* module headers

#include <juce_audio_devices/juce_audio_devices.h>  // AudioDeviceManager, AudioSourcePlayer
#include <juce_gui_basics/juce_gui_basics.h>         // JUCEApplication, DocumentWindow
#include <juce_core/juce_core.h>                     // String, Logger, Random

// ============================================================================
//  EnagaLookAndFeel  –  application-wide dark theme
// ============================================================================

/**
 * Custom dark-mode look and feel applied globally to all Enaga controls.
 * Near-black background with a light-blue accent and high-contrast text.
 */
class EnagaLookAndFeel final : public juce::LookAndFeel_V4
{
public:
    EnagaLookAndFeel()
    {
        const auto bg      = juce::Colour(0xff1a1a1a);  // main background
        const auto panel   = juce::Colour(0xff2d2d2d);  // control backgrounds
        const auto accent  = juce::Colour(0xff4fc3f7);  // highlight / thumb
        const auto textCol = juce::Colour(0xffe0e0e0);  // primary text

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

        const auto tickColour = juce::Colour(0xff4fc3f7).withAlpha(0.55f);
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

// ============================================================================
//  WhiteNoiseAudioSource
// ============================================================================

/**
 * Produces full-scale white noise routed through a first-order Butterworth
 * low-pass IIR filter, then scaled by a gain factor.
 *
 * Both the cutoff (0–100 normalised scale → 20 Hz–20 kHz log) and gain
 * (0–1) are written by the UI thread via lock-free atomics and read on the
 * real-time audio thread using relaxed ordering.  Coefficient updates happen
 * at the start of the first audio block after each cutoff change.
 *
 * On desktop the gain is driven by the Volume slider; on iOS/Android the
 * atomic is left at 1.0f (full scale) and loudness is handled by the OS
 * media volume.
 */
class WhiteNoiseAudioSource final : public juce::AudioSource
{
public:
    /**
     * Set the low-pass filter cutoff on a normalised 0–100 scale.
     * 0 → 20 Hz (heavy filtering), 100 → 20 kHz (wide open, near full spectrum).
     * Thread-safe: called from the message thread, applied on the audio thread.
     */
    void setCutoff(float normalised0to100) noexcept
    {
        cutoff.store(juce::jlimit(0.0f, 100.0f, normalised0to100),
                     std::memory_order_relaxed);
    }

    /**
     * Set output amplitude multiplier in [0, 1].
     * Thread-safe: called from the message thread, read on the audio thread.
     */
    void setGain(float newGain) noexcept
    {
        gain.store(juce::jlimit(0.0f, 1.0f, newGain), std::memory_order_relaxed);
    }

    void prepareToPlay(int /*samplesPerBlockExpected*/, double newSampleRate) override
    {
        sampleRate = newSampleRate;
        lastCutoff = cutoff.load(std::memory_order_relaxed);
        updateFilters();
        for (auto& f : filters)
            f.reset();
    }

    void releaseResources() override
    {
        for (auto& f : filters)
            f.reset();
    }

    void getNextAudioBlock(const juce::AudioSourceChannelInfo& info) override
    {
        // Re-coefficient the filters if the cutoff changed since the last block.
        const float c = cutoff.load(std::memory_order_relaxed);
        if (c != lastCutoff)
        {
            lastCutoff = c;
            updateFilters();
        }

        const float g = gain.load(std::memory_order_relaxed);
        for (int ch = 0; ch < info.buffer->getNumChannels(); ++ch)
        {
            auto* out = info.buffer->getWritePointer(ch, info.startSample);
            for (int i = 0; i < info.numSamples; ++i)
                out[i] = random.nextFloat() * 2.0f - 1.0f;

            if (ch < static_cast<int>(filters.size()))
                filters[static_cast<size_t>(ch)].processSamples(out, info.numSamples);

            juce::FloatVectorOperations::multiply(out, g, info.numSamples);
        }
    }

private:
    /**
     * Recalculate IIR coefficients from the current @c lastCutoff value.
     * Must only be called on the audio thread (or before streaming starts).
     */
    void updateFilters()
    {
        if (sampleRate <= 0.0) return;

        // Logarithmic map: 0 → 20 Hz, 100 → 20 000 Hz.
        const double freq = 20.0 * std::pow(1000.0, static_cast<double>(lastCutoff) / 100.0);
        const auto   coef = juce::IIRCoefficients::makeLowPass(sampleRate, freq);
        for (auto& f : filters)
            f.setCoefficients(coef);
    }

    juce::Random            random;              // PRNG seeded from system entropy
    std::atomic<float>      cutoff { 100.0f };   // normalised 0–100; written by UI thread
    float                   lastCutoff { 100.0f }; // applied value (audio thread only)
    std::atomic<float>      gain   { 1.0f };     // amplitude multiplier [0,1]; written by UI thread, read on audio thread
    double                  sampleRate { 44100.0 };
    std::array<juce::IIRFilter, 2> filters;      // one per stereo channel
};

// ============================================================================
//  PlayButton
// ============================================================================

/**
 * Custom toggle button rendered as:
 *   - A green right-pointing triangle when toggle state is false (noise off).
 *   - A red rounded square when toggle state is true  (noise on).
 *
 * No text is displayed.
 */
class PlayButton final : public juce::Button
{
public:
    PlayButton() : juce::Button("play")
    {
        setClickingTogglesState(true);
    }

    void paintButton(juce::Graphics& g,
                     bool isMouseOver,
                     bool /*isButtonDown*/) override
    {
        const auto  bounds = getLocalBounds().toFloat().reduced(2.0f);
        const float dim    = juce::jmin(bounds.getWidth(), bounds.getHeight());
        const auto  sq     = juce::Rectangle<float>(dim, dim).withCentre(bounds.getCentre());
        const float alpha  = isMouseOver ? 0.80f : 1.0f;

        if (getToggleState())
        {
            // ON / playing → red rounded square (stop)
            g.setColour(juce::Colour(0xffe53935).withAlpha(alpha));
            g.fillRoundedRectangle(sq.reduced(dim * 0.20f), 5.0f);
        }
        else
        {
            // OFF / stopped → green triangle (play)
            g.setColour(juce::Colour(0xff43a047).withAlpha(alpha));
            juce::Path tri;
            const auto r = sq.reduced(dim * 0.10f);
            tri.addTriangle(r.getTopLeft(), r.getBottomLeft(),
                            { r.getRight(), r.getCentreY() });
            g.fillPath(tri);
        }
    }
};

// ============================================================================
//  MainComponent
// ============================================================================

/**
 * Root UI component that fills the application window.
 *
 * Layout (top to bottom):
 *   1. Menu bar (desktop) or image area with overlay button (mobile).
 *   2. Image / branding area with decorative waveform.
 *   3. Play button  +  discrete (stepped) slider with label.
 *   4. "Cutoff" label  +  continuous slider  +  numeric text box (0.0–100.0).
 *   5. "Volume" label  +  volume slider  +  numeric text box  (desktop only).
 *      On iOS/Android perceived loudness is controlled by the system media
 *      volume (hardware buttons), so no application-side slider is shown.
 *
 * Platform-specific navigation:
 *   - Desktop (macOS/Windows/Linux) : juce::MenuBarComponent at the top.
 *   - iOS                           : Gear icon button (⚙) top-right of image.
 *   - Android                       : Hamburger button (☰) top-left of image.
 *
 * All menu options are available on every platform; "Quit" is added only on
 * desktop builds.
 */
class MainComponent final : public juce::Component
                          , public juce::MenuBarModel
{
public:
    using AudioToggleCallback  = std::function<void(bool /*shouldPlay*/)>;
    using AudioFilterCallback  = std::function<void(float /*cutoff 0-100*/)>;
    using AudioGainCallback    = std::function<void(float /*gain 0-1*/)>;

    MainComponent(AudioToggleCallback onToggle,
                  AudioFilterCallback onFilter,
                  AudioGainCallback   onGain)
        : audioToggle(std::move(onToggle))
        , audioFilter(std::move(onFilter))
        , audioGain(std::move(onGain))
    {
        setupPlayButton();
        setupDiscreteSlider();
        setupContinuousSlider();
        setupValueBox();
        setupLabels();
#if ! (JUCE_IOS || JUCE_ANDROID)
        setupVolumeSlider();
#endif

#if JUCE_IOS || JUCE_ANDROID
        setupMobileMenuButton();
#else
        menuBar.setModel(this);
        addAndMakeVisible(menuBar);
#endif
    }

    ~MainComponent() override
    {
#if ! (JUCE_IOS || JUCE_ANDROID)
        menuBar.setModel(nullptr);
#endif
    }

    // -----------------------------------------------------------------------
    //  juce::MenuBarModel interface
    // -----------------------------------------------------------------------

    [[nodiscard]] juce::StringArray getMenuBarNames() override
    {
        return { "File" };
    }

    [[nodiscard]] juce::PopupMenu getMenuForIndex(int /*topLevelMenuIndex*/,
                                                  const juce::String& /*menuName*/) override
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

    void menuItemSelected(int menuItemID, int /*topLevelMenuIndex*/) override
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

    // -----------------------------------------------------------------------
    //  juce::Component interface
    // -----------------------------------------------------------------------

    void resized() override
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

        // Control row height: split remaining space equally across rows.
#if JUCE_IOS || JUCE_ANDROID
        const int numRows = 2;
#else
        const int numRows = 3;
#endif
        const int ctrlH = (area.getHeight() - (numRows - 1) * pad) / numRows;

        // Row 1: play button + label + discrete slider
        auto row1   = area.removeFromTop(ctrlH);
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

        // Row 3: Volume label + volume slider (no value box)
        volumeLabel.setBounds(area.removeFromLeft(64));
        volumeSlider.setBounds(area);
#endif
    }

    void paint(juce::Graphics& g) override
    {
        g.fillAll(juce::Colour(0xff1a1a1a));
        paintImageArea(g);
    }

private:
    // -----------------------------------------------------------------------
    //  Setup helpers
    // -----------------------------------------------------------------------

    void setupPlayButton()
    {
        playButton.onClick = [this]
        {
            if (audioToggle) audioToggle(playButton.getToggleState());
        };
        addAndMakeVisible(playButton);
    }

    void setupDiscreteSlider()
    {
        discreteSlider.setSliderStyle(juce::Slider::LinearHorizontal);
        discreteSlider.setTextBoxStyle(juce::Slider::TextBoxLeft, false, 36, 20);
        discreteSlider.setRange(1.0, 5.0, 1.0);  // 5 discrete steps
        discreteSlider.setValue(defaultDiscreteValue);
        addAndMakeVisible(discreteSlider);
    }

    void setupContinuousSlider()
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

    void setupValueBox()
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

    void setupLabels()
    {
        discreteLabel.setText("Steps", juce::dontSendNotification);
        discreteLabel.setJustificationType(juce::Justification::centredLeft);
        addAndMakeVisible(discreteLabel);

        continuousLabel.setText("Cutoff", juce::dontSendNotification);
        continuousLabel.setJustificationType(juce::Justification::centredLeft);
        addAndMakeVisible(continuousLabel);

#if ! (JUCE_IOS || JUCE_ANDROID)
        volumeLabel.setText("Volume", juce::dontSendNotification);
        volumeLabel.setJustificationType(juce::Justification::centredLeft);
        addAndMakeVisible(volumeLabel);
#endif
    }

    void setupMobileMenuButton()
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
    void setupVolumeSlider()
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
#endif

    // -----------------------------------------------------------------------
    //  Painting
    // -----------------------------------------------------------------------

    void paintImageArea(juce::Graphics& g)
    {
        g.setColour(juce::Colour(0xff222222));
        g.fillRoundedRectangle(imageArea.toFloat(), 8.0f);

        // Decorative static-noise waveform.
        // Fixed seed ensures identical decorative pattern on every paint.
        static constexpr int   waveformSampleCount = 80;    // line segments
        static constexpr int   waveformSeed        = 12345;  // fixed for visual consistency
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

    // -----------------------------------------------------------------------
    //  Value-box / slider synchronisation
    // -----------------------------------------------------------------------

    void syncValueBox()
    {
        continuousValueBox.setText(
            juce::String(continuousSlider.getValue(), 1), false);
    }

    void applyValueBox()
    {
        const double v = juce::jlimit(0.0, 100.0,
                                      continuousValueBox.getText().getDoubleValue());
        continuousSlider.setValue(v, juce::sendNotificationSync);
        syncValueBox();
    }

    // -----------------------------------------------------------------------
    //  Preset I/O
    // -----------------------------------------------------------------------

    void savePreset()
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
            preset.setAttribute("discreteValue",  discreteSlider.getValue());
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

    void loadPreset()
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
                const double discVal = xml->getDoubleAttribute("discreteValue",  defaultDiscreteValue);
                const double contVal = xml->getDoubleAttribute("continuousValue", defaultContinuousValue);
                const bool   playing = xml->getIntAttribute("isPlaying", 0) != 0;

                discreteSlider.setValue(discVal);
                continuousSlider.setValue(contVal);
                syncValueBox();

#if ! (JUCE_IOS || JUCE_ANDROID)
                const double volVal = xml->getDoubleAttribute("volumeValue", defaultVolumeValue);
                volumeSlider.setValue(volVal);
#endif

                if (playing != playButton.getToggleState())
                    playButton.setToggleState(playing, juce::sendNotification);
            }
        });
    }

#if JUCE_IOS || JUCE_ANDROID
    void showMobileMenu()
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

    // -----------------------------------------------------------------------
    //  Data members (declaration order matches dependency chain;
    //  destruction is in reverse order — see LIFO note in EnagaApplication)
    // -----------------------------------------------------------------------

    AudioToggleCallback audioToggle;
    AudioFilterCallback audioFilter;
    AudioGainCallback   audioGain;

    // Default values used at construction and as fallbacks when loading presets.
    static constexpr double defaultDiscreteValue   = 3.0;
    static constexpr double defaultContinuousValue = 100.0;
    static constexpr double defaultVolumeValue     = 100.0;

    PlayButton       playButton;
    juce::Slider     discreteSlider;
    juce::Slider     continuousSlider;
    juce::TextEditor continuousValueBox;
    juce::Label      discreteLabel;
    juce::Label      continuousLabel;

#if ! (JUCE_IOS || JUCE_ANDROID)
    juce::Slider     volumeSlider;
    juce::Label      volumeLabel;
#endif

    juce::Rectangle<int> imageArea;

#if JUCE_IOS || JUCE_ANDROID
    juce::TextButton mobileMenuButton;
#else
    static constexpr int menuBarHeight = 24;
    juce::MenuBarComponent menuBar;
#endif

    std::unique_ptr<juce::FileChooser> fileChooser;
};

// ============================================================================
//  MainWindow
// ============================================================================

/**
 * Single application window hosting MainComponent.
 *
 * Keeping a window open is what keeps the JUCE event loop (and therefore the
 * audio stream) alive.  Closing the window triggers a clean exit via
 * systemRequestedQuit(), which calls shutdown() before the process exits.
 */
class MainWindow final : public juce::DocumentWindow
{
public:
    MainWindow(const juce::String& name,
               MainComponent::AudioToggleCallback  onToggle,
               MainComponent::AudioFilterCallback  onFilter,
               MainComponent::AudioGainCallback    onGain)
        : DocumentWindow(name,
                         juce::Colour(0xff1a1a1a),
                         DocumentWindow::allButtons)
    {
        setUsingNativeTitleBar(true);
        setResizable(true, false);
        setResizeLimits(320, 240,       // minimum per issue requirement
                        10000, 10000);  // effectively unbounded maximum

        setContentOwned(
            new MainComponent(std::move(onToggle), std::move(onFilter), std::move(onGain)), true);

        centreWithSize(480, 420);
        setVisible(true);
    }

    /** Closing the window exits the application cleanly. */
    void closeButtonPressed() override
    {
        juce::JUCEApplication::getInstance()->systemRequestedQuit();
    }
};

// ============================================================================
//  EnagaApplication
// ============================================================================

/**
 * Top-level JUCE application class.
 *
 * JUCE calls initialise() once the platform event loop is ready, then calls
 * shutdown() when the application is about to exit.  On mobile platforms the
 * lifecycle is driven by the OS (UIApplicationDelegate on iOS,
 * android.app.Activity on Android); JUCE abstracts this behind the same
 * JUCEApplication interface used on the desktop.
 *
 * Audio lifetime is user-controlled (play/stop button); the device is opened
 * in initialise() and closed in shutdown().
 *
 * TODO:C++26  'final' is already C++11, but once reflection lands in C++26
 *             consider annotating with [[clang::trivially_relocatable]] or the
 *             equivalent standard attribute for move-optimisation.
 */
class EnagaApplication final : public juce::JUCEApplication
{
public:
    // -----------------------------------------------------------------------
    //  JUCEApplication interface
    // -----------------------------------------------------------------------

    // TODO:C++26  Return type could become std::string_view when JUCE adopts it.
    [[nodiscard]] const juce::String getApplicationName()    override { return "Enaga"; }
    [[nodiscard]] const juce::String getApplicationVersion() override { return "0.1.0"; }
    [[nodiscard]] bool moreThanOneInstanceAllowed()           override { return false;   }

    /**
     * Called by JUCE after all platform initialisation is complete.
     *
     * Sets the global look and feel, creates the main window, and opens the
     * default audio output device (CoreAudio / WASAPI / ALSA, depending on
     * the platform).  Audio streaming starts only when the user presses the
     * play button.
     *
     * @param commandLine  Space-separated command-line arguments provided by
     *                     the platform.  Unused here.
     *                     TODO:C++26  Prefer std::span<std::string_view>.
     */
    void initialise(const juce::String& commandLine) override
    {
        juce::ignoreUnused(commandLine);

        // Apply dark theme globally before any window is created.
        juce::LookAndFeel::setDefaultLookAndFeel(&lookAndFeel);

        // Create the main window; audio starts only when the play button is pressed.
        mainWindow = std::make_unique<MainWindow>(
            getApplicationName(),
            [this](bool shouldPlay) { toggleAudio(shouldPlay);                  },
            [this](float v)         { noiseSource.setCutoff(v);                 },
            [this](float g)         { noiseSource.setGain(g);                   });

        // Open the platform's default audio output device (stereo, no input).
        const auto error = deviceManager.initialiseWithDefaultDevices(0, 2);
        if (error.isNotEmpty())
        {
            juce::Logger::writeToLog("Audio device error: " + error);
            juce::AlertWindow::showMessageBoxAsync(
                juce::MessageBoxIconType::WarningIcon,
                "Audio Error",
                "Could not open the audio device:\n\n" + error
                    + "\n\nClose and re-open the application after checking "
                      "your audio settings.",
                "OK");
            return;
        }

        // Connect source to player; the callback is added when the play button
        // is pressed via toggleAudio(true).
        sourcePlayer.setSource(&noiseSource);
    }

    /**
     * Called by JUCE just before the application process exits.
     * Tears down audio in the reverse order of initialise() to avoid
     * use-after-free on the audio thread.
     */
    void shutdown() override
    {
        mainWindow.reset();
        juce::LookAndFeel::setDefaultLookAndFeel(nullptr);
        deviceManager.removeAudioCallback(&sourcePlayer);
        sourcePlayer.setSource(nullptr);
        deviceManager.closeAudioDevice();
    }

    /** OS-level quit request (e.g. Command-Q on macOS, Alt-F4 on Windows). */
    void systemRequestedQuit() override
    {
        quit();
    }

    /** Called when a second instance tries to launch (desktop only). */
    void anotherInstanceStarted(const juce::String& commandLine) override
    {
        juce::ignoreUnused(commandLine);
    }

private:
    /** Starts or stops the audio callback in response to the play button. */
    void toggleAudio(bool shouldPlay)
    {
        if (shouldPlay)
            deviceManager.addAudioCallback(&sourcePlayer);
        else
            deviceManager.removeAudioCallback(&sourcePlayer);
    }

    // Declaration order matches the dependency chain (destroyed in reverse).
    EnagaLookAndFeel             lookAndFeel;   // must outlive all UI components
    juce::AudioDeviceManager     deviceManager; // owns the hardware device
    juce::AudioSourcePlayer      sourcePlayer;  // bridges AudioSource → device
    WhiteNoiseAudioSource        noiseSource;   // generates the noise samples
    std::unique_ptr<MainWindow>  mainWindow;    // destroyed first (UI last)
};

// ============================================================================
//  Platform entry point
// ============================================================================

// START_JUCE_APPLICATION expands to the appropriate platform entry point:
//   • main()                    on macOS / Windows / Linux
//   • UIApplicationMain(…)      on iOS
//   • android_main(…)           on Android
//
// TODO:C++26  If JUCE adopts std::execution::main this macro may be removed
//             in favour of a standard mechanism.
START_JUCE_APPLICATION(EnagaApplication)
