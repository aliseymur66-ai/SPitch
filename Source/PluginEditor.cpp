#include "PluginProcessor.h"
#include "PluginEditor.h"
#include <cmath>

namespace {
    const juce::Colour colBgDark     { 0xff08060a };
    const juce::Colour colPanel      { 0xff120d17 };
    const juce::Colour colGoldBright { 0xfffff0b8 };
    const juce::Colour colGoldMain   { 0xfff4be49 };
    const juce::Colour colGoldDim    { 0xff8a5a12 };
    const juce::Colour colCyan       { 0xff38bdf8 };
    const juce::Colour colEmerald    { 0xff34d399 };
    const juce::Colour colTextMuted  { 0xffcbbd9f };

    void configureRotary(juce::Slider& s, const juce::String& suffix = "") {
        s.setSliderStyle(juce::Slider::RotaryHorizontalVerticalDrag);
        s.setTextBoxStyle(juce::Slider::TextBoxBelow, false, 64, 16);
        s.setTextValueSuffix(suffix);
        s.setColour(juce::Slider::rotarySliderFillColourId, colGoldMain);
        s.setColour(juce::Slider::rotarySliderOutlineColourId, juce::Colour { 0xff281f30 });
        s.setColour(juce::Slider::thumbColourId, colGoldBright);
        s.setColour(juce::Slider::textBoxTextColourId, colGoldBright);
        s.setColour(juce::Slider::textBoxOutlineColourId, juce::Colours::transparentBlack);
    }

    void configureLinear(juce::Slider& s, const juce::String& suffix = "") {
        s.setSliderStyle(juce::Slider::LinearHorizontal);
        s.setTextBoxStyle(juce::Slider::TextBoxRight, false, 54, 16);
        s.setTextValueSuffix(suffix);
        s.setColour(juce::Slider::trackColourId, colGoldMain);
        s.setColour(juce::Slider::backgroundColourId, juce::Colour { 0xff241c2c });
        s.setColour(juce::Slider::thumbColourId, colGoldBright);
        s.setColour(juce::Slider::textBoxTextColourId, colGoldBright);
        s.setColour(juce::Slider::textBoxOutlineColourId, juce::Colours::transparentBlack);
    }
}

