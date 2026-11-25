#pragma once
#include <JuceHeader.h>
#include <algorithm>
#include <array>
#include <atomic>
#include <cmath>
#include <vector>

/*
    SPitchEngine v2
    ---------------
    - YIN pitch detection (əvvəlki zero-crossing əvəzinə) -> dəqiq və sabit
    - Avtomatik Lad + Kök (tonika) aşkarlanması (AI Analyzer) - səs histoqramı ilə
    - Avtomatik pitch düzəltmə (AI Pitch) - lad dərəcələrinə yapışır
    - Natural Mode: tahrir / sürüşmə zamanı düzəltməni azaldır
    - Bütün parametrlər real səsə təsir edir (formant, transition, drift, delayMs ...)
    - Audio thread-də heç bir yaddaş ayırma (allocation) yoxdur
*/
class SPitchEngine {
public:
    enum class LadType {
        Sur = 0, Rast, Segah, Cahargah, BayatiSiraz, Humayun,
        Suster, Nava, Diger, Major, Minor
    };
    static constexpr int kNumLads = 11;

    struct EngineParams {
        LadType lad = LadType::Sur;
        int   rootMidiNote = 62;
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
        float tuningHz = 440.0f, vibratoRateHz = 5.8f, formantAmount = 0.7f, harmonyMix = 0.6f;
        int   harmonyVoices = 2;   // 0..2
        int   harmonyType = 2;     // 0 Major, 1 Minor, 2 Lad-dan
        int   reverbType = 1;      // 0 Room, 1 Hall, 2 Plate
        LadType mugamHint = LadType::Rast;  // AI Analyzer üçün üstünlük verilən lad
        bool  analyzerLock = false;         // true = tapılan lad/kök sabitlənir
        float delayMs = 220.0f;          // yeni: delay vaxtı
        float reverbMix = 0.40f;
        float outputGainDb = 1.5f;
        bool  naturalMode = true;
        bool  aiPitchEnabled = true;     // avtomatik pitch düzəltmə
        bool  aiAnalyzerEnabled = true;  // avtomatik lad aşkarlama
        bool  autoRoot = true;           // yeni: kökü (tonikanı) də avtomatik tap
        bool  bypassed = false;
    };

    struct Telemetry {
        float inputHz = 0.0f;
        float targetHz = 0.0f;
        float commaCents = 0.0f;
        float rmsLevel = 0.0f;
        LadType recommendedLad = LadType::Sur;
        // yeni sahələr
        int   detectedRootMidi = 62;
        float confidence = 0.0f;
        float shiftCents = 0.0f;
        bool  voiced = false;
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

    // ---------------------------------------------------------------- prepare
    void prepareToPlay(double newSampleRate, int /*samplesPerBlock*/) {
        sampleRate = (newSampleRate > 1000.0) ? newSampleRate : 44100.0;
        const float sr = static_cast<float>(sampleRate);

        // pitch shifter
        grainLength = std::max(256.0, sampleRate * 0.045);
        const int psSize = static_cast<int>(sampleRate * 0.25) + 64;
        pitchBufL.assign(static_cast<size_t>(psSize), 0.0f);
        pitchBufR.assign(static_cast<size_t>(psSize), 0.0f);

        // delay (maks 1 saniyə)
        const int dSize = static_cast<int>(sampleRate * 1.0) + 16;
        delayBufL.assign(static_cast<size_t>(dSize), 0.0f);
        delayBufR.assign(static_cast<size_t>(dSize), 0.0f);

        // reverb (Freeverb tipli, sample rate-ə uyğun miqyaslanır)
        const float k = sr / 44100.0f;
        static const int combL[4] = { 1116, 1188, 1277, 1356 };
        static const int apL[2]   = { 556, 441 };
        for (int c = 0; c < 4; ++c) {
            combsL[c].buf.assign(static_cast<size_t>(std::max(8, int(combL[c] * k))), 0.0f);
            combsR[c].buf.assign(static_cast<size_t>(std::max(8, int((combL[c] + 23) * k))), 0.0f);
        }
        for (int a = 0; a < 2; ++a) {
            apsL[a].buf.assign(static_cast<size_t>(std::max(8, int(apL[a] * k))), 0.0f);
            apsR[a].buf.assign(static_cast<size_t>(std::max(8, int((apL[a] + 23) * k))), 0.0f);
        }

        // YIN analiz parametrləri (aşağı sample rate-ə decimation)
        decim = std::max(1, static_cast<int>(std::lround(sampleRate / 11025.0)));
        fsD = sampleRate / decim;
        tauMin = std::max(2, static_cast<int>(fsD / 1100.0));
        tauMax = std::min(static_cast<int>(fsD / 65.0) + 1, kAnalysisSize / 2);
        anaRing.assign(kAnalysisSize, 0.0f);
        anaLin.assign(kAnalysisSize, 0.0f);
        yinD.assign(static_cast<size_t>(tauMax + 2), 1.0f);

        // sabit filtr əmsalları
        warmCoef = 1.0f - std::exp(-2.0f * juce::MathConstants<float>::pi * 600.0f / sr);
        formCoef = 1.0f - std::exp(-2.0f * juce::MathConstants<float>::pi * 1500.0f / sr);
        airA     = std::exp(-2.0f * juce::MathConstants<float>::pi * 4000.0f / sr);

        resetState();
    }

