#include "PluginProcessor.h"
#include "PluginEditor.h"
#include <algorithm>
#include <cmath>

SPitchAudioProcessor::SPitchAudioProcessor()
    : AudioProcessor(BusesProperties()
        .withInput("Input", juce::AudioChannelSet::stereo(), true)
        .withOutput("Output", juce::AudioChannelSet::stereo(), true)),
      apvts(*this, nullptr, "Parameters", createParameters()) {}

juce::AudioProcessorValueTreeState::ParameterLayout SPitchAudioProcessor::createParameters() {
    std::vector<std::unique_ptr<juce::RangedAudioParameter>> params;
    params.push_back(std::make_unique<juce::AudioParameterInt>("ROOT", "Root Note", 0, 11, 2)); // Default: D (Re)
    params.push_back(std::make_unique<juce::AudioParameterInt>("SCALE", "Scale/Mugham", 0, 12, 7)); // Default: Şur
    params.push_back(std::make_unique<juce::AudioParameterFloat>("CORRECTION", "Correction", 0.0f, 100.0f, 90.0f));
    params.push_back(std::make_unique<juce::AudioParameterFloat>("SPEED", "Speed", 1.0f, 100.0f, 40.0f));
    params.push_back(std::make_unique<juce::AudioParameterFloat>("VIBRATO", "Keep Vibrato", 0.0f, 100.0f, 75.0f));
    return { params.begin(), params.end() };
}

void SPitchAudioProcessor::prepareToPlay(double sampleRate, int) {
    currentSampleRate = sampleRate;
    int bufLen = static_cast<int>(sampleRate * 0.25);
    delayLine.assign(static_cast<size_t>(std::max(2048, bufLen)), 0.0f);
    writeIndex = 0;
    smoothedRatio = 1.0;
    phaseAccumulator = 0.0;
    currentDetectedHz.store(0.0f);
    currentTargetHz.store(0.0f);
}

double SPitchAudioProcessor::detectPitchYIN(const float* buffer, int numSamples, double sampleRate) {
    if (buffer == nullptr || numSamples < 64 || sampleRate <= 0.0) return 0.0;

    double energy = 0.0;
    for (int i = 0; i < numSamples; ++i) {
        energy += static_cast<double>(buffer[i]) * static_cast<double>(buffer[i]);
    }
    double rms = std::sqrt(energy / numSamples);
    if (rms < 0.008) return 0.0;

    int minLag = std::max(2, static_cast<int>(sampleRate / 1000.0));
    int maxLag = std::min(numSamples / 2 - 1, static_cast<int>(sampleRate / 65.0));
    if (maxLag <= minLag + 2) return 0.0;

    std::vector<double> diff(maxLag + 1, 0.0);
    std::vector<double> cmndf(maxLag + 1, 1.0);

    for (int tau = 1; tau <= maxLag; ++tau) {
        double sum = 0.0;
        int limit = numSamples - tau;
        for (int j = 0; j < limit; ++j) {
            double delta = static_cast<double>(buffer[j]) - static_cast<double>(buffer[j + tau]);
            sum += delta * delta;
        }
        diff[tau] = sum;
    }

    double runningSum = 0.0;
    cmndf[0] = 1.0;
    for (int tau = 1; tau <= maxLag; ++tau) {
        runningSum += diff[tau];
        cmndf[tau] = (runningSum > 1e-12) ? (diff[tau] * tau / runningSum) : 1.0;
    }

    const double threshold = 0.15;
    int bestTau = -1;
    for (int tau = minLag; tau < maxLag; ++tau) {
        if (cmndf[tau] < threshold) {
            while (tau + 1 < maxLag && cmndf[tau + 1] < cmndf[tau]) {
                ++tau;
            }
            bestTau = tau;
            break;
        }
    }

    if (bestTau == -1) {
        double minVal = 1e9;
        for (int tau = minLag; tau <= maxLag; ++tau) {
            if (cmndf[tau] < minVal) {
                minVal = cmndf[tau];
                bestTau = tau;
            }
        }
    }

    if (bestTau <= minLag || bestTau >= maxLag) return 0.0;

    double s0 = cmndf[bestTau - 1];
    double s1 = cmndf[bestTau];
    double s2 = cmndf[bestTau + 1];
    double denom = 2.0 * (s0 - 2.0 * s1 + s2);
    double betterTau = static_cast<double>(bestTau);
    if (std::abs(denom) > 1e-12) {
        double shift = (s0 - s2) / denom;
        if (std::abs(shift) < 1.0) betterTau += shift;
    }

    double freq = sampleRate / betterTau;
    return (freq >= 65.0 && freq <= 1050.0) ? freq : 0.0;
}

