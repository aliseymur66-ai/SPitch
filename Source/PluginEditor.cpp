#include "PluginProcessor.h"
#include "PluginEditor.h"
#include <cmath>

namespace {
    const juce::Colour bgVoid      { 0xff070509 };
    const juce::Colour bgPanel     { 0xff120d18 };
    const juce::Colour bgCard      { 0xff191221 };
    const juce::Colour goldHi      { 0xfffff2be };
    const juce::Colour goldMain    { 0xfff5bf47 };
    const juce::Colour goldBorder  { 0x8ef5bf47 };
    const juce::Colour cyanAccent  { 0xff38bdf8 };
    const juce::Colour textMuted   { 0xffcbbd9f };

    void styleRotarySlider(juce::Slider& s, const juce::String& suffix = {}) {
        s.setSliderStyle(juce::Slider::RotaryHorizontalVerticalDrag);
        s.setTextBoxStyle(juce::Slider::TextBoxBelow, false, 68, 18);
        if (suffix.isNotEmpty()) {
            s.setTextValueSuffix(suffix);
        }
        s.setColour(juce::Slider::rotarySliderFillColourId, goldMain);
        s.setColour(juce::Slider::rotarySliderOutlineColourId, juce::Colour(0xff2b2036));
        s.setColour(juce::Slider::thumbColourId, goldHi);
        s.setColour(juce::Slider::textBoxTextColourId, goldHi);
        s.setColour(juce::Slider::textBoxOutlineColourId, goldBorder);
        s.setColour(juce::Slider::textBoxBackgroundColourId, bgCard);
    }

    void styleCombo(juce::ComboBox& c) {
        c.setColour(juce::ComboBox::backgroundColourId, bgCard);
        c.setColour(juce::ComboBox::textColourId, juce::Colour(0xfffaf6ee));
        c.setColour(juce::ComboBox::outlineColourId, goldBorder);
        c.setColour(juce::ComboBox::arrowColourId, goldMain);
    }

    void styleToggle(juce::ToggleButton& b) {
        b.setColour(juce::ToggleButton::textColourId, juce::Colour(0xfffaf6ee));
        b.setColour(juce::ToggleButton::tickColourId, goldHi);
        b.setColour(juce::ToggleButton::tickDisabledColourId, textMuted);
    }
}

SPitchAudioProcessorEditor::SPitchAudioProcessorEditor(SPitchAudioProcessor& p)
    : AudioProcessorEditor(&p), audioProcessor(p) {
    setSize(980, 560);
    setResizable(true, true);
    setResizeLimits(760, 440, 1440, 860);

    populateComboBoxes();

    styleCombo(ladBox);
    styleCombo(keyBox);
    styleCombo(tuningBox);
    styleCombo(vocalModeBox);

    styleRotarySlider(strengthSlider, "%");
    styleRotarySlider(toleranceSlider, " ct");
    styleRotarySlider(vibratoSlider, "%");
    styleRotarySlider(formantSlider, " st");
    styleRotarySlider(reverbSlider, "%");
    styleRotarySlider(delaySlider, "%");
    styleRotarySlider(gainSlider, " dB");

    styleToggle(naturalModeButton);
    styleToggle(aiPitchButton);
    styleToggle(aiAnalyzerButton);
    styleToggle(bypassButton);

    addAndMakeVisible(ladBox);
    addAndMakeVisible(keyBox);
    addAndMakeVisible(tuningBox);
    addAndMakeVisible(vocalModeBox);

    addAndMakeVisible(strengthSlider);
    addAndMakeVisible(toleranceSlider);
    addAndMakeVisible(vibratoSlider);
    addAndMakeVisible(formantSlider);
    addAndMakeVisible(reverbSlider);
    addAndMakeVisible(delaySlider);
    addAndMakeVisible(gainSlider);

    addAndMakeVisible(naturalModeButton);
    addAndMakeVisible(aiPitchButton);
    addAndMakeVisible(aiAnalyzerButton);
    addAndMakeVisible(bypassButton);

    auto& apvts = audioProcessor.apvts;
    ladAttach        = std::make_unique<ComboAttachment>(apvts, "lad", ladBox);
    keyAttach        = std::make_unique<ComboAttachment>(apvts, "rootNote", keyBox);
    tuningAttach     = std::make_unique<ComboAttachment>(apvts, "tuningHz", tuningBox);
    vocalModeAttach  = std::make_unique<ComboAttachment>(apvts, "vocalMode", vocalModeBox);

    strengthAttach   = std::make_unique<SliderAttachment>(apvts, "correctionStrength", strengthSlider);
    toleranceAttach  = std::make_unique<SliderAttachment>(apvts, "toleranceCents", toleranceSlider);
    vibratoAttach    = std::make_unique<SliderAttachment>(apvts, "vibratoDepth", vibratoSlider);
    formantAttach    = std::make_unique<SliderAttachment>(apvts, "formantShift", formantSlider);
    reverbAttach     = std::make_unique<SliderAttachment>(apvts, "reverbMix", reverbSlider);
    delayAttach      = std::make_unique<SliderAttachment>(apvts, "delayMix", delaySlider);
    gainAttach       = std::make_unique<SliderAttachment>(apvts, "outputGain", gainSlider);

    naturalAttach    = std::make_unique<ButtonAttachment>(apvts, "naturalMode", naturalModeButton);
    aiPitchAttach    = std::make_unique<ButtonAttachment>(apvts, "aiPitchEnabled", aiPitchButton);
    aiAnalyzerAttach = std::make_unique<ButtonAttachment>(apvts, "aiAnalyzerEnabled", aiAnalyzerButton);
    bypassAttach     = std::make_unique<ButtonAttachment>(apvts, "bypass", bypassButton);

    startTimerHz(25);
}