    void reset() { resetState(); }

    static const std::array<float, 7>& ladDegrees(LadType l) noexcept { return getLadCents(l); }

    int getLatencySamples() const noexcept {
        return static_cast<int>(grainLength * 0.5) + 6;
    }

    // ---------------------------------------------------------------- process
    void processBlock(juce::AudioBuffer<float>& buffer, const EngineParams& p) {
        juce::ScopedNoDenormals noDenormals;

        const int n = buffer.getNumSamples();
        const int ch = buffer.getNumChannels();
        if (n <= 0 || ch <= 0 || pitchBufL.empty()) return;

        if (p.bypassed) { wasBypassed = true; return; }
        if (wasBypassed) { shiftSm = 0.0f; targetShiftCents = 0.0f; curTargetValid = false; wasBypassed = false; }
        if (!paramsInit) snapParams(p);
        refHz = std::clamp(p.tuningHz, 400.0f, 480.0f);

        float* outL = buffer.getWritePointer(0);
        float* outR = (ch > 1) ? buffer.getWritePointer(1) : nullptr;

        const float sr = static_cast<float>(sampleRate);
        const float paramCoeff  = 1.0f - std::exp(-1.0f / (0.02f * sr));
        const float coeffSpeed  = 1.0f - std::exp(-1.0f / (std::max(0.003f, p.speedMs * 0.001f) * sr));
        const float coeffTrans  = 1.0f - std::exp(-1.0f / (std::max(0.003f, p.transitionMs * 0.001f) * sr));
        transitionSamples = static_cast<int>(std::max(0.0f, p.transitionMs) * 0.001f * sr);

        const float gainTarget = std::pow(10.0f, p.outputGainDb / 20.0f);
        const float delayTarget = std::clamp(p.delayMs * 0.001f * sr, 10.0f,
                                             static_cast<float>(delayBufL.size()) - 8.0f);
        const int psSize = static_cast<int>(pitchBufL.size());
        const int dSize  = static_cast<int>(delayBufL.size());
        const float twoPi = juce::MathConstants<float>::twoPi;

        auto sm = [paramCoeff](float& s, float t) { s += paramCoeff * (t - s); };

        for (int i = 0; i < n; ++i) {
            const float inL = outL[i];
            const float inR = outR ? outR[i] : inL;
            const float mono = 0.5f * (inL + inR);

            levelEnv += 0.0005f * (mono * mono - levelEnv);
            pushAnalysisSample(mono, p);

            sm(smWarmth, p.warmth);       sm(smAir, p.air4K);
            sm(smWidth, p.stereoWidth);   sm(smDelayMix, p.delayMix);
            sm(smReverbMix, p.reverbMix); sm(smGain, gainTarget);
            sm(smVib, p.vibratoDepth);    sm(smDrift, p.pitchDrift);
            sm(smFormant, p.formantShiftSt);
            smDelay += 0.0005f * (delayTarget - smDelay);

            // pitch shift əmsalı (düzəltmə + vibrato + drift) - sentlə
            const float coeff = (transLeft > 0) ? coeffTrans : coeffSpeed;
            if (transLeft > 0) --transLeft;
            shiftSm += coeff * (targetShiftCents - shiftSm);

            lfoPhase += twoPi * std::clamp(p.vibratoRateHz, 1.0f, 12.0f) / sr;     if (lfoPhase > twoPi) lfoPhase -= twoPi;
            driftPhase += twoPi * 0.35f / sr;  if (driftPhase > twoPi) driftPhase -= twoPi;
            drift2Phase += twoPi * 0.91f / sr; if (drift2Phase > twoPi) drift2Phase -= twoPi;

            const float vibCents   = std::sin(lfoPhase) * smVib * 15.0f;
            const float driftCents = (std::sin(driftPhase) + 0.6f * std::sin(drift2Phase)) * smDrift * 8.0f;
            const double ratio = std::exp2(static_cast<double>(shiftSm + vibCents + driftCents) / 1200.0);

            // ---- granular pitch shifter (iki pəncərəli, Hann crossfade)
            pitchBufL[static_cast<size_t>(pitchWritePos)] = inL;
            pitchBufR[static_cast<size_t>(pitchWritePos)] = inR;

            grainPhaseA += (1.0 - ratio) / grainLength;
            grainPhaseA -= std::floor(grainPhaseA);
            double phaseB = grainPhaseA + 0.5;
            if (phaseB >= 1.0) phaseB -= 1.0;

            const double delayA = grainPhaseA * grainLength + 6.0;
            const double delayB = phaseB * grainLength + 6.0;
            const float winA = static_cast<float>(0.5 - 0.5 * std::cos(2.0 * juce::MathConstants<double>::pi * grainPhaseA));
            const float winB = 1.0f - winA;   // Hann + 0.5 ofset: cəmi həmişə 1

            float l = readHermite(pitchBufL, pitchWritePos, delayA) * winA
                    + readHermite(pitchBufL, pitchWritePos, delayB) * winB;
            float r = outR ? (readHermite(pitchBufR, pitchWritePos, delayA) * winA
                            + readHermite(pitchBufR, pitchWritePos, delayB) * winB)
                           : l;
            // ---- harmoniya səsləri (düzəldilmiş notun üstünə lad dərəcəsi ilə)
            {
                const float hTarget = (voiced && p.harmonyVoices > 0) ? p.harmonyMix : 0.0f;
                smHarm += paramCoeff * (hTarget - smHarm);
                if (smHarm > 0.002f) {
                    const int nv = std::min(2, p.harmonyVoices);
                    for (int v = 0; v < nv; ++v) {
                        hInt[v] += coeffTrans * (hIntTarget[v] - hInt[v]);
                        const double vr = ratio * std::exp2(static_cast<double>(hInt[v]) / 1200.0);
                        hPhase[v] += (1.0 - vr) / grainLength;
                        hPhase[v] -= std::floor(hPhase[v]);
                        double pb = hPhase[v] + 0.5;
                        if (pb >= 1.0) pb -= 1.0;
                        const float wA = static_cast<float>(0.5 - 0.5 * std::cos(2.0 * juce::MathConstants<double>::pi * hPhase[v]));
                        const float hs = (readHermite(pitchBufL, pitchWritePos, hPhase[v] * grainLength + 6.0) * wA
                                        + readHermite(pitchBufL, pitchWritePos, pb * grainLength + 6.0) * (1.0f - wA)) * smHarm * 0.55f;
                        l += hs * (v == 0 ? 1.0f : 0.45f);
                        r += hs * (v == 0 ? 0.45f : 1.0f);
                    }
                }
            }
            if (++pitchWritePos >= psSize) pitchWritePos = 0;

            // ---- formant (təxmini spektral meyl)
            const float tilt = std::clamp(1.0f + 0.085f * smFormant * p.formantAmount, 0.5f, 1.8f);
            lpFormL += formCoef * (l - lpFormL);
            lpFormR += formCoef * (r - lpFormR);
            l = lpFormL + (l - lpFormL) * tilt;
            r = lpFormR + (r - lpFormR) * tilt;

            // ---- warmth
            const float wm = 0.35f * smWarmth;
            lpWarmL += warmCoef * (l - lpWarmL);
            lpWarmR += warmCoef * (r - lpWarmR);
            l = l * (1.0f - wm) + lpWarmL * wm;
            r = r * (1.0f - wm) + lpWarmR * wm;

            // ---- air (4 kHz high-pass əlavəsi)
            hpAirL = airA * (hpAirL + l - prevInL);  prevInL = l;
            hpAirR = airA * (hpAirR + r - prevInR);  prevInR = r;
            l += hpAirL * smAir * 0.35f;
            r += hpAirR * smAir * 0.35f;

            // ---- ping-pong delay
            float rp = static_cast<float>(delayWritePos) - smDelay;
            if (rp < 0.0f) rp += static_cast<float>(dSize);
            const int i0 = static_cast<int>(rp) % dSize;
            const int i1 = (i0 + 1) % dSize;
            const float fr = rp - std::floor(rp);
            const float delL = delayBufL[static_cast<size_t>(i0)] * (1.0f - fr) + delayBufL[static_cast<size_t>(i1)] * fr;
            const float delR = delayBufR[static_cast<size_t>(i0)] * (1.0f - fr) + delayBufR[static_cast<size_t>(i1)] * fr;
            dlyLpL += 0.45f * (delR * 0.38f - dlyLpL);
            dlyLpR += 0.45f * (delL * 0.38f - dlyLpR);
            delayBufL[static_cast<size_t>(delayWritePos)] = 0.5f * (l + r) + dlyLpL;
            delayBufR[static_cast<size_t>(delayWritePos)] = dlyLpR;
            if (++delayWritePos >= dSize) delayWritePos = 0;
            l += delL * smDelayMix * 0.5f;
            r += delR * smDelayMix * 0.5f;

            // ---- reverb
            const float revIn = (l + r) * 0.5f * 0.15f;
            const float rvFb   = (p.reverbType == 0) ? 0.70f : (p.reverbType == 1 ? 0.80f : 0.86f);
            const float rvDamp = (p.reverbType == 0) ? 0.45f : (p.reverbType == 1 ? 0.30f : 0.12f);
            float revL = 0.0f, revR = 0.0f;
            for (int c = 0; c < 4; ++c) {
                revL += combsL[c].process(revIn, rvFb, rvDamp);
                revR += combsR[c].process(revIn, rvFb, rvDamp);
            }
            for (int a = 0; a < 2; ++a) {
                revL = apsL[a].process(revL);
                revR = apsR[a].process(revR);
            }
            l += revL * smReverbMix * 0.7f;
            r += revR * smReverbMix * 0.7f;

            // ---- stereo width (wet siqnallar üzərində də işləsin deyə sonda)
            if (outR) {
                const float mid  = 0.5f * (l + r);
                const float side = 0.5f * (l - r) * (0.5f + smWidth);
                l = mid + side;
                r = mid - side;
            }

            // ---- output
            outL[i] = softClip(l * smGain);
            if (outR) outR[i] = softClip(r * smGain);
        }

        tRms.store(std::clamp(std::sqrt(levelEnv) * 3.5f, 0.0f, 1.0f), std::memory_order_relaxed);
        tShift.store(shiftSm, std::memory_order_relaxed);
        tVoiced.store(voiced, std::memory_order_relaxed);
    }

