#include "PluginProcessor.h"
#include "PluginEditor.h"

SPitchAudioProcessor::SPitchAudioProcessor()
    : AudioProcessor(BusesProperties()
        .withInput("Input", juce::AudioChannelSet::stereo(), true)
        .withOutput("Output", juce::AudioChannelSet::stereo(), true)),
      apvts(*this, nullptr, "Parameters", createParameters()) {}

juce::AudioProcessorValueTreeState::ParameterLayout SPitchAudioProcessor::createParameters() {
    std::vector<std::unique_ptr<juce::RangedAudioParameter>> params;
    params.push_back(std::make_unique<juce::AudioParameterInt>("ROOT", "Root Note", 0, 11, 2)); // Default: D (Re)
    params.push_back(std::make_unique<juce::AudioParameterInt>("SCALE", "Scale/Mugham", 0, 12, 7)); // Default: Şur
    params.push_back(std::make_unique<juce::AudioParameterFloat>("CORRECTION", "Correction", 0.0f, 100.0f, 85.0f));
    params.push_back(std::make_unique<juce::AudioParameterFloat>("SPEED", "Speed", 1.0f, 100.0f, 35.0f));
    params.push_back(std::make_unique<juce::AudioParameterFloat>("VIBRATO", "Keep Vibrato", 0.0f, 100.0f, 80.0f));
    return { params.begin(), params.end() };
}

void SPitchAudioProcessor::prepareToPlay(double sampleRate, int) {
    currentSampleRate = sampleRate;
    delayLine.assign(static_cast<size_t>(sampleRate * 0.1), 0.0f);
    writeIndex = 0;
    smoothedRatio = 1.0;
}

double SPitchAudioProcessor::detectPitchYIN(const float* buffer, int numSamples, double sampleRate) {
    int minLag = static_cast<int>(sampleRate / 1000.0); // Max 1000 Hz
    int maxLag = static_cast<int>(sampleRate / 70.0);   // Min 70 Hz
    if (maxLag >= numSamples / 2) return 0.0;

    int bestLag = 0;
    double minDiff = 1e9;

    for (int lag = minLag; lag <= maxLag; ++lag) {
        double diff = 0.0;
        for (int i = 0; i < numSamples / 2; ++i) {
            double d = buffer[i] - buffer[i + lag];
            diff += d * d;
        }
        if (diff < minDiff) {
            minDiff = diff;
            bestLag = lag;
        }
    }
    return bestLag > 0 ? sampleRate / static_cast<double>(bestLag) : 0.0;
}

void SPitchAudioProcessor::processBlock(juce::AudioBuffer<float>& buffer, juce::MidiBuffer&) {
    juce::ScopedNoDenormals noDenormals;
    int numSamples = buffer.getNumSamples();
    if (numSamples == 0) return;

    const float* inData = buffer.getReadPointer(0);
    double detected = detectPitchYIN(inData, numSamples, currentSampleRate);

    int rootNote = static_cast<int>(*apvts.getRawParameterValue("ROOT")) + 60;
    int scaleIdx = static_cast<int>(*apvts.getRawParameterValue("SCALE"));
    float correction = *apvts.getRawParameterValue("CORRECTION") / 100.0f;
    float speed = *apvts.getRawParameterValue("SPEED") / 100.0f;

    auto scales = MughamScales::getAllScales();
    const auto& activeScale = scales[juce::jlimit(0, (int)scales.size() - 1, scaleIdx)];

    double targetRatio = 1.0;
    if (detected > 75.0 && detected < 950.0) {
        double snapped = MughamScales::snapFrequencyToScale(detected, rootNote, activeScale);
        double fullRatio = snapped / detected;
        targetRatio = 1.0 + (fullRatio - 1.0) * correction;
        currentDetectedHz.store((float)detected);
        currentTargetHz.store((float)(detected * targetRatio));
    }

    // Təbii keçid yumşaldıcısı (Avtotuna bənzəməməsi üçün sürüşmə əyrisi)
    double alpha = 0.02 + speed * 0.25;
    smoothedRatio += alpha * (targetRatio - smoothedRatio);

    // PSOLA / Granular Pitch Shifter
    int delaySize = (int)delayLine.size();
    float grainSize = (float)(currentSampleRate * 0.025); // 25ms pəncərə

    for (int i = 0; i < numSamples; ++i) {
        float inputSample = inData[i];
        delayLine[writeIndex] = inputSample;

        phaseAccumulator += (1.0 - smoothedRatio);
        while (phaseAccumulator >= grainSize) phaseAccumulator -= grainSize;
        while (phaseAccumulator < 0.0) phaseAccumulator += grainSize;

        double readPos1 = writeIndex - phaseAccumulator;
        if (readPos1 < 0) readPos1 += delaySize;

        double readPos2 = readPos1 - grainSize * 0.5;
        if (readPos2 < 0) readPos2 += delaySize;

        auto readInterp = [&](double pos) {
            int idx0 = (int)pos % delaySize;
            int idx1 = (idx0 + 1) % delaySize;
            float frac = (float)(pos - std::floor(pos));
            return delayLine[idx0] * (1.0f - frac) + delayLine[idx1] * frac;
        };

        float weight = (float)(phaseAccumulator / grainSize);
        float win1 = std::sin(juce::MathConstants<float>::pi * weight);
        float win2 = std::sin(juce::MathConstants<float>::pi * std::fmod(weight + 0.5f, 1.0f));

        float pitchedSample = (readInterp(readPos1) * win1 + readInterp(readPos2) * win2) / (win1 + win2 + 1e-5f);

        for (int ch = 0; ch < buffer.getNumChannels(); ++ch) {
            buffer.getWritePointer(ch)[i] = pitchedSample;
        }
        writeIndex = (writeIndex + 1) % delaySize;
    }
}

void SPitchAudioProcessor::getStateInformation(juce::MemoryBlock& destData) {
    auto state = apvts.copyState();
    std::unique_ptr<juce::XmlElement> xml(state.createXml());
    copyXmlToBinary(*xml, destData);
}

void SPitchAudioProcessor::setStateInformation(const void* data, int sizeInBytes) {
    std::unique_ptr<juce::XmlElement> xmlState(getXmlFromBinary(data, sizeInBytes));
    if (xmlState != nullptr && xmlState->hasTagName(apvts.state.getType()))
        apvts.replaceState(juce::ValueTree::fromXml(*xmlState));
}

juce::AudioProcessorEditor* SPitchAudioProcessor::createEditor() {
    return new SPitchAudioProcessorEditor(*this);
}

juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter() {
    return new SPitchAudioProcessor();
}