void SPitchAudioProcessor::processBlock(juce::AudioBuffer<float>& buffer, juce::MidiBuffer&) {
    juce::ScopedNoDenormals noDenormals;
    const int numSamples = buffer.getNumSamples();
    const int numChannels = buffer.getNumChannels();
    if (numSamples == 0 || numChannels == 0 || delayLine.empty()) return;

    std::vector<float> monoIn(static_cast<size_t>(numSamples), 0.0f);
    for (int ch = 0; ch < numChannels; ++ch) {
        const float* chData = buffer.getReadPointer(ch);
        for (int i = 0; i < numSamples; ++i) {
            monoIn[static_cast<size_t>(i)] += chData[i] / static_cast<float>(numChannels);
        }
    }

    const int delaySize = static_cast<int>(delayLine.size());
    const int analysisLen = std::min(delaySize, std::max(numSamples, static_cast<int>(currentSampleRate * 0.06)));
    std::vector<float> analysisBuf(static_cast<size_t>(analysisLen), 0.0f);

    int tempWrite = writeIndex;
    for (int i = 0; i < numSamples; ++i) {
        delayLine[static_cast<size_t>(tempWrite)] = monoIn[static_cast<size_t>(i)];
        tempWrite = (tempWrite + 1) % delaySize;
    }

    int startRead = (tempWrite - analysisLen) % delaySize;
    if (startRead < 0) startRead += delaySize;
    for (int k = 0; k < analysisLen; ++k) {
        analysisBuf[static_cast<size_t>(k)] = delayLine[static_cast<size_t>((startRead + k) % delaySize)];
    }

    double detected = detectPitchYIN(analysisBuf.data(), analysisLen, currentSampleRate);

    const int rootNote = static_cast<int>(*apvts.getRawParameterValue("ROOT")) + 60;
    const int scaleIdx = static_cast<int>(*apvts.getRawParameterValue("SCALE"));
    const float correction = *apvts.getRawParameterValue("CORRECTION") / 100.0f;
    const float speed = *apvts.getRawParameterValue("SPEED") / 100.0f;

    const auto scales = MughamScales::getAllScales();
    const auto& activeScale = scales[juce::jlimit(0, static_cast<int>(scales.size()) - 1, scaleIdx)];

    double targetRatio = 1.0;
    if (detected > 65.0 && detected < 1050.0) {
        double snapped = MughamScales::snapFrequencyToScale(detected, rootNote, activeScale);
        double centsDiff = 1200.0 * std::log2(snapped / detected);
        if (std::abs(centsDiff) <= 250.0) {
            double fullRatio = snapped / detected;
            targetRatio = 1.0 + (fullRatio - 1.0) * correction;
        }
        currentDetectedHz.store(static_cast<float>(detected));
        currentTargetHz.store(static_cast<float>(detected * targetRatio));
    } else {
        currentDetectedHz.store(0.0f);
        currentTargetHz.store(0.0f);
    }

    const double alpha = 1.0 - std::exp(-numSamples / (currentSampleRate * (0.008 + (1.0 - speed) * 0.08)));
    smoothedRatio += alpha * (targetRatio - smoothedRatio);
    smoothedRatio = juce::jlimit(0.5, 2.0, smoothedRatio);

    const double grainSize = std::max(256.0, currentSampleRate * 0.032);
    const double twoPi = juce::MathConstants<double>::twoPi;

    for (int i = 0; i < numSamples; ++i) {
        const float inputSample = monoIn[static_cast<size_t>(i)];
        delayLine[static_cast<size_t>(writeIndex)] = inputSample;

        phaseAccumulator += (1.0 - smoothedRatio);
        while (phaseAccumulator >= grainSize) phaseAccumulator -= grainSize;
        while (phaseAccumulator < 0.0) phaseAccumulator += grainSize;

        double readPos1 = static_cast<double>(writeIndex) - phaseAccumulator;
        while (readPos1 < 0.0) readPos1 += delaySize;

        double readPos2 = readPos1 - grainSize * 0.5;
        while (readPos2 < 0.0) readPos2 += delaySize;

        auto readInterp = [&](double pos) -> float {
            int idx0 = static_cast<int>(pos) % delaySize;
            if (idx0 < 0) idx0 += delaySize;
            int idx1 = (idx0 + 1) % delaySize;
            float frac = static_cast<float>(pos - std::floor(pos));
            return delayLine[static_cast<size_t>(idx0)] * (1.0f - frac) + delayLine[static_cast<size_t>(idx1)] * frac;
        };

        double normPhase = phaseAccumulator / grainSize;
        double w1 = 0.5 * (1.0 - std::cos(twoPi * normPhase));
        double normPhase2 = normPhase + 0.5;
        if (normPhase2 >= 1.0) normPhase2 -= 1.0;
        double w2 = 0.5 * (1.0 - std::cos(twoPi * normPhase2));

        double sumW = w1 + w2;
        float pitchedSample = (sumW > 1e-6)
            ? static_cast<float>((readInterp(readPos1) * w1 + readInterp(readPos2) * w2) / sumW)
            : inputSample;

        if (std::abs(inputSample) < 0.0015f) {
            pitchedSample = inputSample;
        }

        for (int ch = 0; ch < numChannels; ++ch) {
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