    Telemetry getTelemetry() const noexcept {
        Telemetry t;
        t.inputHz        = tInputHz.load(std::memory_order_relaxed);
        t.targetHz       = tTargetHz.load(std::memory_order_relaxed);
        t.commaCents     = tComma.load(std::memory_order_relaxed);
        t.rmsLevel       = tRms.load(std::memory_order_relaxed);
        t.recommendedLad = static_cast<LadType>(tLad.load(std::memory_order_relaxed));
        t.detectedRootMidi = tRootMidi.load(std::memory_order_relaxed);
        t.confidence     = tConf.load(std::memory_order_relaxed);
        t.shiftCents     = tShift.load(std::memory_order_relaxed);
        t.voiced         = tVoiced.load(std::memory_order_relaxed);
        return t;
    }

private:
    // ------------------------------------------------------------ constants
    static constexpr int kAnalysisSize = 1024;   // decimasiya olunmuş nümunə
    static constexpr int kHop          = 128;    // decimasiya olunmuş hop (~11-12 ms)
    static constexpr int kMedian       = 5;
    static constexpr int kKeyBins      = 120;    // 10 sent-lik qutular

    struct Comb {
        std::vector<float> buf; size_t pos = 0; float store = 0.0f;
        float process(float in, float fb, float damp) noexcept {
            const float out = buf[pos];
            store = out * (1.0f - damp) + store * damp;
            buf[pos] = in + store * fb;
            if (++pos >= buf.size()) pos = 0;
            return out;
        }
    };
    struct Allpass {
        std::vector<float> buf; size_t pos = 0;
        float process(float in) noexcept {
            const float b = buf[pos];
            const float out = -in + b;
            buf[pos] = in + b * 0.5f;
            if (++pos >= buf.size()) pos = 0;
            return out;
        }
    };