SPitchAudioProcessorEditor::~SPitchAudioProcessorEditor() {
    stopTimer();
}

void SPitchAudioProcessorEditor::populateComboBoxes() {
    ladBox.clear();
    ladBox.addItemList({
        "Şur", "Rast", "Segah", "Çahargah", "Bayatı-Şiraz",
        "Humayun", "Şüştər", "Nava", "Diğər", "Major", "Minor"
    }, 1);

    keyBox.clear();
    keyBox.addItemList({ "C", "C#", "D", "D#", "E", "F", "F#", "G", "G#", "A", "A#", "B" }, 1);

    tuningBox.clear();
    tuningBox.addItemList({ "432 Hz", "440 Hz", "442 Hz" }, 1);

    vocalModeBox.clear();
    vocalModeBox.addItemList({ "Natural", "Mugham AI", "Modern Pop" }, 1);
}

void SPitchAudioProcessorEditor::timerCallback() {
    latestTelemetry = audioProcessor.getEngineTelemetry();
    phaseAnim += 0.16f;
    if (phaseAnim > juce::MathConstants<float>::twoPi) {
        phaseAnim -= juce::MathConstants<float>::twoPi;
    }
    repaint();
}

void SPitchAudioProcessorEditor::paint(juce::Graphics& g) {
    auto bounds = getLocalBounds().toFloat();

    juce::ColourGradient bgGrad(
        bgPanel, bounds.getCentreX(), 0.0f,
        bgVoid, bounds.getCentreX(), bounds.getHeight(), false
    );
    g.setGradientFill(bgGrad);
    g.fillRect(bounds);

    auto headerRect = bounds.removeFromTop(58.0f);
    g.setColour(bgCard);
    g.fillRect(headerRect);
    g.setColour(goldBorder);
    g.drawLine(0.0f, headerRect.getBottom(), getWidth() * 1.0f, headerRect.getBottom(), 1.5f);

    g.setColour(goldHi);
    g.setFont(juce::Font(24.0f, juce::Font::bold));
    g.drawText("SPitch", headerRect.reduced(16.0f, 4.0f), juce::Justification::centredLeft);

    g.setColour(goldMain);
    g.setFont(juce::Font(22.0f, juce::Font::italic | juce::Font::bold));
    g.drawText("Seymur Ali  |  Muğam & Vocal Studio", headerRect, juce::Justification::centred);

    auto footerRect = bounds.removeFromBottom(32.0f);
    g.setColour(bgCard);
    g.fillRect(footerRect);
    g.setColour(goldBorder);
    g.drawLine(0.0f, footerRect.getY(), getWidth() * 1.0f, footerRect.getY(), 1.2f);

    const juce::String recLad = SPitchEngine::ladToString(latestTelemetry.recommendedLad);
    const juce::String telemetryText =
        "AI Lad: " + recLad +
        "   |   Giriş: " + juce::String(latestTelemetry.inputHz, 1) + " Hz" +
        "   |   AI Pitch: " + juce::String(latestTelemetry.targetHz, 1) + " Hz" +
        "   |   Komma: " + juce::String(latestTelemetry.commaCents, 0) + " ct";

    g.setColour(cyanAccent);
    g.setFont(juce::Font(12.5f, juce::Font::bold));
    g.drawText(telemetryText, footerRect.reduced(14.0f, 2.0f), juce::Justification::centredLeft);

    g.setColour(textMuted);
    g.setFont(juce::Font(11.5f, juce::Font::italic));
    g.drawText("Muğam ruhu, müasir texnologiya...", footerRect.reduced(14.0f, 2.0f), juce::Justification::centredRight);

    auto topBar = bounds.removeFromTop(44.0f);
    g.setColour(bgPanel);
    g.fillRect(topBar);
    g.setColour(goldBorder);
    g.drawLine(0.0f, topBar.getBottom(), getWidth() * 1.0f, topBar.getBottom(), 1.0f);

    auto bottomModRow = bounds.removeFromBottom(126.0f);
    g.setColour(bgPanel);
    g.fillRect(bottomModRow);
    g.setColour(goldBorder);
    g.drawLine(0.0f, bottomModRow.getY(), getWidth() * 1.0f, bottomModRow.getY(), 1.0f);

    auto rightPanel = bounds.removeFromRight(210.0f);
    g.setColour(bgPanel);
    g.fillRect(rightPanel);
    g.setColour(goldBorder);
    g.drawLine(rightPanel.getX(), rightPanel.getY(), rightPanel.getX(), rightPanel.getBottom(), 1.0f);

    g.setColour(goldHi);
    g.setFont(juce::Font(14.0f, juce::Font::bold));
    g.drawText("Correction", rightPanel.removeFromTop(28.0f), juce::Justification::centred);

    auto canvasArea = bounds.reduced(10.0f, 8.0f);
    g.setColour(bgCard);
    g.fillRoundedRectangle(canvasArea, 8.0f);
    g.setColour(goldBorder);
    g.drawRoundedRectangle(canvasArea, 8.0f, 1.2f);

    g.setColour(juce::Colour(0x22f5bf47));
    for (int row = 1; row < 8; ++row) {
        const float y = canvasArea.getY() + (canvasArea.getHeight() * static_cast<float>(row)) / 8.0f;
        g.drawHorizontalLine(static_cast<int>(y), canvasArea.getX() + 4.0f, canvasArea.getRight() - 4.0f);
    }

    juce::Path wavePath;
    const int steps = 80;
    const float baseAmp = std::max(8.0f, latestTelemetry.rmsLevel * canvasArea.getHeight() * 0.35f);
    for (int i = 0; i <= steps; ++i) {
        const float t = static_cast<float>(i) / static_cast<float>(steps);
        const float x = canvasArea.getX() + t * canvasArea.getWidth();
        const float y = canvasArea.getCentreY()
                      + std::sin(t * juce::MathConstants<float>::twoPi * 2.5f + phaseAnim) * baseAmp
                      + std::cos(t * juce::MathConstants<float>::twoPi * 5.0f - phaseAnim * 0.6f) * (baseAmp * 0.35f);
        if (i == 0) wavePath.startNewSubPath(x, y);
        else wavePath.lineTo(x, y);
    }

    g.setColour(goldHi);
    g.strokePath(wavePath, juce::PathStrokeType(2.2f));

    g.setColour(goldMain);
    g.drawRoundedRectangle(getLocalBounds().toFloat().reduced(1.0f), 6.0f, 1.8f);
}