SPitchAudioProcessorEditor::SPitchAudioProcessorEditor(SPitchAudioProcessor& p)
    : AudioProcessorEditor(&p), audioProcessor(p) {
    setSize(1040, 620);

    titleLabel.setText("SPitch", juce::dontSendNotification);
    titleLabel.setFont(juce::FontOptions(26.0f, juce::Font::bold));
    titleLabel.setColour(juce::Label::textColourId, colGoldBright);
    titleLabel.setJustificationType(juce::Justification::centred);
    addAndMakeVisible(titleLabel);

    subtitleLabel.setText("Azerbaijani Vocal Studio  •  AI Pitch & Mugham Engine", juce::dontSendNotification);
    subtitleLabel.setFont(juce::FontOptions(12.0f, juce::Font::plain));
    subtitleLabel.setColour(juce::Label::textColourId, colTextMuted);
    subtitleLabel.setJustificationType(juce::Justification::centred);
    addAndMakeVisible(subtitleLabel);

    ladBox.addItemList({
        "Şur", "Rast", "Segah", "Çahargah", "Bayatı-Şiraz",
        "Humayun", "Şüştər", "Nava", "Diğər", "Major", "Minor"
    }, 1);
    ladBox.setColour(juce::ComboBox::backgroundColourId, colPanel);
    ladBox.setColour(juce::ComboBox::textColourId, colGoldBright);
    ladBox.setColour(juce::ComboBox::outlineColourId, colGoldMain);
    addAndMakeVisible(ladBox);

    configureLinear(rootSlider, " MIDI");
    configureRotary(strengthSlider, "%");
    configureRotary(toleranceSlider, " ct");
    configureRotary(vibratoSlider, "%");
    configureRotary(formantSlider, " st");
    configureRotary(driftSlider, "%");
    configureRotary(air4kSlider, "%");
    configureRotary(warmthSlider, "%");
    configureRotary(stereoSlider, "%");
    configureRotary(delaySlider, "%");
    configureRotary(reverbSlider, "%");
    configureRotary(gainSlider, " dB");

    for (auto* sl : { &rootSlider, &strengthSlider, &toleranceSlider, &vibratoSlider,
                     &formantSlider, &driftSlider, &air4kSlider, &warmthSlider,
                     &stereoSlider, &delaySlider, &reverbSlider, &gainSlider }) {
        addAndMakeVisible(*sl);
    }

    for (auto* btn : { &aiPitchButton, &aiAnalyzerButton, &naturalModeButton, &bypassButton }) {
        btn->setColour(juce::ToggleButton::textColourId, colGoldBright);
        btn->setColour(juce::ToggleButton::tickColourId, colGoldMain);
        addAndMakeVisible(*btn);
    }

    telemetryLabel.setText("GİRİŞ: 0.0 Hz  |  HƏDƏF: 0.0 Hz  |  KOMMA: +0.0 ct", juce::dontSendNotification);
    telemetryLabel.setFont(juce::FontOptions(12.5f, juce::Font::bold));
    telemetryLabel.setColour(juce::Label::textColourId, colCyan);
    addAndMakeVisible(telemetryLabel);

    aiStatusLabel.setText("AI ANALYZER: Muğam və vokal tonallığı izlənilir...", juce::dontSendNotification);
    aiStatusLabel.setFont(juce::FontOptions(11.5f, juce::Font::plain));
    aiStatusLabel.setColour(juce::Label::textColourId, colEmerald);
    addAndMakeVisible(aiStatusLabel);

    auto& apvts = audioProcessor.apvts;
    ladAttachment         = std::make_unique<ComboAttachment>(apvts, "lad", ladBox);
    rootAttachment        = std::make_unique<SliderAttachment>(apvts, "rootMidi", rootSlider);
    strengthAttachment    = std::make_unique<SliderAttachment>(apvts, "strength", strengthSlider);
    toleranceAttachment   = std::make_unique<SliderAttachment>(apvts, "tolerance", toleranceSlider);
    vibratoAttachment     = std::make_unique<SliderAttachment>(apvts, "vibrato", vibratoSlider);
    formantAttachment     = std::make_unique<SliderAttachment>(apvts, "formant", formantSlider);
    driftAttachment       = std::make_unique<SliderAttachment>(apvts, "drift", driftSlider);
    air4kAttachment       = std::make_unique<SliderAttachment>(apvts, "air4k", air4kSlider);
    warmthAttachment      = std::make_unique<SliderAttachment>(apvts, "warmth", warmthSlider);
    stereoAttachment      = std::make_unique<SliderAttachment>(apvts, "stereoWidth", stereoSlider);
    delayAttachment       = std::make_unique<SliderAttachment>(apvts, "delayMix", delaySlider);
    reverbAttachment      = std::make_unique<SliderAttachment>(apvts, "reverbMix", reverbSlider);
    gainAttachment        = std::make_unique<SliderAttachment>(apvts, "outputGainDb", gainSlider);

    aiPitchAttachment     = std::make_unique<ButtonAttachment>(apvts, "aiPitch", aiPitchButton);
    aiAnalyzerAttachment  = std::make_unique<ButtonAttachment>(apvts, "aiAnalyzer", aiAnalyzerButton);
    naturalModeAttachment = std::make_unique<ButtonAttachment>(apvts, "naturalMode", naturalModeButton);
    bypassAttachment      = std::make_unique<ButtonAttachment>(apvts, "bypass", bypassButton);

    startTimerHz(25);
}

SPitchAudioProcessorEditor::~SPitchAudioProcessorEditor() {
    stopTimer();
}

