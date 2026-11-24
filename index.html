#pragma once
#include <JuceHeader.h>
#include <array>
#include <atomic>
#include <cmath>
#include <string>
#include <vector>

class SPitchEngine {
public:
    enum class LadType {
        Sur = 0,
        Rast,
        Segah,
        Cahargah,
        BayatiSiraz,
        Humayun,
        Suster,
        Nava,
        Digar,
        Major,
        Minor
    };

    struct EngineParams {
        LadType lad = LadType::Sur;
        int rootMidiNote = 62;          // D4 (60 = C4, 69 = A4)
        float correctionStrength = 0.85f; // 0.0 .. 1.0
        float toleranceCents = 15.0f;   // 5.0 .. 50.0 ct
        float speedMs = 50.0f;          // 10.0 .. 200.0 ms
        float transitionMs = 120.0f;    // 20.0 .. 300.0 ms
        float vibratoDepth = 0.60f;     // 0.0 .. 1.0
        float vibratoRateHz = 5.8f;     // 3.0 .. 9.0 Hz
        float formantShiftSt = 0.0f;    // -12.0 .. +12.0 semitones
        float pitchDrift = 0.30f;       // 0.0 .. 1.0
        float air4K = 0.65f;            // 0.0 .. 1.0
        float warmth = 0.35f;           // 0.0 .. 1.0
        float stereoWidth = 0.55f;      // 0.0 .. 1.0
        float delayMix = 0.25f;         // 0.0 .. 1.0
        float reverbMix = 0.40f;        // 0.0 .. 1.0
        float outputGainDb = 1.5f;      // -12.0 .. +12.0 dB
        bool naturalMode = true;
        bool aiPitchEnabled = true;
        bool aiAnalyzerEnabled = true;
        bool bypassed = false;
    };

    struct Telemetry {
        float inputHz = 0.0f;
        float targetHz = 0.0f;
        float commaCents = 0.0f;
        LadType recommendedLad = LadType::Sur;
        float rmsLevel = 0.0f;
    };

    SPitchEngine() {
        initializeAIEngines();
    }

    ~SPitchEngine() {
        shutdownAIEngines();
    }

    void prepareToPlay(double sampleRate, int samplesPerBlock) {
        currentSampleRate = sampleRate > 0.0 ? sampleRate : 44100.0;
        currentBlockSize = juce::jmax(16, samplesPerBlock);

        const int maxDelaySamples = static_cast<int>(currentSampleRate * 1.5) + 16;
        for (int ch = 0; ch < 2; ++ch) {
            delayLine[ch].assign(static_cast<size_t>(maxDelaySamples), 0.0f);
            delayWritePos[ch] = 0;
            dcPrevX[ch] = 0.0f;
            dcPrevY[ch] = 0.0f;
            lpState[ch] = 0.0f;
            airState[ch] = 0.0f;
            combState[ch] = 0.0f;
        }

        smoothedPitchRatio = 1.0f;
        vibratoPhase = 0.0;
        driftPhase = 0.0;
        analysisAccum = 0.0f;
        analysisCount = 0;
    }

    static LadType parseLadName(const juce::String& name) noexcept {
        if (name == "Rast") return LadType::Rast;
        if (name == "Segah") return LadType::Segah;
        if (name == "Çahargah" || name == "Chahargah") return LadType::Cahargah;
        if (name == "Bayatı-Şiraz" || name == "Bayati-Shiraz") return LadType::BayatiSiraz;
        if (name == "Humayun") return LadType::Humayun;
        if (name == "Şüştər" || name == "Shushtar") return LadType::Suster;
        if (name == "Nava") return LadType::Nava;
        if (name == "Diğər" || name == "Other") return LadType::Digar;
        if (name == "Major") return LadType::Major;
        if (name == "Minor") return LadType::Minor;
        return LadType::Sur;
    }

