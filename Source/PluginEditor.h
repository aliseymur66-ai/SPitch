#pragma once
#include <juce_gui_extra/juce_gui_extra.h>
#include "PluginProcessor.h"

class SPitchAudioProcessorEditor : public juce::AudioProcessorEditor, private juce::Timer {
public:
    explicit SPitchAudioProcessorEditor(SPitchAudioProcessor&);
    ~SPitchAudioProcessorEditor() override = default;

    void paint(juce::Graphics&) override;
    void resized() override;
    void timerCallback() override;

private:
    SPitchAudioProcessor& audioProcessor;

    juce::ComboBox rootSelector;
    juce::ComboBox scaleSelector;
    juce::Slider correctionSlider, speedSlider, vibratoSlider;

    std::unique_ptr<juce::AudioProcessorValueTreeState::ComboBoxAttachment> rootAtt, scaleAtt;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> corrAtt, speedAtt, vibAtt;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(SPitchAudioProcessorEditor)
};