void SPitchAudioProcessorEditor::timerCallback() {
    animatedPhase += 0.14f;
    const auto tel = audioProcessor.getEngineTelemetry();

    const juce::String sign = tel.commaCents > 0.0f ? "+" : "";
    telemetryLabel.setText(
        "GİRİŞ: " + juce::String(tel.inputHz, 1) + " Hz  |  " +
        "HƏDƏF: " + juce::String(tel.targetHz, 1) + " Hz  |  " +
        "KOMMA: " + sign + juce::String(tel.commaCents, 1) + " ct",
        juce::dontSendNotification
    );

    const char* recName = SPitchEngine::ladToString(tel.recommendedLad);
    aiStatusLabel.setText(
        juce::String("AI ANALYZER: Tövsiyə olunan lad -> ") + recName +
        "  |  Səviyyə: " + juce::String(tel.rmsLevel * 100.0f, 0) + "%",
        juce::dontSendNotification
    );

    repaint();
}

void SPitchAudioProcessorEditor::drawButaMotif(juce::Graphics& g, juce::Rectangle<float> bounds, bool mirrorX) const {
    juce::Graphics::ScopedSaveState state(g);
    if (mirrorX) {
        g.addTransform(juce::AffineTransform::scale(-1.0f, 1.0f, bounds.getCentreX(), bounds.getCentreY()));
    }

    juce::Path buta;
    const float x = bounds.getX();
    const float y = bounds.getY();
    const float w = bounds.getWidth();
    const float h = bounds.getHeight();

    buta.startNewSubPath(x + w * 0.55f, y + h * 0.08f);
    buta.cubicTo(x + w * 0.12f, y + h * 0.14f, x + w * 0.08f, y + h * 0.88f, x + w * 0.58f, y + h * 0.90f);
    buta.cubicTo(x + w * 0.92f, y + h * 0.90f, x + w * 0.94f, y + h * 0.42f, x + w * 0.56f, y + h * 0.46f);
    buta.cubicTo(x + w * 0.36f, y + h * 0.48f, x + w * 0.38f, y + h * 0.68f, x + w * 0.55f, y + h * 0.68f);

    g.setColour(colGoldMain.withAlpha(0.32f));
    g.strokePath(buta, juce::PathStrokeType(2.0f));
}

void SPitchAudioProcessorEditor::drawPitchCurve(juce::Graphics& g, juce::Rectangle<float> bounds) const {
    g.setColour(juce::Colour { 0xff0b090f });
    g.fillRoundedRectangle(bounds, 8.0f);

    g.setColour(colGoldMain.withAlpha(0.35f));
    g.drawRoundedRectangle(bounds, 8.0f, 1.2f);

    // Tor xəttləri
    g.setColour(colGoldMain.withAlpha(0.09f));
    for (int i = 1; i < 10; ++i) {
        const float gy = bounds.getY() + (bounds.getHeight() * i) / 10.0f;
        g.drawHorizontalLine(static_cast<int>(gy), bounds.getX() + 6.0f, bounds.getRight() - 6.0f);
    }
    for (int j = 1; j < 12; ++j) {
        const float gx = bounds.getX() + (bounds.getWidth() * j) / 12.0f;
        g.drawVerticalLine(static_cast<int>(gx), bounds.getY() + 6.0f, bounds.getBottom() - 6.0f);
    }

    const float str = static_cast<float>(strengthSlider.getValue() / 100.0);
    juce::Path curve;
    const int steps = 48;
    for (int i = 0; i <= steps; ++i) {
        const float t = static_cast<float>(i) / static_cast<float>(steps);
        const float px = bounds.getX() + 12.0f + t * (bounds.getWidth() - 24.0f);
        const float rawWave = 0.5f + 0.24f * std::sin(t * 7.2f + animatedPhase * 0.25f)
                                    - 0.11f * std::cos(t * 13.0f);
        const float snapped = 0.5f + (rawWave - 0.5f) * (1.0f - str * 0.22f);
        const float py = bounds.getY() + juce::jlimit(0.12f, 0.88f, snapped) * bounds.getHeight();

        if (i == 0) curve.startNewSubPath(px, py);
        else curve.lineTo(px, py);
    }

    g.setColour(colGoldMain.withAlpha(0.28f));
    g.strokePath(curve, juce::PathStrokeType(6.0f));

    g.setColour(colGoldBright);
    g.strokePath(curve, juce::PathStrokeType(2.2f));
}

