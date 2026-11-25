#pragma once
#include <JuceHeader.h>
#include <array>
#include <cmath>
#include <algorithm>

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
        Diger,
        Major,
        Minor
    };

    struct EngineParams {
        LadType lad = LadType::Sur;
        int rootMidiNote = 62;
        float correctionStrength = 0.85f;
        float toleranceCents = 15.0f;
        float speedMs = 50.0f;
        float transitionMs = 120.0f;
        float vibratoDepth = 0.60f;
        float formantShiftSt = 1.0f;
        float pitchDrift = 0.30f;
        float air4K = 0.65f;
        float warmth = 0.35f;
        float stereoWidth = 0.55f;
        float delayMix = 0.25f;
        float reverbMix = 0.40f;
        float outputGainDb = 1.5f;
        bool naturalMode = true;
        bool aiPitchEnabled = true;
        bool aiAnalyzerEnabled = true;
        bool bypassed = false;
    };

    struct Telemetry {
        float inputHz = 0.0f;
        float targetHz = 0.0f;
        float commaCents = 0.0f;
        float rmsLevel = 0.0f;
        LadType recommendedLad = LadType::Sur;
    };

    static const char* ladToString(LadType lad) noexcept {
        switch (lad) {
            case LadType::Sur:         return "Şur";
            case LadType::Rast:        return "Rast";
            case LadType::Segah:       return "Segah";
            case LadType::Cahargah:    return "Çahargah";
            case LadType::BayatiSiraz: return "Bayatı-Şiraz";
            case LadType::Humayun:     return "Humayun";
            case LadType::Suster:      return "Şüştər";
            case LadType::Nava:        return "Nava";
            case LadType::Diger:       return "Diğər";
            case LadType::Major:       return "Major";
            case LadType::Minor:       return "Minor";
            default:                   return "Şur";
        }
    }

    void prepareToPlay(double newSampleRate, int samplesPerBlock) {
        sampleRate = (newSampleRate > 1000.0) ? newSampleRate : 44100.0;
        blockSize = std::max(1, samplesPerBlock);

        const int delaySamples = static_cast<int>(sampleRate * 0.6) + 16;
        delayBufferL.assign(static_cast<size_t>(delaySamples), 0.0f);
        delayBufferR.assign(static_cast<size_t>(delaySamples), 0.0f);
        delayWritePos = 0;

        combPosL.fill(0);
        combPosR.fill(0);
        combBufL[0].assign(1116, 0.0f); combBufL[1].assign(1188, 0.0f);
        combBufL[2].assign(1277, 0.0f); combBufL[3].assign(1356, 0.0f);
        combBufR[0].assign(1139, 0.0f); combBufR[1].assign(1211, 0.0f);
        combBufR[2].assign(1300, 0.0f); combBufR[3].assign(1379, 0.0f);

        lfoPhase = 0.0;
        driftPhase = 0.0;
        lpWarmL = lpWarmR = 0.0f;
        hpAirL = hpAirR = 0.0f;
        prevSampleL = prevSampleR = 0.0f;
        rmsSmoothed = 0.0f;
        estimatedHz = 0.0f;
        targetHz = 0.0f;
        commaCents = 0.0f;
    }

    void processBlock(juce::AudioBuffer<float>& buffer, const EngineParams& params) {
        const int numSamples = buffer.getNumSamples();
        const int numChannels = buffer.getNumChannels();
        if (numSamples <= 0 || numChannels <= 0) return;

        if (params.bypassed) {
            return;
        }

        const float* inL = buffer.getReadPointer(0);
        const float* inR = (numChannels > 1) ? buffer.getReadPointer(1) : inL;

        // Sadə RMS və tezlik təxmini (AI Analyzer + AI Pitch telemetriyası üçün)
        double sumSq = 0.0;
        int zeroCrossings = 0;
        for (int i = 0; i < numSamples; ++i) {
            const float s = 0.5f * (inL[i] + inR[i]);
            sumSq += static_cast<double>(s) * static_cast<double>(s);
            if ((prevMono <= 0.0f && s > 0.0f) || (prevMono >= 0.0f && s < 0.0f)) {
                ++zeroCrossings;
            }
            prevMono = s;
        }

        const float rms = std::sqrt(static_cast<float>(sumSq / static_cast<double>(numSamples)));
        rmsSmoothed = 0.85f * rmsSmoothed + 0.15f * rms;

        if (rmsSmoothed > 0.004f) {
            const float blockSec = static_cast<float>(numSamples) / static_cast<float>(sampleRate);
            const float rawFreq = (zeroCrossings * 0.5f) / std::max(0.0001f, blockSec);
            if (rawFreq >= 65.0f && rawFreq <= 1100.0f) {
                estimatedHz = (estimatedHz <= 0.0f) ? rawFreq : (0.75f * estimatedHz + 0.25f * rawFreq);
            }
        }

        const LadType activeLad = params.aiAnalyzerEnabled ? recommendLad(estimatedHz, params.lad) : params.lad;
        recommendedLad = activeLad;

        const auto& scaleCents = getLadCents(activeLad);
        const float rootHz = 440.0f * std::pow(2.0f, (static_cast<float>(params.rootMidiNote) - 69.0f) / 12.0f);

        if (estimatedHz > 50.0f && rootHz > 20.0f) {
            const float totalCentsFromRoot = 1200.0f * std::log2(estimatedHz / rootHz);
            const float octaveFloor = std::floor(totalCentsFromRoot / 1200.0f) * 1200.0f;
            const float inOctaveCents = totalCentsFromRoot - octaveFloor;

            float bestDegree = scaleCents[0];
            float minDist = std::abs(inOctaveCents - scaleCents[0]);
            for (float c : scaleCents) {
                const float d = std::abs(inOctaveCents - c);
                if (d < minDist) {
                    minDist = d;
                    bestDegree = c;
                }
            }

            const float diffCents = bestDegree - inOctaveCents;
            const float withinTol = (std::abs(diffCents) <= params.toleranceCents) ? 1.0f : 0.0f;
            const float natFactor = (params.naturalMode && withinTol > 0.5f) ? 0.35f : 1.0f;
            const float effStrength = params.aiPitchEnabled
                ? std::clamp(params.correctionStrength * natFactor, 0.0f, 1.0f)
                : 0.0f;

            const float correctedOctaveCents = inOctaveCents + diffCents * effStrength;
            const float targetTotalCents = octaveFloor + correctedOctaveCents;
            targetHz = rootHz * std::pow(2.0f, targetTotalCents / 1200.0f);

            const float nearestSemitoneCents = std::round(bestDegree / 100.0f) * 100.0f;
            commaCents = bestDegree - nearestSemitoneCents;
        } else {
            targetHz = estimatedHz;
            commaCents = 0.0f;
        }

        const float gainLinear = std::pow(10.0f, params.outputGainDb / 20.0f);
        const int delaySamples = std::clamp(
            static_cast<int>(sampleRate * 0.22), 1, static_cast<int>(delayBufferL.size()) - 1
        );

        float* outL = buffer.getWritePointer(0);
        float* outR = (numChannels > 1) ? buffer.getWritePointer(1) : nullptr;

        for (int i = 0; i < numSamples; ++i) {
            float l = inL[i];
            float r = inR[i];

            // Vibrato və Pitch Drift modulyasiyası
            lfoPhase += (2.0 * juce::MathConstants<double>::pi * 5.6) / sampleRate;
            if (lfoPhase > juce::MathConstants<double>::twoPi) lfoPhase -= juce::MathConstants<double>::twoPi;

            driftPhase += (2.0 * juce::MathConstants<double>::pi * 0.35) / sampleRate;
            if (driftPhase > juce::MathConstants<double>::twoPi) driftPhase -= juce::MathConstants<double>::twoPi;

            const float vibMod = static_cast<float>(std::sin(lfoPhase)) * params.vibratoDepth * 0.015f;
            const float driftMod = static_cast<float>(std::sin(driftPhase)) * params.pitchDrift * 0.02f;
            const float modFactor = 1.0f + vibMod + driftMod;

            l *= modFactor;
            r *= modFactor;

            // Warmth (aşağı-orta istilik)
            const float warmCoeff = 0.12f + 0.25f * params.warmth;
            lpWarmL += warmCoeff * (l - lpWarmL);
            lpWarmR += warmCoeff * (r - lpWarmR);
            l = l * (1.0f - 0.35f * params.warmth) + lpWarmL * (0.35f * params.warmth);
            r = r * (1.0f - 0.35f * params.warmth) + lpWarmR * (0.35f * params.warmth);

            // 4K Air (yüksək tezlik parıltısı)
            const float hpL = l - prevSampleL + 0.92f * hpAirL;
            const float hpR = r - prevSampleR + 0.92f * hpAirR;
            prevSampleL = l;
            prevSampleR = r;
            hpAirL = hpL;
            hpAirR = hpR;
            l += hpL * params.air4K * 0.35f;
            r += hpR * params.air4K * 0.35f;

            // Stereo Width
            const float mid = 0.5f * (l + r);
            const float side = 0.5f * (l - r) * (0.5f + params.stereoWidth);
            l = mid + side;
            r = mid - side;

            // Delay
            const int readPos = (delayWritePos - delaySamples + static_cast<int>(delayBufferL.size()))
                          % static_cast<int>(delayBufferL.size());
            const float delL = delayBufferL[static_cast<size_t>(readPos)];
            const float delR = delayBufferR[static_cast<size_t>(readPos)];
            delayBufferL[static_cast<size_t>(delayWritePos)] = l + delL * 0.35f;
            delayBufferR[static_cast<size_t>(delayWritePos)] = r + delR * 0.35f;
            delayWritePos = (delayWritePos + 1) % static_cast<int>(delayBufferL.size());

            l += delL * params.delayMix * 0.5f;
            r += delR * params.delayMix * 0.5f;

            // Yüngül stereo Reverb
            float revL = 0.0f;
            float revR = 0.0f;
            for (int c = 0; c < 4; ++c) {
                auto& bL = combBufL[static_cast<size_t>(c)];
                auto& bR = combBufR[static_cast<size_t>(c)];
                const size_t pL = combPosL[static_cast<size_t>(c)] % bL.size();
                const size_t pR = combPosR[static_cast<size_t>(c)] % bR.size();
                const float yL = bL[pL];
                const float yR = bR[pR];
                bL[pL] = l + yL * 0.68f;
                bR[pR] = r + yR * 0.68f;
                combPosL[static_cast<size_t>(c)] = (pL + 1) % bL.size();
                combPosR[static_cast<size_t>(c)] = (pR + 1) % bR.size();
                revL += yL * 0.25f;
                revR += yR * 0.25f;
            }

            l = l + revL * params.reverbMix * 0.45f;
            r = r + revR * params.reverbMix * 0.45f;

            outL[i] = std::clamp(l * gainLinear, -1.0f, 1.0f);
            if (outR != nullptr) {
                outR[i] = std::clamp(r * gainLinear, -1.0f, 1.0f);
            }
        }
    }

    Telemetry getTelemetry() const noexcept {
        Telemetry t;
        t.inputHz = estimatedHz;
        t.targetHz = targetHz;
        t.commaCents = commaCents;
        t.rmsLevel = std::clamp(rmsSmoothed * 3.5f, 0.0f, 1.0f);
        t.recommendedLad = recommendedLad;
        return t;
    }