void SPitchAudioProcessorEditor::resized() {
    auto bounds = getLocalBounds();
    auto header = bounds.removeFromTop(58);
    bypassButton.setBounds(header.removeFromRight(100).reduced(10, 14));

    auto topBar = bounds.removeFromTop(44).reduced(10, 7);
    const int topW = topBar.getWidth();
    ladBox.setBounds(topBar.removeFromLeft(static_cast<int>(topW * 0.34f)).reduced(3, 0));
    keyBox.setBounds(topBar.removeFromLeft(static_cast<int>(topW * 0.16f)).reduced(3, 0));
    tuningBox.setBounds(topBar.removeFromLeft(static_cast<int>(topW * 0.22f)).reduced(3, 0));
    vocalModeBox.setBounds(topBar.reduced(3, 0));

    bounds.removeFromBottom(32);

    auto bottomRow = bounds.removeFromBottom(126).reduced(8, 6);
    const int modW = bottomRow.getWidth() / 5;
    vibratoSlider.setBounds(bottomRow.removeFromLeft(modW).reduced(4));
    formantSlider.setBounds(bottomRow.removeFromLeft(modW).reduced(4));
    reverbSlider.setBounds(bottomRow.removeFromLeft(modW).reduced(4));
    delaySlider.setBounds(bottomRow.removeFromLeft(modW).reduced(4));
    gainSlider.setBounds(bottomRow.reduced(4));

    auto rightPanel = bounds.removeFromRight(210).reduced(10, 32);
    const int dialH = rightPanel.getHeight() / 3;
    strengthSlider.setBounds(rightPanel.removeFromTop(dialH).reduced(4));
    toleranceSlider.setBounds(rightPanel.removeFromTop(dialH).reduced(4));
    naturalModeButton.setBounds(rightPanel.removeFromTop(26).reduced(2, 0));
    aiPitchButton.setBounds(rightPanel.removeFromTop(26).reduced(2, 0));
    aiAnalyzerButton.setBounds(rightPanel.removeFromTop(26).reduced(2, 0));
}