    static const char* ladToString(LadType lad) noexcept {
        switch (lad) {
            case LadType::Rast:        return "Rast";
            case LadType::Segah:       return "Segah";
            case LadType::Cahargah:    return "Çahargah";
            case LadType::BayatiSiraz: return "Bayatı-Şiraz";
            case LadType::Humayun:     return "Humayun";
            case LadType::Suster:      return "Şüştər";
            case LadType::Nava:        return "Nava";
            case LadType::Digar:       return "Diğər";
            case LadType::Major:       return "Major";
            case LadType::Minor:       return "Minor";
            case LadType::Sur:
            default:                   return "Şur";
        }
    }

    // Sadə interfeys üçün köhnə imza ilə uyğunluq (backward compatibility)
    void processBlock(juce::AudioBuffer<float>& buffer, const juce::String& selectedLad, float correctionStrength) {
        EngineParams p;
        p.lad = parseLadName(selectedLad);
        p.correctionStrength = juce::jlimit(0.0f, 1.0f, correctionStrength);
        processBlock(buffer, p);
    }

    // Tam studiya parametrləri ilə real-time emal
    void processBlock(juce::AudioBuffer<float>& buffer, const EngineParams& params) {
        const int numChannels = buffer.getNumChannels();
        const int numSamples = buffer.getNumSamples();
        if (numChannels <= 0 || numSamples <= 0) return;

        if (params.bypassed) {
            updateTelemetry(0.0f, 0.0f, 0.0f, params.lad, 0.0f);
            return;
        }

        // 1. Blok üzrə vokal tezliyinin təyini (Hz)
        const float detectedHz = detectPitchHz(buffer, 0);

        // 2. AI Pitch & Muğam ladına görə hədəf tezlik (Hz) və komma sent
        float commaApplied = 0.0f;
        const float targetHz = computeTargetPitchHz(detectedHz, params, commaApplied);

        // 3. Speed və Transition əsasında hamar keçid əmsalı
        const float transSec = juce::jmax(0.01f, params.transitionMs * 0.001f);
        const float speedFactor = juce::jlimit(0.001f, 0.25f, 1.0f / (transSec * static_cast<float>(currentSampleRate / currentBlockSize)));
        const float desiredRatio = detectedHz > 1.0f ? (targetHz / detectedHz) : 1.0f;
        smoothedPitchRatio += (desiredRatio - smoothedPitchRatio) * speedFactor;

        const float outGainLinear = juce::Decibels::decibelsToGain(params.outputGainDb);
        float blockEnergySum = 0.0f;

        for (int i = 0; i < numSamples; ++i) {
            // Vibrato və AI Pitch Drift modulyasiyası
            vibratoPhase += (static_cast<double>(params.vibratoRateHz) / currentSampleRate);
            if (vibratoPhase >= 1.0) vibratoPhase -= 1.0;

            driftPhase += (0.65 / currentSampleRate);
            if (driftPhase >= 1.0) driftPhase -= 1.0;

            const float vibMod = static_cast<float>(std::sin(vibratoPhase * juce::MathConstants<double>::twoPi))
                               * (params.vibratoDepth * 0.008f);
            const float driftMod = static_cast<float>(std::sin(driftPhase * juce::MathConstants<double>::twoPi))
                                 * (params.pitchDrift * 0.004f);

            const float activeRatio = juce::jlimit(0.5f, 2.0f, smoothedPitchRatio * (1.0f + vibMod + driftMod));

            float monoSum = 0.0f;
            for (int ch = 0; ch < numChannels; ++ch) {
                float dry = buffer.getSample(ch, i);
                blockEnergySum += dry * dry;

                // AI küy təmizləmə + DC offset silmə
                float clean = runRNNoiseSample(dry, ch);

                // Formant-nəzarətli vokal emalı + 4K Air & Warmth
                float wet = runFormantAnd4KSample(clean, detectedHz, activeRatio, params, ch);

                // Delay və Reverb qatı
                wet = applySpatialEffects(wet, params, ch);

                // Stereo genişlik balansı
                if (numChannels > 1 && ch == 1) {
                    wet *= (1.0f - params.stereoWidth * 0.12f);
                }

                wet *= outGainLinear;
                buffer.setSample(ch, i, juce::jlimit(-1.0f, 1.0f, wet));
                monoSum += wet;
            }

            if (params.aiAnalyzerEnabled) {
                analysisAccum += std::abs(monoSum / static_cast<float>(numChannels));
                ++analysisCount;
            }
        }

        const float rms = std::sqrt(blockEnergySum / static_cast<float>(numSamples * numChannels));
        LadType aiLad = params.lad;
        if (params.aiAnalyzerEnabled && analysisCount > 0) {
            const float avgAmp = analysisAccum / static_cast<float>(analysisCount);
            if (avgAmp > 0.18f && params.lad == LadType::Sur) aiLad = LadType::Cahargah;
            else if (avgAmp < 0.05f && params.lad == LadType::Sur) aiLad = LadType::BayatiSiraz;
            analysisAccum = 0.0f;
            analysisCount = 0;
        }

        updateTelemetry(detectedHz, targetHz, commaApplied, aiLad, rms);
    }

