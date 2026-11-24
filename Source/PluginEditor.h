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
    void drawButaMotif(juce::Graphics& g, juce::Rectangle<float> bounds, bool mirrorX) const;
    void drawPitchCurve(juce::Graphics& g, juce::Rectangle<float> bounds) const;
    void drawMiniPiano(juce::Graphics& g, juce::Rectangle<float> bounds) const;

    SPitchAudioProcessor& audioProcessor;

    juce::ComboBox ladBox;
    juce::Slider rootSlider;
    juce::Slider strengthSlider;
    juce::Slider toleranceSlider;
    juce::Slider vibratoSlider;
    juce::Slider formantSlider;
    juce::Slider driftSlider;
    juce::Slider air4kSlider;
    juce::Slider warmthSlider;
    juce::Slider stereoSlider;
    juce::Slider delaySlider;
    juce::Slider reverbSlider;
    juce::Slider gainSlider;

    juce::ToggleButton aiPitchButton { "AI Pitch" };
    juce::ToggleButton aiAnalyzerButton { "AI Analyzer" };
    juce::ToggleButton naturalModeButton { "Natural Mode" };
    juce::ToggleButton bypassButton { "Bypass" };

    juce::Label titleLabel;
    juce::Label subtitleLabel;
    juce::Label telemetryLabel;
    juce::Label aiStatusLabel;

    using ComboAttachment = juce::AudioProcessorValueTreeState::ComboBoxAttachment;
    using SliderAttachment = juce::AudioProcessorValueTreeState::SliderAttachment;
    using ButtonAttachment = juce::AudioProcessorValueTreeState::ButtonAttachment;

    std::unique_ptr<ComboAttachment> ladAttachment;
    std::unique_ptr<SliderAttachment> rootAttachment;
    std::unique_ptr<SliderAttachment> strengthAttachment;
    std::unique_ptr<SliderAttachment> toleranceAttachment;
    std::unique_ptr<SliderAttachment> vibratoAttachment;
    std::unique_ptr<SliderAttachment> formantAttachment;
    std::unique_ptr<SliderAttachment> driftAttachment;
    std::unique_ptr<SliderAttachment> air4kAttachment;
    std::unique_ptr<SliderAttachment> warmthAttachment;
    std::unique_ptr<SliderAttachment> stereoAttachment;
    std::unique_ptr<SliderAttachment> delayAttachment;
    std::unique_ptr<SliderAttachment> reverbAttachment;
    std::unique_ptr<SliderAttachment> gainAttachment;

    std::unique_ptr<ButtonAttachment> aiPitchAttachment;
    std::unique_ptr<ButtonAttachment> aiAnalyzerAttachment;
    std::unique_ptr<ButtonAttachment> naturalModeAttachment;
    std::unique_ptr<ButtonAttachment> bypassAttachment;

    float animatedPhase = 0.0f;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(SPitchAudioProcessorEditor)
};
