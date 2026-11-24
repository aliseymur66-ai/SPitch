#include "PluginEditor.h"
#include <cmath>

SPitchAudioProcessorEditor::SPitchAudioProcessorEditor(SPitchAudioProcessor& p)
    : AudioProcessorEditor(&p), audioProcessor(p) {
    setSize(900, 560);

    juce::StringArray notes = {"C (Do)", "C#", "D (Re)", "D#", "E (Mi)", "F (Fa)", "F#", "G (Sol)", "G#", "A (Lya)", "A#", "B (Si)"};
    rootSelector.addItemList(notes, 1);
    addAndMakeVisible(rootSelector);
    rootAtt = std::make_unique<juce::AudioProcessorValueTreeState::ComboBoxAttachment>(audioProcessor.apvts, "ROOT", rootSelector);

    auto scales = MughamScales::getAllScales();
    for (int i = 0; i < static_cast<int>(scales.size()); ++i)
        scaleSelector.addItem(scales[i].name, i + 1);
    addAndMakeVisible(scaleSelector);
    scaleAtt = std::make_unique<juce::AudioProcessorValueTreeState::ComboBoxAttachment>(audioProcessor.apvts, "SCALE", scaleSelector);

    auto setupKnob = [&](juce::Slider& s) {
        s.setSliderStyle(juce::Slider::RotaryHorizontalVerticalDrag);
        s.setTextBoxStyle(juce::Slider::TextBoxBelow, false, 60, 18);
        s.setColour(juce::Slider::rotarySliderFillColourId, juce::Colour(0xff4da3ff));
        s.setColour(juce::Slider::thumbColourId, juce::Colour(0xffe6e9ef));
        s.setColour(juce::Slider::trackColourId, juce::Colour(0xff252a34));
        addAndMakeVisible(s);
    };

    setupKnob(correctionSlider);
    setupKnob(speedSlider);
    setupKnob(vibratoSlider);

    corrAtt = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(audioProcessor.apvts, "CORRECTION", correctionSlider);
    speedAtt = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(audioProcessor.apvts, "SPEED", speedSlider);
    vibAtt = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(audioProcessor.apvts, "VIBRATO", vibratoSlider);

    scaleSelector.onChange = [this] { repaint(); };
    rootSelector.onChange = [this] { repaint(); };

    startTimerHz(30);
}

void SPitchAudioProcessorEditor::timerCallback() {
    repaint();
}

void SPitchAudioProcessorEditor::paint(juce::Graphics& g) {
    g.fillAll(juce::Colour(0xff0d0f12));

    g.setColour(juce::Colour(0xff151920));
    g.fillRoundedRectangle(10, 10, getWidth() - 20, 75, 8.0f);
    g.setColour(juce::Colour(0xff252b36));
    g.drawRoundedRectangle(10, 10, getWidth() - 20, 75, 8.0f, 1.0f);

    g.setColour(juce::Colour(0xffe6e9ef));
    g.setFont(juce::FontOptions(22.0f, juce::Font::bold));
    g.drawText("SPitch", 24, 18, 120, 28, juce::Justification::left);

    g.setColour(juce::Colour(0xff8b94a5));
    g.setFont(juce::FontOptions(11.0f));
    g.drawText("STUDIO PITCH & MUGHAM ENGINE", 24, 48, 210, 18, juce::Justification::left);

    g.setColour(juce::Colour(0xff9aa4b5));
    g.setFont(juce::FontOptions(10.0f, juce::Font::bold));
    g.drawText("CORRECTION", 540, 13, 90, 14, juce::Justification::centred);
    g.drawText("SPEED", 650, 13, 90, 14, juce::Justification::centred);
    g.drawText("VIBRATO", 760, 13, 90, 14, juce::Justification::centred);

    int pianoX = 15, pianoY = 100, pianoW = 140, numKeys = 24;
    float keyH = (getHeight() - 115.0f) / numKeys;

    int root = rootSelector.getSelectedId() - 1;
    auto scales = MughamScales::getAllScales();
    int sIdx = juce::jlimit(0, static_cast<int>(scales.size()) - 1, scaleSelector.getSelectedId() - 1);
    const auto& activeScale = scales[sIdx];

    for (int i = 0; i < numKeys; ++i) {
        int noteIndex = (numKeys - 1 - i);
        int pitchClass = noteIndex % 12;
        bool isBlack = (pitchClass == 1 || pitchClass == 3 || pitchClass == 6 || pitchClass == 8 || pitchClass == 10);

        float y = pianoY + i * keyH;
        int relSemitone = (pitchClass - root + 12) % 12;

        bool inScale = false;
        double microCent = 0.0;
        for (double c : activeScale.centsFromRoot) {
            int nearestSemi = static_cast<int>(std::round(c / 100.0)) % 12;
            if (nearestSemi == relSemitone) {
                inScale = true;
                microCent = c - std::round(c / 100.0) * 100.0;
                break;
            }
        }

        juce::Colour keyCol = isBlack ? juce::Colour(0xff171b22) : juce::Colour(0xffc9d1dc);
        if (inScale) {
            keyCol = (std::abs(microCent) > 5.0) ? juce::Colour(0xffd49b35) : juce::Colour(0xff4da3ff);
        }

        g.setColour(keyCol);
        g.fillRoundedRectangle(static_cast<float>(pianoX), y + 1.0f,
                               isBlack ? pianoW * 0.68f : static_cast<float>(pianoW),
                               keyH - 2.0f, 3.0f);

        if (inScale && std::abs(microCent) > 5.0) {
            g.setColour(juce::Colour(0xff0d0f12));
            g.setFont(juce::FontOptions(10.0f, juce::Font::bold));
            g.drawText(juce::String(static_cast<int>(microCent)) + "c",
                       pianoX + 5, static_cast<int>(y), pianoW - 10, static_cast<int>(keyH),
                       juce::Justification::right);
        }
    }

    juce::Rectangle<float> gridArea(170, 100, getWidth() - 185.0f, getHeight() - 115.0f);
    g.setColour(juce::Colour(0xff12161d));
    g.fillRoundedRectangle(gridArea, 8.0f);
    g.setColour(juce::Colour(0xff252b36));
    g.drawRoundedRectangle(gridArea, 8.0f, 1.0f);

    float detHz = audioProcessor.currentDetectedHz.load();
    float tarHz = audioProcessor.currentTargetHz.load();

    g.setColour(juce::Colour(0xffe6e9ef));
    g.setFont(juce::FontOptions(14.0f, juce::Font::bold));
    g.drawText("INPUT: " + juce::String(detHz, 1) + " Hz    |    TARGET: " + juce::String(tarHz, 1) + " Hz",
               gridArea.reduced(18), juce::Justification::topLeft);
}

void SPitchAudioProcessorEditor::resized() {
    rootSelector.setBounds(210, 28, 110, 32);
    scaleSelector.setBounds(330, 28, 190, 32);

    correctionSlider.setBounds(545, 25, 80, 58);
    speedSlider.setBounds(655, 25, 80, 58);
    vibratoSlider.setBounds(765, 25, 80, 58);
}