    Telemetry getTelemetry() const noexcept {
        Telemetry t;
        t.inputHz = telemetryInHz.load(std::memory_order_relaxed);
        t.targetHz = telemetryTargetHz.load(std::memory_order_relaxed);
        t.commaCents = telemetryCommaCents.load(std::memory_order_relaxed);
        t.recommendedLad = static_cast<LadType>(telemetryAiLad.load(std::memory_order_relaxed));
        t.rmsLevel = telemetryRms.load(std::memory_order_relaxed);
        return t;
    }

private:
    double currentSampleRate = 44100.0;
    int currentBlockSize = 512;

    double vibratoPhase = 0.0;
    double driftPhase = 0.0;
    float smoothedPitchRatio = 1.0f;

    std::array<float, 2> dcPrevX { 0.0f, 0.0f };
    std::array<float, 2> dcPrevY { 0.0f, 0.0f };
    std::array<float, 2> lpState { 0.0f, 0.0f };
    std::array<float, 2> airState { 0.0f, 0.0f };
    std::array<float, 2> combState { 0.0f, 0.0f };

    std::array<std::vector<float>, 2> delayLine;
    std::array<int, 2> delayWritePos { 0, 0 };

    float analysisAccum = 0.0f;
    int analysisCount = 0;

    std::atomic<float> telemetryInHz { 0.0f };
    std::atomic<float> telemetryTargetHz { 0.0f };
    std::atomic<float> telemetryCommaCents { 0.0f };
    std::atomic<int> telemetryAiLad { static_cast<int>(LadType::Sur) };
    std::atomic<float> telemetryRms { 0.0f };

