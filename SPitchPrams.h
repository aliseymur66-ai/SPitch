#pragma once
#include <JuceHeader.h>
#include "SPitchEngine.h"

namespace SPitchParams
{
    inline juce::String U8(const char* s) { return juce::String::fromUTF8(s); }

    inline juce::StringArray ladNames()
    {
        juce::StringArray a;
        for (int i = 0; i < SPitchEngine::kNumLads; ++i)
            a.add(U8(SPitchEngine::ladToString(static_cast<SPitchEngine::LadType>(i))));
        return a;
    }

    inline constexpr float kTuning[]      = { 415.f, 430.f, 432.f, 435.f, 438.f, 440.f, 442.f, 444.f, 446.f, 450.f };
    inline constexpr float kVibRate[]     = { 4.5f, 5.0f, 5.5f, 5.8f, 6.0f, 6.5f, 7.0f };
    inline constexpr float kDelayFactor[] = { 1.0f, 0.5f, 0.75f, 0.25f, 2.0f };   // vurğu (beat) nisbəti

    inline juce::AudioProcessorValueTreeState::ParameterLayout createLayout()
    {
        using namespace juce;
        AudioProcessorValueTreeState::ParameterLayout L;

        auto F = [&](const char* id, const char* name, float a, float b, float def)
        { L.add(std::make_unique<AudioParameterFloat>(ParameterID { id, 1 }, name, NormalisableRange<float>(a, b), def)); };
        auto C = [&](const char* id, const char* name, StringArray items, int def)
        { L.add(std::make_unique<AudioParameterChoice>(ParameterID { id, 1 }, name, items, def)); };
        auto B = [&](const char* id, const char* name, bool def)
        { L.add(std::make_unique<AudioParameterBool>(ParameterID { id, 1 }, name, def)); };

        StringArray keys { "C", "C#", "D", "D#", "E", "F", "F#", "G", "G#", "A", "A#", "B" };
        StringArray tunings; for (float t : kTuning) tunings.add(String(static_cast<int>(t)) + " Hz");
        StringArray rates;   for (float t : kVibRate) rates.add(String(t, 1).replace(".", " ") + " Hz");
        StringArray shifts;  for (int i = 0; i < 13; ++i) { float v = -3.0f + 0.5f * i; shifts.add((v > 0 ? "+" : "") + String(v, 1)); }

        C("lad", "Lad", ladNames(), 0);
        C("mod", "Harmoniya tipi", { "Major", "Minor", "Lad" }, 0);
        C("mugam", "Muğam Ladı", ladNames(), 1);
        C("takrar", "Analiz rejimi", { "Takrar", "Kilid" }, 0);
        C("key", "Key", keys, 0);
        C("tuning", "Tuning", tunings, 5);

        F("strength", "Strength", 0.0f, 1.0f, 0.75f);
        F("tolerance", "Tolerance", 0.0f, 50.0f, 15.0f);
        F("speed", "Speed", 1.0f, 400.0f, 70.0f);
        F("transition", "Transition", 0.0f, 500.0f, 120.0f);
        B("natural", "Natural Mode", true);
        C("vocalMode", "Vocal Mode", { "Natural", "Soft", "Pop", "Hard" }, 0);

        F("harmonyMix", "Harmony", 0.0f, 1.0f, 0.6f);
        C("voices", "Voices", { "Off", "1", "2" }, 2);
        F("vibDepth", "Vibrato Depth", 0.0f, 1.0f, 0.65f);
        C("vibRate", "Vibrato Rate", rates, 3);
        F("formantAmt", "Formant", 0.0f, 1.0f, 0.7f);
        C("formantShift", "Formant Shift", shifts, 8);
        F("reverbMix", "Reverb Mix", 0.0f, 1.0f, 0.35f);
        C("reverbType", "Reverb Type", { "Room", "Hall", "Plate" }, 1);
        F("delayMix", "Delay Mix", 0.0f, 1.0f, 0.25f);
        C("delayDiv", "Delay Time", { "1/4", "1/8", "1/8.", "1/16", "1/2" }, 1);
        F("gainDb", "Master Gain", -24.0f, 12.0f, 1.5f);

        F("warmth", "Warmth", 0.0f, 1.0f, 0.35f);
        F("air", "Air", 0.0f, 1.0f, 0.65f);
        F("width", "Stereo Width", 0.0f, 1.0f, 0.55f);
        F("drift", "Pitch Drift", 0.0f, 1.0f, 0.30f);

        B("aiPitch", "AI Pitch", true);
        B("aiAnalyzer", "Milli AI Analiz", true);
        B("autoRoot", "Auto Root", true);
        B("bypass", "Bypass", false);
        return L;
    }