private:
    static const std::array<float, 7>& getLadCents(LadType lad) noexcept {
        static const std::array<float, 7> sur        { 0.0f, 155.0f, 300.0f, 500.0f, 700.0f, 855.0f, 1000.0f };
        static const std::array<float, 7> rast       { 0.0f, 200.0f, 365.0f, 500.0f, 700.0f, 900.0f, 1065.0f };
        static const std::array<float, 7> segah      { 0.0f, 145.0f, 345.0f, 500.0f, 645.0f, 845.0f, 1000.0f };
        static const std::array<float, 7> cahargah   { 0.0f, 135.0f, 400.0f, 500.0f, 700.0f, 835.0f, 1100.0f };
        static const std::array<float, 7> bayati     { 0.0f, 200.0f, 300.0f, 500.0f, 700.0f, 800.0f, 1065.0f };
        static const std::array<float, 7> humayun    { 0.0f, 145.0f, 400.0f, 500.0f, 645.0f, 800.0f, 1000.0f };
        static const std::array<float, 7> suster     { 0.0f, 145.0f, 300.0f, 445.0f, 700.0f, 800.0f, 1000.0f };
        static const std::array<float, 7> nava       { 0.0f, 155.0f, 300.0f, 500.0f, 700.0f, 855.0f, 1065.0f };
        static const std::array<float, 7> diger      { 0.0f, 150.0f, 350.0f, 500.0f, 700.0f, 900.0f, 1100.0f };
        static const std::array<float, 7> major      { 0.0f, 200.0f, 400.0f, 500.0f, 700.0f, 900.0f, 1100.0f };
        static const std::array<float, 7> minor      { 0.0f, 200.0f, 300.0f, 500.0f, 700.0f, 800.0f, 1000.0f };

        switch (lad) {
            case LadType::Sur:         return sur;
            case LadType::Rast:        return rast;
            case LadType::Segah:       return segah;
            case LadType::Cahargah:    return cahargah;
            case LadType::BayatiSiraz: return bayati;
            case LadType::Humayun:     return humayun;
            case LadType::Suster:      return suster;
            case LadType::Nava:        return nava;
            case LadType::Diger:       return diger;
            case LadType::Major:       return major;
            case LadType::Minor:       return minor;
            default:                   return sur;
        }
    }

    static LadType recommendLad(float hz, LadType fallback) noexcept {
        if (hz <= 50.0f) return fallback;
        if (hz >= 480.0f) return LadType::Cahargah;
        if (hz >= 340.0f) return LadType::Sur;
        if (hz >= 240.0f) return LadType::Segah;
        return LadType::Rast;
    }

    double sampleRate = 44100.0;
    int blockSize = 512;

    std::vector<float> delayBufferL;
    std::vector<float> delayBufferR;
    int delayWritePos = 0;

    std::array<std::vector<float>, 4> combBufL;
    std::array<std::vector<float>, 4> combBufR;
    std::array<size_t, 4> combPosL { 0, 0, 0, 0 };
    std::array<size_t, 4> combPosR { 0, 0, 0, 0 };

    double lfoPhase = 0.0;
    double driftPhase = 0.0;

    float lpWarmL = 0.0f;
    float lpWarmR = 0.0f;
    float hpAirL = 0.0f;
    float hpAirR = 0.0f;
    float prevSampleL = 0.0f;
    float prevSampleR = 0.0f;
    float prevMono = 0.0f;

    float rmsSmoothed = 0.0f;
    float estimatedHz = 0.0f;
    float targetHz = 0.0f;
    float commaCents = 0.0f;
    LadType recommendedLad = LadType::Sur;
};