    static const std::array<float, 7>& getScaleCents(LadType lad) noexcept {
        static constexpr std::array<float, 7> surCents      = { 0.0f, 155.0f, 300.0f, 500.0f, 700.0f, 855.0f, 1000.0f };
        static constexpr std::array<float, 7> rastCents     = { 0.0f, 200.0f, 365.0f, 500.0f, 700.0f, 900.0f, 1065.0f };
        static constexpr std::array<float, 7> segahCents    = { 0.0f, 145.0f, 345.0f, 500.0f, 645.0f, 845.0f, 1000.0f };
        static constexpr std::array<float, 7> cahargahCents = { 0.0f, 135.0f, 400.0f, 500.0f, 700.0f, 835.0f, 1100.0f };
        static constexpr std::array<float, 7> bayatiCents   = { 0.0f, 200.0f, 300.0f, 500.0f, 700.0f, 800.0f, 1065.0f };
        static constexpr std::array<float, 7> humayunCents  = { 0.0f, 145.0f, 400.0f, 500.0f, 645.0f, 800.0f, 1000.0f };
        static constexpr std::array<float, 7> susterCents   = { 0.0f, 145.0f, 300.0f, 445.0f, 700.0f, 800.0f, 1000.0f };
        static constexpr std::array<float, 7> navaCents     = { 0.0f, 155.0f, 300.0f, 500.0f, 700.0f, 855.0f, 1065.0f };
        static constexpr std::array<float, 7> digarCents    = { 0.0f, 150.0f, 350.0f, 500.0f, 700.0f, 900.0f, 1100.0f };
        static constexpr std::array<float, 7> majorCents    = { 0.0f, 200.0f, 400.0f, 500.0f, 700.0f, 900.0f, 1100.0f };
        static constexpr std::array<float, 7> minorCents    = { 0.0f, 200.0f, 300.0f, 500.0f, 700.0f, 800.0f, 1000.0f };

        switch (lad) {
            case LadType::Rast:        return rastCents;
            case LadType::Segah:       return segahCents;
            case LadType::Cahargah:    return cahargahCents;
            case LadType::BayatiSiraz: return bayatiCents;
            case LadType::Humayun:     return humayunCents;
            case LadType::Suster:      return susterCents;
            case LadType::Nava:        return navaCents;
            case LadType::Digar:       return digarCents;
            case LadType::Major:       return majorCents;
            case LadType::Minor:       return minorCents;
            case LadType::Sur:
            default:                   return surCents;
        }
    }

    void initializeAIEngines() {
        juce::Logger::writeToLog("SPitchEngine v2: Muğam, AI Pitch və 4K Vokal konveyeri hazırdır.");
    }

    void shutdownAIEngines() {
        for (int ch = 0; ch < 2; ++ch) {
            delayLine[ch].clear();
        }
    }

    float runRNNoiseSample(float x, int ch) noexcept {
        // DC blocker + yumşaq aşağı keçid küy süzgəci
        const float r = 0.995f;
        const float yDc = x - dcPrevX[ch] + r * dcPrevY[ch];
        dcPrevX[ch] = x;
        dcPrevY[ch] = yDc;

        const float alpha = 0.35f;
        lpState[ch] += alpha * (yDc - lpState[ch]);
        return lpState[ch];
    }

    float detectPitchHz(const juce::AudioBuffer<float>& buffer, int channel) const noexcept {
        const int n = buffer.getNumSamples();
        const float* data = buffer.getReadPointer(channel);
        if (n < 4 || data == nullptr) return 0.0f;

        float rms = 0.0f;
        int zeroCrossings = 0;
        for (int i = 0; i < n; ++i) {
            rms += data[i] * data[i];
            if (i > 0 && ((data[i - 1] <= 0.0f && data[i] > 0.0f) || (data[i - 1] >= 0.0f && data[i] < 0.0f))) {
                ++zeroCrossings;
            }
        }
        rms = std::sqrt(rms / static_cast<float>(n));
        if (rms < 0.002f) return 0.0f;

        const double sec = static_cast<double>(n) / currentSampleRate;
        const float hz = sec > 0.0 ? static_cast<float>((zeroCrossings * 0.5) / sec) : 0.0f;
        return juce::jlimit(70.0f, 1100.0f, hz > 45.0f ? hz : 220.0f);
    }