    inline SPitchEngine::EngineParams makeEngineParams(juce::AudioProcessorValueTreeState& s, double bpm)
    {
        auto f = [&](const char* id) { return s.getRawParameterValue(id)->load(); };
        auto idx = [&](const char* id) { return static_cast<int>(f(id) + 0.5f); };

        SPitchEngine::EngineParams p;
        p.lad = static_cast<SPitchEngine::LadType>(idx("lad"));
        p.mugamHint = static_cast<SPitchEngine::LadType>(idx("mugam"));
        p.analyzerLock = idx("takrar") == 1;
        p.rootMidiNote = 60 + idx("key");
        p.tuningHz = kTuning[std::clamp(idx("tuning"), 0, 9)];

        p.correctionStrength = f("strength");
        p.toleranceCents = f("tolerance");
        p.speedMs = f("speed");
        p.transitionMs = f("transition");
        p.naturalMode = f("natural") > 0.5f;

        switch (idx("vocalMode")) {
            case 1: p.correctionStrength *= 0.7f; p.speedMs *= 1.4f; break;                                   // Soft
            case 2: p.correctionStrength = std::max(p.correctionStrength, 0.9f); p.speedMs *= 0.6f;
                    p.toleranceCents *= 0.7f; break;                                                           // Pop
            case 3: p.correctionStrength = 1.0f; p.speedMs = 8.0f; p.transitionMs = 10.0f;
                    p.toleranceCents = 0.0f; p.naturalMode = false; break;                                    // Hard
            default: break;
        }

        p.harmonyMix = f("harmonyMix");
        p.harmonyVoices = std::clamp(idx("voices"), 0, 2);
        p.harmonyType = std::clamp(idx("mod"), 0, 2);
        p.vibratoDepth = f("vibDepth");
        p.vibratoRateHz = kVibRate[std::clamp(idx("vibRate"), 0, 6)];
        p.formantAmount = f("formantAmt");
        p.formantShiftSt = -3.0f + 0.5f * static_cast<float>(idx("formantShift"));
        p.reverbMix = f("reverbMix");
        p.reverbType = idx("reverbType");
        p.delayMix = f("delayMix");
        const float beatMs = 60000.0f / static_cast<float>(std::clamp(bpm, 30.0, 300.0));
        p.delayMs = std::clamp(beatMs * kDelayFactor[std::clamp(idx("delayDiv"), 0, 4)], 20.0f, 900.0f);
        p.outputGainDb = f("gainDb");

        p.warmth = f("warmth");
        p.air4K = f("air");
        p.stereoWidth = f("width");
        p.pitchDrift = f("drift");

        p.aiPitchEnabled = f("aiPitch") > 0.5f;
        p.aiAnalyzerEnabled = f("aiAnalyzer") > 0.5f;
        p.autoRoot = f("autoRoot") > 0.5f;
        p.bypassed = f("bypass") > 0.5f;
        return p;
    }

    // ------------------------------------------------------------------ presetlər
    struct Preset { const char* name; std::vector<std::pair<const char*, float>> v; };

    inline const std::vector<Preset>& factoryPresets()
    {
        static const std::vector<Preset> presets = {
            { "Azerbaijan Muğam Style", { { "lad", 0 }, { "strength", 0.75f }, { "tolerance", 15 }, { "speed", 70 }, { "transition", 120 },
                                          { "harmonyMix", 0.6f }, { "voices", 2 }, { "vibDepth", 0.65f }, { "reverbMix", 0.35f }, { "delayMix", 0.25f } } },
            { "Natural Vocal",          { { "lad", 9 }, { "strength", 0.45f }, { "tolerance", 25 }, { "speed", 90 }, { "transition", 150 },
                                          { "voices", 0 }, { "vibDepth", 0.5f }, { "reverbMix", 0.2f }, { "delayMix", 0.1f } } },
            { "Hard Tune",              { { "strength", 1.0f }, { "tolerance", 5 }, { "speed", 8 }, { "transition", 10 }, { "natural", 0 },
                                          { "vocalMode", 3 }, { "voices", 0 }, { "vibDepth", 0.1f } } },
            { "Rast Classic",           { { "lad", 1 }, { "mugam", 1 }, { "strength", 0.7f }, { "speed", 75 }, { "voices", 1 } } },
            { "Segah Lyrical",          { { "lad", 2 }, { "mugam", 2 }, { "strength", 0.65f }, { "speed", 80 }, { "transition", 160 },
                                          { "reverbMix", 0.45f }, { "delayMix", 0.3f }, { "voices", 1 } } },
            { "Bayatı-Şiraz Soulful",   { { "lad", 4 }, { "mugam", 4 }, { "strength", 0.7f }, { "vibDepth", 0.75f }, { "reverbMix", 0.4f } } },
            { "Çahargah Bright",        { { "lad", 3 }, { "mugam", 3 }, { "strength", 0.8f }, { "air", 0.8f }, { "formantShift", 9 } } },
        };
        return presets;
    }

    inline void setReal(juce::AudioProcessorValueTreeState& s, const char* id, float v)
    {
        if (auto* p = s.getParameter(id)) p->setValueNotifyingHost(p->convertTo0to1(v));
    }

    inline void applyFactoryPreset(juce::AudioProcessor& proc, juce::AudioProcessorValueTreeState& s,
                                   juce::UndoManager& undo, int index)
    {
        const auto& all = factoryPresets();
        if (index < 0 || index >= static_cast<int>(all.size())) return;
        undo.beginNewTransaction();
        for (auto* prm : proc.getParameters())
            if (auto* rp = dynamic_cast<juce::RangedAudioParameter*>(prm))
                if (rp->paramID != "bypass") rp->setValueNotifyingHost(rp->getDefaultValue());
        for (auto& kv : all[static_cast<size_t>(index)].v) setReal(s, kv.first, kv.second);
    }

    inline juce::File userPresetDir()
    {
        auto d = juce::File::getSpecialLocation(juce::File::userApplicationDataDirectory).getChildFile("SPitch").getChildFile("Presets");
        d.createDirectory();
        return d;
    }
}