    // --------------------------------------------------------------- helpers
    static float softClip(float x) noexcept {
        const float a = std::fabs(x);
        if (a <= 0.8f) return x;
        return std::copysign(0.8f + 0.2f * std::tanh((a - 0.8f) * 5.0f), x);
    }

    static float readHermite(const std::vector<float>& buf, int writePos, double delay) noexcept {
        const int size = static_cast<int>(buf.size());
        double rp = static_cast<double>(writePos) - delay;
        while (rp < 0.0) rp += size;
        while (rp >= size) rp -= size;
        const int i1 = static_cast<int>(rp);
        const float f = static_cast<float>(rp - i1);
        const int i0 = (i1 - 1 + size) % size;
        const int i2 = (i1 + 1) % size;
        const int i3 = (i1 + 2) % size;
        const float y0 = buf[static_cast<size_t>(i0)], y1 = buf[static_cast<size_t>(i1)];
        const float y2 = buf[static_cast<size_t>(i2)], y3 = buf[static_cast<size_t>(i3)];
        const float c1 = 0.5f * (y2 - y0);
        const float c2 = y0 - 2.5f * y1 + 2.0f * y2 - 0.5f * y3;
        const float c3 = 0.5f * (y3 - y0) + 1.5f * (y1 - y2);
        return ((c3 * f + c2) * f + c1) * f + y1;
    }

