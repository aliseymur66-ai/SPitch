#pragma once
#include <JuceHeader.h>
#include "SPitchEngine.h"
#include "SPitchParams.h"

class SPitchProcessor : public juce::AudioProcessor
{
public:
    SPitchProcessor();

    const juce::String getName() const override { return "SPitch"; }
    void prepareToPlay(double sampleRate, int samplesPerBlock) override;
    void releaseResources() override {}
    bool isBusesLayoutSupported(const BusesLayout& layouts) const override;
    void processBlock(juce::AudioBuffer<float>&, juce::MidiBuffer&) override;

    juce::AudioProcessorEditor* createEditor() override;
    bool hasEditor() const override { return true; }

    bool acceptsMidi() const override { return false; }
    bool producesMidi() const override { return false; }
    bool isMidiEffect() const override { return false; }
    double getTailLengthSeconds() const override { return 2.0; }

    int getNumPrograms() override { return 1; }
    int getCurrentProgram() override { return 0; }
    void setCurrentProgram(int) override {}
    const juce::String getProgramName(int) override { return {}; }
    void changeProgramName(int, const juce::String&) override {}

    void getStateInformation(juce::MemoryBlock& destData) override;
    void setStateInformation(const void* data, int sizeInBytes) override;

    juce::UndoManager undoManager;                 // apvts-dən ƏVVƏL elan olunmalıdır
    juce::AudioProcessorValueTreeState apvts;
    SPitchEngine engine;

    std::atomic<float> cpuLoad { 0.0f }, outLevel { 0.0f }, latencyMs { 0.0f };
    std::atomic<bool> resetRequested { false };

private:
    double currentSampleRate = 44100.0;
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(SPitchProcessor)
};
