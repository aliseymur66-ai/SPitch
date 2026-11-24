#pragma once
#include <juce_audio_processors/juce_audio_processors.h>
#include <juce_dsp/juce_dsp.h>
#include "MughamScales.h"

class SPitchAudioProcessor : public juce::AudioProcessor {
public:
    SPitchAudioProcessor();
    ~SPitchAudioProcessor() override = default;

    void prepareToPlay(double sampleRate, int samplesPerBlock) override;
    void releaseResources() override {}
    void processBlock(juce::AudioBuffer<float>&, juce::MidiBuffer&) override;

    juce::AudioProcessorEditor* createEditor() override;
    bool hasEditor() const override { return true; }

    const juce::String getName() const override { return "SPitch"; }
    bool acceptsMidi() const override { return false; }
    bool producesMidi() const override { return false; }
    double getTailLengthSeconds() const override { return 0.0; }

    int getNumPrograms() override { return 1; }
    int getCurrentProgram() override { return 0; }
    void setCurrentProgram(int) override {}
    const juce::String getProgramName(int) override { return {}; }
    void changeProgramName(int, const juce::String&) override {}

    void getStateInformation(juce::MemoryBlock& destData) override;
    void setStateInformation(const void* data, int sizeInBytes) override;

    juce::AudioProcessorValueTreeState apvts;
    std::atomic<float> currentDetectedHz { 0.0f };
    std::atomic<float> currentTargetHz   { 0.0f };

private:
    juce::AudioProcessorValueTreeState::ParameterLayout createParameters();
    double detectPitchYIN(const float* buffer, int numSamples, double sampleRate);

    double currentSampleRate = 44100.0;
    double smoothedRatio = 1.0;
    double phaseAccumulator = 0.0;
    std::vector<float> delayLine;
    int writeIndex = 0;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(SPitchAudioProcessor)
};