    static const std::array<float, 7>& getLadCents(LadType lad) noexcept {
        static const std::array<float, 7> sur      { 0.0f, 155.0f, 300.0f, 500.0f, 700.0f, 855.0f, 1000.0f };
        static const std::array<float, 7> rast     { 0.0f, 200.0f, 365.0f, 500.0f, 700.0f, 900.0f, 1065.0f };
        static const std::array<float, 7> segah    { 0.0f, 145.0f, 345.0f, 500.0f, 645.0f, 845.0f, 1000.0f };
        static const std::array<float, 7> cahargah { 0.0f, 135.0f, 400.0f, 500.0f, 700.0f, 835.0f, 1100.0f };
        static const std::array<float, 7> bayati   { 0.0f, 200.0f, 300.0f, 500.0f, 700.0f, 800.0f, 1065.0f };
        static const std::array<float, 7> humayun  { 0.0f, 145.0f, 400.0f, 500.0f, 645.0f, 800.0f, 1000.0f };
        static const std::array<float, 7> suster   { 0.0f, 145.0f, 300.0f, 445.0f, 700.0f, 800.0f, 1000.0f };
        static const std::array<float, 7> nava     { 0.0f, 155.0f, 300.0f, 500.0f, 700.0f, 855.0f, 1065.0f };
        static const std::array<float, 7> diger    { 0.0f, 150.0f, 350.0f, 500.0f, 700.0f, 900.0f, 1100.0f };
        static const std::array<float, 7> major    { 0.0f, 200.0f, 400.0f, 500.0f, 700.0f, 900.0f, 1100.0f };
        static const std::array<float, 7> minor    { 0.0f, 200.0f, 300.0f, 500.0f, 700.0f, 800.0f, 1000.0f };
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

    static float wrap1200(float c) noexcept { return c - 1200.0f * std::floor(c / 1200.0f); }
    static float manualRootCents(int midi) noexcept { return wrap1200((static_cast<float>(midi) - 69.0f) * 100.0f); }

    // ------------------------------------------------------------- state mgmt
    void snapParams(const EngineParams& p) {
        smWarmth = p.warmth; smAir = p.air4K; smWidth = p.stereoWidth;
        smDelayMix = p.delayMix; smReverbMix = p.reverbMix;
        smGain = std::pow(10.0f, p.outputGainDb / 20.0f);
        smVib = p.vibratoDepth; smDrift = p.pitchDrift; smFormant = p.formantShiftSt;
        smDelay = std::clamp(p.delayMs * 0.001f * static_cast<float>(sampleRate), 10.0f,
                             static_cast<float>(delayBufL.size()) - 8.0f);
        paramsInit = true;
    }

    void resetState() {
        std::fill(pitchBufL.begin(), pitchBufL.end(), 0.0f);
        std::fill(pitchBufR.begin(), pitchBufR.end(), 0.0f);
        std::fill(delayBufL.begin(), delayBufL.end(), 0.0f);
        std::fill(delayBufR.begin(), delayBufR.end(), 0.0f);
        for (auto& c : combsL) { std::fill(c.buf.begin(), c.buf.end(), 0.0f); c.pos = 0; c.store = 0.0f; }
        for (auto& c : combsR) { std::fill(c.buf.begin(), c.buf.end(), 0.0f); c.pos = 0; c.store = 0.0f; }
        for (auto& a : apsL)   { std::fill(a.buf.begin(), a.buf.end(), 0.0f); a.pos = 0; }
        for (auto& a : apsR)   { std::fill(a.buf.begin(), a.buf.end(), 0.0f); a.pos = 0; }
        std::fill(anaRing.begin(), anaRing.end(), 0.0f);
        hist.fill(0.0f);

        pitchWritePos = 0; delayWritePos = 0;
        grainPhaseA = 0.0;
        lfoPhase = driftPhase = drift2Phase = 0.0f;
        hPhase[0] = hPhase[1] = 0.0; hInt[0] = hInt[1] = hIntTarget[0] = hIntTarget[1] = 0.0f; smHarm = 0.0f;
        lpWarmL = lpWarmR = lpFormL = lpFormR = 0.0f;
        hpAirL = hpAirR = prevInL = prevInR = 0.0f;
        dlyLpL = dlyLpR = 0.0f;
        levelEnv = 0.0f;
        shiftSm = targetShiftCents = 0.0f;
        transLeft = 0;
        decimAcc = 0.0f; decimCount = 0; hopCount = 0; anaWrite = 0; anaFilled = 0;
        medIdx = 0; medCount = 0; havePrev = false; prevCm = 0.0f;
        unvoicedFrames = 100; voiced = false;
        curTargetValid = false; curTargetAbs = 0.0f;
        keyReady = false; framesSinceKey = 0;
        detLad = LadType::Sur; detRootBin = 0;
        activeLad = LadType::Sur; activeRootCents = 0.0f;
        lastLad = LadType::Sur; lastRoot = -1000.0f;
        paramsInit = false; wasBypassed = false;
    }

    // --------------------------------------------------------------- analysis
    void pushAnalysisSample(float mono, const EngineParams& p) noexcept {
        decimAcc += mono;
        if (++decimCount < decim) return;
        anaRing[static_cast<size_t>(anaWrite)] = decimAcc / static_cast<float>(decim);
        decimAcc = 0.0f; decimCount = 0;
        anaWrite = (anaWrite + 1) % kAnalysisSize;
        if (anaFilled < kAnalysisSize) ++anaFilled;
        if (++hopCount >= kHop) {
            hopCount = 0;
            if (anaFilled >= kAnalysisSize) analyseFrame(p);
        }
    }

    float detectPitchYin(float& confidence) noexcept {
        confidence = 0.0f;
        const int W = kAnalysisSize - tauMax - 1;
        const float* x = anaLin.data();

        float runSum = 0.0f;
        yinD[0] = 1.0f;
        for (int tau = 1; tau <= tauMax; ++tau) {
            float d = 0.0f;
            for (int j = 0; j < W; ++j) { const float df = x[j] - x[j + tau]; d += df * df; }
            runSum += d;
            yinD[static_cast<size_t>(tau)] = (runSum > 1e-9f) ? d * static_cast<float>(tau) / runSum : 1.0f;
        }

        int tauEst = -1;
        for (int tau = tauMin; tau < tauMax; ++tau) {
            if (yinD[static_cast<size_t>(tau)] < 0.18f) {
                while (tau + 1 < tauMax && yinD[static_cast<size_t>(tau + 1)] < yinD[static_cast<size_t>(tau)]) ++tau;
                tauEst = tau;
                break;
            }
        }
        if (tauEst < 0) return 0.0f;

        float better = static_cast<float>(tauEst);
        if (tauEst > 1 && tauEst < tauMax) {
            const float s0 = yinD[static_cast<size_t>(tauEst - 1)];
            const float s1 = yinD[static_cast<size_t>(tauEst)];
            const float s2 = yinD[static_cast<size_t>(tauEst + 1)];
            const float den = s0 + s2 - 2.0f * s1;
            if (std::fabs(den) > 1e-9f) better += 0.5f * (s0 - s2) / den;
        }
        confidence = std::clamp(1.0f - yinD[static_cast<size_t>(tauEst)], 0.0f, 1.0f);
        return static_cast<float>(fsD / better);
    }

    void analyseFrame(const EngineParams& p) noexcept {
        for (int k = 0; k < kAnalysisSize; ++k)
            anaLin[static_cast<size_t>(k)] = anaRing[static_cast<size_t>((anaWrite + k) % kAnalysisSize)];

        float conf = 0.0f;
        const float hz = detectPitchYin(conf);
        const bool gate = std::sqrt(levelEnv) > 0.003f;

        if (!gate || hz < 60.0f || hz > 1200.0f || conf < 0.6f) {
            if (++unvoicedFrames > 6) {          // ~70 ms susqunluq -> düzəltməni burax
                voiced = false;
                targetShiftCents = 0.0f;
                havePrev = false;
                curTargetValid = false;
                medCount = 0; medIdx = 0;
                tConf.store(0.0f, std::memory_order_relaxed);
            }
            return;
        }
        unvoicedFrames = 0;
        voiced = true;

        const float c = 1200.0f * std::log2(hz / refHz);   // mütləq sent (A440-a nəzərən)

        // median süzgəc (oktava səhvlərinə qarşı)
        medBuf[static_cast<size_t>(medIdx)] = c;
        medIdx = (medIdx + 1) % kMedian;
        medCount = std::min(medCount + 1, kMedian);
        auto tmp = medBuf;
        std::sort(tmp.begin(), tmp.begin() + medCount);
        const float cm = tmp[static_cast<size_t>(medCount / 2)];

        // lad analizi üçün histoqram (10 sent-lik qutular, oktava daxilində)
        for (auto& h : hist) h *= 0.9993f;
        hist[static_cast<size_t>(static_cast<int>(wrap1200(cm) / 10.0f) % kKeyBins)] += conf * conf;
        if (++framesSinceKey >= 40) { framesSinceKey = 0; if (!(p.analyzerLock && keyReady)) estimateKey(p); }

        // aktiv lad + kök
        const float manualRoot = manualRootCents(p.rootMidiNote);
        if (p.aiAnalyzerEnabled && keyReady) {
            activeLad = detLad;
            activeRootCents = p.autoRoot ? static_cast<float>(detRootBin) * 10.0f : manualRoot;
        } else {
            activeLad = p.lad;
            activeRootCents = manualRoot;
        }
        if (activeLad != lastLad || std::fabs(activeRootCents - lastRoot) > 0.5f) {
            lastLad = activeLad; lastRoot = activeRootCents; curTargetValid = false;
        }

        // ən yaxın lad dərəcəsi (1200 = oktava üst kök də namizəddir)
        const auto& scale = getLadCents(activeLad);
        const float rel = wrap1200(cm - activeRootCents);
        const float octBase = cm - rel;
        float bestD = scale[0];
        float bestDist = std::fabs(rel - scale[0]);
        for (float d : scale) {
            const float dist = std::fabs(rel - d);
            if (dist < bestDist) { bestDist = dist; bestD = d; }
        }
        if (std::fabs(rel - 1200.0f) < bestDist) { bestDist = std::fabs(rel - 1200.0f); bestD = 1200.0f; }

        float targetAbs = octBase + bestD;

        // histerezis: not arasında titrəməsin
        if (curTargetValid && std::fabs(cm - curTargetAbs) < bestDist + 25.0f
                           && std::fabs(cm - curTargetAbs) < 150.0f) {
            targetAbs = curTargetAbs;
        } else {
            if (!curTargetValid || std::fabs(targetAbs - curTargetAbs) > 1.0f) transLeft = transitionSamples;
            curTargetAbs = targetAbs;
            curTargetValid = true;
        }

        // düzəltmə gücü
        const float diff = targetAbs - cm;
        float strength = 0.0f;
        if (p.aiPitchEnabled) {
            strength = p.correctionStrength;
            if (p.naturalMode) {
                if (std::fabs(diff) <= p.toleranceCents) strength *= 0.45f;       // tolerans daxilində yumşaq
                if (havePrev && std::fabs(cm - prevCm) > 40.0f) strength *= 0.3f; // tahrir / sürüşmə
            }
        }
        prevCm = cm; havePrev = true;
        targetShiftCents = std::clamp(diff * std::clamp(strength, 0.0f, 1.0f), -400.0f, 400.0f);

        // telemetriya
        const float degRel = wrap1200(targetAbs - activeRootCents);
        {   // harmoniya intervalları
            int idx = 0; float bd = 1e9f;
            for (int k = 0; k < 7; ++k) { const float d = std::fabs(degRel - scale[static_cast<size_t>(k)]); if (d < bd) { bd = d; idx = k; } }
            if (std::fabs(degRel - 1200.0f) < bd) idx = 0;
            auto up = [&](int steps) { const int j = idx + steps; return scale[static_cast<size_t>(j % 7)] + 1200.0f * static_cast<float>(j / 7) - scale[static_cast<size_t>(idx)]; };
            if (p.harmonyType == 0)      { hIntTarget[0] = 400.0f; hIntTarget[1] = 700.0f; }
            else if (p.harmonyType == 1) { hIntTarget[0] = 300.0f; hIntTarget[1] = 700.0f; }
            else                         { hIntTarget[0] = up(2);  hIntTarget[1] = up(4); }
        }
        float midi = 69.0f + activeRootCents / 100.0f;
        midi += 12.0f * std::round((static_cast<float>(p.rootMidiNote) - midi) / 12.0f);

        tInputHz.store(refHz * std::exp2(cm / 1200.0f), std::memory_order_relaxed);
        tTargetHz.store(refHz * std::exp2(targetAbs / 1200.0f), std::memory_order_relaxed);
        tComma.store(degRel - std::round(degRel / 100.0f) * 100.0f, std::memory_order_relaxed);
        tLad.store(static_cast<int>(activeLad), std::memory_order_relaxed);
        tRootMidi.store(static_cast<int>(std::lround(midi)), std::memory_order_relaxed);
        tConf.store(conf, std::memory_order_relaxed);
    }

    // Lad + kök aşkarlama: histoqramı bütün lad/kök kombinasiyaları ilə müqayisə edir
    void estimateKey(const EngineParams& p) noexcept {
        float total = 0.0f;
        for (float h : hist) total += h;
        if (total < 25.0f) return;

        std::array<float, kKeyBins> S{};
        for (int i = 0; i < kKeyBins; ++i) {
            auto h = [&](int o) { return hist[static_cast<size_t>((i + o + kKeyBins) % kKeyBins)]; };
            S[static_cast<size_t>(i)] = h(0) + 0.6f * (h(-1) + h(1)) + 0.25f * (h(-2) + h(2));
        }

        static const float w[7] = { 1.5f, 1.0f, 1.0f, 1.0f, 1.2f, 1.0f, 1.0f };
        const int manualBin = static_cast<int>(std::lround(manualRootCents(p.rootMidiNote) / 10.0f)) % kKeyBins;
        const int rBegin = p.autoRoot ? 0 : manualBin;
        const int rEnd   = p.autoRoot ? kKeyBins : manualBin + 1;

        float bestScore = -1.0f;
        LadType bestLad = p.lad;
        int bestRoot = manualBin;

        for (int li = 0; li < kNumLads; ++li) {
            const auto lad = static_cast<LadType>(li);
            const auto& degs = getLadCents(lad);
            for (int r = rBegin; r < rEnd; ++r) {
                float s = 0.0f;
                for (int k = 0; k < 7; ++k) {
                    const int idx = (r + static_cast<int>(std::lround(degs[static_cast<size_t>(k)] / 10.0f))) % kKeyBins;
                    s += w[k] * S[static_cast<size_t>(idx)];
                }
                if (keyReady && lad == detLad && r == detRootBin) s *= 1.12f;  // sabitlik
                if (lad == p.lad && r == manualBin) s *= 1.05f;
                if (lad == p.mugamHint) s *= 1.08f;                // istifadəçi seçiminə kiçik üstünlük
                if (s > bestScore) { bestScore = s; bestLad = lad; bestRoot = r; }
            }
        }
        detLad = bestLad;
        detRootBin = bestRoot;
        keyReady = true;
    }

    // ------------------------------------------------------------------ data
    double sampleRate = 44100.0;
    float refHz = 440.0f;

    // pitch shifter
    std::vector<float> pitchBufL, pitchBufR;
    int pitchWritePos = 0;
    double grainLength = 2048.0;
    double grainPhaseA = 0.0;

    // delay / reverb
    std::vector<float> delayBufL, delayBufR;
    int delayWritePos = 0;
    std::array<Comb, 4> combsL, combsR;
    std::array<Allpass, 2> apsL, apsR;

    // filtrlər
    float warmCoef = 0.1f, formCoef = 0.1f, airA = 0.5f;
    float lpWarmL = 0, lpWarmR = 0, lpFormL = 0, lpFormR = 0;
    float hpAirL = 0, hpAirR = 0, prevInL = 0, prevInR = 0;
    float dlyLpL = 0, dlyLpR = 0;
    float lfoPhase = 0, driftPhase = 0, drift2Phase = 0;
    double hPhase[2] = { 0.0, 0.0 };
    float hInt[2] = { 0.0f, 0.0f }, hIntTarget[2] = { 0.0f, 0.0f }, smHarm = 0.0f;

    // hamarlanmış parametrlər
    float smWarmth = 0, smAir = 0, smWidth = 0, smDelayMix = 0, smReverbMix = 0;
    float smGain = 1, smVib = 0, smDrift = 0, smFormant = 0, smDelay = 1000;
    bool paramsInit = false, wasBypassed = false;

    // pitch analiz
    int decim = 4, tauMin = 10, tauMax = 170;
    double fsD = 11025.0;
    std::vector<float> anaRing, anaLin, yinD;
    float decimAcc = 0.0f;
    int decimCount = 0, hopCount = 0, anaWrite = 0, anaFilled = 0;
    std::array<float, kMedian> medBuf{};
    int medIdx = 0, medCount = 0;
    float levelEnv = 0.0f;
    int unvoicedFrames = 100;
    bool voiced = false;

    // düzəltmə vəziyyəti
    float targetShiftCents = 0.0f, shiftSm = 0.0f;
    int transLeft = 0, transitionSamples = 0;
    bool havePrev = false, curTargetValid = false;
    float prevCm = 0.0f, curTargetAbs = 0.0f;
    LadType activeLad = LadType::Sur, lastLad = LadType::Sur;
    float activeRootCents = 0.0f, lastRoot = -1000.0f;

    // avtomatik lad/kök
    std::array<float, kKeyBins> hist{};
    bool keyReady = false;
    int framesSinceKey = 0, detRootBin = 0;
    LadType detLad = LadType::Sur;

    // telemetriya (UI thread ilə təhlükəsiz)
    std::atomic<float> tInputHz{0.0f}, tTargetHz{0.0f}, tComma{0.0f}, tRms{0.0f}, tConf{0.0f}, tShift{0.0f};
    std::atomic<int>   tLad{0}, tRootMidi{62};
    std::atomic<bool>  tVoiced{false};
};