void SPitchAudioProcessorEditor::drawMiniPiano(juce::Graphics& g, juce::Rectangle<float> bounds) const {
    const int numKeys = 24;
    const float keyW = bounds.getWidth() / static_cast<float>(numKeys);
    const int rootNote = static_cast<int>(rootSlider.getValue()) % 12;

    for (int i = 0; i < numKeys; ++i) {
        const int pc = i % 12;
        const bool isInRoot = (pc == rootNote);
        const bool isBlack = (pc == 1 || pc == 3 || pc == 6 || pc == 8 || pc == 10);

        juce::Rectangle<float> kr(bounds.getX() + i * keyW, bounds.getY(), keyW - 1.0f, bounds.getHeight());
        if (isInRoot) g.setColour(colGoldMain);
        else if (isBlack) g.setColour(juce::Colour { 0xff1b1524 });
        else g.setColour(juce::Colour { 0xffd8dee9 });

        g.fillRect(kr);
    }
}

void SPitchAudioProcessorEditor::paint(juce::Graphics& g) {
    juce::ColourGradient bgGrad(
        juce::Colour { 0xff160f1f }, 0.0f, 0.0f,
        colBgDark, 0.0f, static_cast<float>(getHeight()), false
    );
    g.setGradientFill(bgGrad);
    g.fillAll();

    auto outer = getLocalBounds().toFloat().reduced(6.0f);
    g.setColour(colGoldMain.withAlpha(0.55f));
    g.drawRoundedRectangle(outer, 12.0f, 1.8f);

    // Milli buta naxışları
    drawButaMotif(g, { 18.0f, 12.0f, 72.0f, 84.0f }, false);
    drawButaMotif(g, { static_cast<float>(getWidth()) - 90.0f, 12.0f, 72.0f, 84.0f }, true);

    // Qrafik və piano sahəsi
    drawPitchCurve(g, { 24.0f, 178.0f, 680.0f, 275.0f });
    drawMiniPiano(g, { 24.0f, 556.0f, static_cast<float>(getWidth()) - 48.0f, 34.0f });
}

void SPitchAudioProcessorEditor::resized() {
    const int w = getWidth();

    titleLabel.setBounds(w / 2 - 160, 14, 320, 32);
    subtitleLabel.setBounds(w / 2 - 220, 44, 440, 20);

    ladBox.setBounds(24, 76, 170, 28);
    rootSlider.setBounds(206, 74, 210, 32);

    aiPitchButton.setBounds(432, 74, 110, 30);
    aiAnalyzerButton.setBounds(546, 74, 125, 30);
    naturalModeButton.setBounds(676, 74, 130, 30);
    bypassButton.setBounds(812, 74, 90, 30);

    aiStatusLabel.setBounds(24, 114, w - 48, 22);
    telemetryLabel.setBounds(24, 140, w - 48, 24);

    // Sağ tərəf: Correction dairələri
    const int rightX = 724;
    strengthSlider.setBounds(rightX, 178, 136, 132);
    toleranceSlider.setBounds(rightX + 148, 178, 136, 132);
    formantSlider.setBounds(rightX, 320, 136, 132);
    gainSlider.setBounds(rightX + 148, 320, 136, 132);

    // Aşağı 4K / FX sırası
    const int fxY = 464;
    const int fxW = 112;
    const int gap = 10;
    juce::Slider* fxRow[] = {
        &vibratoSlider, &driftSlider, &air4kSlider, &warmthSlider,
        &stereoSlider, &delaySlider, &reverbSlider
    };
    for (int i = 0; i < 7; ++i) {
        fxRow[i]->setBounds(24 + i * (fxW + gap), fxY, fxW, 82);
    }
}
