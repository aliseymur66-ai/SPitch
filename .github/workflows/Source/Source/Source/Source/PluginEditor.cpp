#include "PluginEditor.h"

SPitchAudioProcessorEditor::SPitchAudioProcessorEditor(SPitchAudioProcessor& p)
    : AudioProcessorEditor(&p), audioProcessor(p) {
    setSize(900, 560);

    // Kök Not (Mayə) Seçicisi
    juce::StringArray notes = {"C (Do)", "C#", "D (Re)", "D#", "E (Mi)", "F (Fa)", "F#", "G (Sol)", "G#", "A (Lya)", "A#", "B (Si)"};
    rootSelector.addItemList(notes, 1);
    addAndMakeVisible(rootSelector);
    rootAtt = std::make_unique<juce::AudioProcessorValueTreeState::ComboBoxAttachment>(audioProcessor.apvts, "ROOT", rootSelector);

    // Muğam və Lad Seçicisi
    auto scales = MughamScales::getAllScales();
    for (int i = 0; i < (int)scales.size(); ++i)
        scaleSelector.addItem(scales[i].name, i + 1);
    addAndMakeVisible(scaleSelector);
    scaleAtt = std::make_unique<juce::AudioProcessorValueTreeState::ComboBoxAttachment>(audioProcessor.apvts, "SCALE", scaleSelector);

    auto setupKnob = [&](juce::Slider& s) {
        s.setSliderStyle(juce::Slider::RotaryHorizontalVerticalDrag);
        s.setTextBoxStyle(juce::Slider::TextBoxBelow, false, 60, 18);
        s.setColour(juce::Slider::rotarySliderFillColourId, juce::Colour(0xff00e5ff));
        s.setColour(juce::Slider::thumbColourId, juce::Colour(0xffffb300));
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
    // Tünd Qrafit Fon
    g.fillAll(juce::Colour(0xff101318));

    // Üst Panel
    g.setColour(juce::Colour(0xff181d26));
    g.fillRoundedRectangle(10, 10, getWidth() - 20, 75, 10.0f);

    g.setColour(juce::Colour(0xff00e5ff));
    g.setFont(juce::FontOptions(24.0f, juce::Font::bold));
    g.drawText("SPitch", 25, 22, 120, 30, juce::Justification::left);

    g.setColour(juce::Colours::grey);
    g.setFont(juce::FontOptions(11.0f));
    g.drawText("MUGHAM & NATURAL PITCH ENGINE", 25, 50, 200, 20, juce::Justification::left);

    // Düymə Başlıqları
    g.setColour(juce::Colours::lightgrey);
    g.drawText("CORRECTION", 540, 12, 90, 16, juce::Justification::centred);
    g.drawText("SPEED", 650, 12, 90, 16, juce::Justification::centred);
    g.drawText("ZENGULE", 760, 12, 90, 16, juce::Justification::centred);

    // Sol Panel: Müasir Şaquli Piano Klavişləri (2 Oktava)
    int pianoX = 15, pianoY = 100, pianoW = 140, numKeys = 24;
    float keyH = (getHeight() - 115.0f) / numKeys;

    int root = rootSelector.getSelectedId() - 1;
    auto scales = MughamScales::getAllScales();
    int sIdx = juce::jlimit(0, (int)scales.size() - 1, scaleSelector.getSelectedId() - 1);
    const auto& activeScale = scales[sIdx];

    for (int i = 0; i < numKeys; ++i) {
        int noteIndex = (numKeys - 1 - i);
        int pitchClass = noteIndex % 12;
        bool isBlack = (pitchClass == 1 || pitchClass == 3 || pitchClass == 6 || pitchClass == 8 || pitchClass == 10);

        float y = pianoY + i * keyH;
        int relSemitone = (pitchClass - root + 12) % 12;

        // Seçilmiş Muğamda aktiv pərdə olub-olmadığını yoxlayırıq
        bool inScale = false;
        double microCent = 0.0;
        for (double c : activeScale.centsFromRoot) {
            int nearestSemi = (int)std::round(c / 100.0) % 12;
            if (nearestSemi == relSemitone) {
                inScale = true;
                microCent = c - std::round(c / 100.0) * 100.0;
                break;
            }
        }

        juce::Colour keyCol = isBlack ? juce::Colour(0xff1a1f29) : juce::Colour(0xffdce3ec);
        if (inScale) {
            keyCol = (std::abs(microCent) > 5.0) ? juce::Colour(0xffffb300) : juce::Colour(0xff00e5ff);
        }

        g.setColour(keyCol);
        g.fillRoundedRectangle((float)pianoX, y + 1.0f, isBlack ? pianoW * 0.68f : (float)pianoW, keyH - 2.0f, 4.0f);

        if (inScale && std::abs(microCent) > 5.0) {
            g.setColour(juce::Colours::black);
            g.setFont(juce::FontOptions(10.0f, juce::Font::bold));
            g.drawText(juce::String((int)microCent) + "c", pianoX + 5, (int)y, pianoW - 10, (int)keyH, juce::Justification::right);
        }
    }

    // Mərkəzi Pitch Monitor Ekranı
    juce::Rectangle<float> gridArea(170, 100, getWidth() - 185.0f, getHeight() - 115.0f);
    g.setColour(juce::Colour(0xff141820));
    g.fillRoundedRectangle(gridArea, 10.0f);
    g.setColour(juce::Colour(0xff262e3d));
    g.drawRoundedRectangle(gridArea, 10.0f, 1.5f);

    // Real-vaxt Tezlik Göstəricisi
    float detHz = audioProcessor.currentDetectedHz.load();
    float tarHz = audioProcessor.currentTargetHz.load();

    g.setColour(juce::Colour(0xff00e5ff));
    g.setFont(juce::FontOptions(16.0f, juce::Font::bold));
    g.drawText("Giris Tezliyi: " + juce::String(detHz, 1) + " Hz   |   Hedef Perde: " + juce::String(tarHz, 1) + " Hz",
               gridArea.reduced(20), juce::Justification::topLeft);
}

void SPitchAudioProcessorEditor::resized() {
    rootSelector.setBounds(210, 28, 110, 32);
    scaleSelector.setBounds(330, 28, 190, 32);

    correctionSlider.setBounds(545, 25, 80, 58);
    speedSlider.setBounds(655, 25, 80, 58);
    vibratoSlider.setBounds(765, 25, 80, 58);
}
