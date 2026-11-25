#pragma once
#include <JuceHeader.h>
#include "PluginProcessor.h"

class SPitchAudioProcessorEditor : public juce::AudioProcessorEditor,
                                   private juce::Timer {
public:
    explicit SPitchAudioProcessorEditor(SPitchAudioProcessor&);
    ~SPitchAudioProcessorEditor() override;

    void paint(juce::Graphics&) override;
    void resized() override;

private:
    void timerCallback() override;
    void populateComboBoxes();

    SPitchAudioProcessor& audioProcessor;

    juce::ComboBox ladBox;
    juce::ComboBox keyBox;
    juce::ComboBox tuningBox;
    juce::ComboBox vocalModeBox;

    juce::Slider strengthSlider;
    juce::Slider toleranceSlider;
    juce::Slider vibratoSlider;
    juce::Slider formantSlider;
    juce::Slider reverbSlider;
    juce::Slider delaySlider;
    juce::Slider gainSlider;

    juce::ToggleButton naturalModeButton { "Natural Mode" };
    juce::ToggleButton aiPitchButton { "AI Pitch" };
    juce::ToggleButton aiAnalyzerButton { "AI Analyzer" };
    juce::ToggleButton bypassButton { "Bypass" };

    using SliderAttachment = juce::AudioProcessorValueTreeState::SliderAttachment;
    using ButtonAttachment = juce::AudioProcessorValueTreeState::ButtonAttachment;
    using ComboAttachment  = juce::AudioProcessorValueTreeState::ComboBoxAttachment;

    std::unique_ptr<ComboAttachment> ladAttach;
    std::unique_ptr<ComboAttachment> keyAttach;
    std::unique_ptr<ComboAttachment> tuningAttach;
    std::unique_ptr<ComboAttachment> vocalModeAttach;

    std::unique_ptr<SliderAttachment> strengthAttach;
    std::unique_ptr<SliderAttachment> toleranceAttach;
    std::unique_ptr<SliderAttachment> vibratoAttach;
    std::unique_ptr<SliderAttachment> formantAttach;
    std::unique_ptr<SliderAttachment> reverbAttach;
    std::unique_ptr<SliderAttachment> delayAttach;
    std::unique_ptr<SliderAttachment> gainAttach;

    std::unique_ptr<ButtonAttachment> naturalAttach;
    std::unique_ptr<ButtonAttachment> aiPitchAttach;
    std::unique_ptr<ButtonAttachment> aiAnalyzerAttach;
    std::unique_ptr<ButtonAttachment> bypassAttach;

    SPitchEngine::Telemetry latestTelemetry {};
    float phaseAnim = 0.0f;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(SPitchAudioProcessorEditor)
};