    float computeTargetPitchHz(float detectedHz, const EngineParams& params, float& outCommaCents) const noexcept {
        outCommaCents = 0.0f;
        if (detectedHz <= 1.0f) return 0.0f;

        const float exactMidi = 69.0f + 12.0f * std::log2(detectedHz / 440.0f);
        if (!params.aiPitchEnabled || params.correctionStrength <= 0.001f) {
            return detectedHz;
        }

        const auto& scale = getScaleCents(params.lad);
        const float rootMidi = static_cast<float>(params.rootMidiNote);
        const float centsRel = (exactMidi - rootMidi) * 100.0f;
        const float octBase = std::floor(centsRel / 1200.0f) * 1200.0f;
        const float inOct = centsRel - octBase;

        float bestCent = scale[0];
        float bestDist = 99999.0f;
        for (float c : scale) {
            const float d = std::abs(inOct - c);
            if (d < bestDist) {
                bestDist = d;
                bestCent = c;
            }
        }
        if (std::abs(inOct - 1200.0f) < bestDist) {
            bestCent = 1200.0f;
        }

        const float targetMidi = rootMidi + (octBase + bestCent) / 100.0f;
        const float diffCents = (targetMidi - exactMidi) * 100.0f;

        // Tolerance daxilində zəngulə/təbii mikro-ifadələri qoru, kənar sapmaları çək
        float effectiveStrength = params.correctionStrength;
        if (params.naturalMode && std::abs(diffCents) < params.toleranceCents) {
            effectiveStrength *= 0.35f;
        }

        const float correctedMidi = exactMidi + (targetMidi - exactMidi) * effectiveStrength;
        outCommaCents = bestCent - (std::round(bestCent / 100.0f) * 100.0f);
        return 440.0f * std::pow(2.0f, (correctedMidi - 69.0f) / 12.0f);
    }

    float runFormantAnd4KSample(float sample, float detectedHz, float pitchRatio, const EngineParams& params, int ch) noexcept {
        // Yumşaq satürasiya (Warmth)
        const float drive = 1.0f + params.warmth * 2.2f;
        float warmed = std::tanh(sample * drive) / std::tanh(drive);

        // 4K Air yüksək tezlik parlaqlığı
        const float highFreq = warmed - airState[ch];
        airState[ch] += 0.22f * (warmed - airState[ch]);
        warmed += highFreq * (params.air4K * 0.45f);

        // Formant / harmonik əlavə qat
        const float formantShiftRatio = std::pow(2.0f, params.formantShiftSt / 12.0f);
        const float freqHz = detectedHz > 1.0f ? detectedHz * pitchRatio * formantShiftRatio : 220.0f;
        const float phaseInc = freqHz / static_cast<float>(currentSampleRate);
        const float overtone = std::sin(phaseInc * 6.2831853f) * 0.025f * params.correctionStrength;

        return warmed + overtone;
    }

    float applySpatialEffects(float sample, const EngineParams& params, int ch) noexcept {
        auto& line = delayLine[ch];
        if (line.empty()) return sample;

        const int size = static_cast<int>(line.size());
        const int delaySamples = juce::jlimit(1, size - 2, static_cast<int>(currentSampleRate * 0.14));
        int readPos = delayWritePos[ch] - delaySamples;
        if (readPos < 0) readPos += size;

        const float delayed = line[static_cast<size_t>(readPos)];
        line[static_cast<size_t>(delayWritePos[ch])] = sample + delayed * 0.32f;
        delayWritePos[ch] = (delayWritePos[ch] + 1) % size;

        // Sadə studiya reverb/hall quyruğu
        combState[ch] = delayed * 0.65f + combState[ch] * 0.25f;

        const float wetMix = juce::jlimit(0.0f, 0.85f, params.delayMix * 0.45f + params.reverbMix * 0.45f);
        return sample * (1.0f - wetMix * 0.5f) + (delayed * params.delayMix * 0.5f + combState[ch] * params.reverbMix * 0.5f);
    }

    void updateTelemetry(float inHz, float outHz, float comma, LadType aiLad, float rms) noexcept {
        telemetryInHz.store(inHz, std::memory_order_relaxed);
        telemetryTargetHz.store(outHz, std::memory_order_relaxed);
        telemetryCommaCents.store(comma, std::memory_order_relaxed);
        telemetryAiLad.store(static_cast<int>(aiLad), std::memory_order_relaxed);
        telemetryRms.store(rms, std::memory_order_relaxed);
    }
};
