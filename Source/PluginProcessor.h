#pragma once
#include <JuceHeader.h>

namespace SPitchParams {
    inline constexpr const char* lad                = "lad";
    inline constexpr const char* rootNote           = "rootNote";
    inline constexpr const char* tuningHz           = "tuningHz";
    inline constexpr const char* vocalMode          = "vocalMode";
    inline constexpr const char* correctionStrength = "correctionStrength";
    inline constexpr const char* toleranceCents     = "toleranceCents";
    inline constexpr const char* vibratoDepth       = "vibratoDepth";
    inline constexpr const char* formantShift       = "formantShift";
    inline constexpr const char* reverbMix          = "reverbMix";
    inline constexpr const char* delayMix           = "delayMix";
    inline constexpr const char* outputGain         = "outputGain";
    inline constexpr const char* naturalMode        = "naturalMode";
    inline constexpr const char* aiPitchEnabled     = "aiPitchEnabled";
    inline constexpr const char* aiAnalyzerEnabled  = "aiAnalyzerEnabled";
    inline constexpr const char* bypass             = "bypass";

    inline juce::AudioProcessorValueTreeState::ParameterLayout createParameterLayout() {
        std::vector<std::unique_ptr<juce::RangedAudioParameter>> params;

        params.push_back(std::make_unique<juce::AudioParameterChoice>(
            lad, "Lad",
            juce::StringArray {
                "Şur", "Rast", "Segah", "Çahargah", "Bayatı-Şiraz",
                "Humayun", "Şüştər", "Nava", "Diğər", "Major", "Minor"
            },
            0
        ));

        params.push_back(std::make_unique<juce::AudioParameterChoice>(
            rootNote, "Key",
            juce::StringArray { "C", "C#", "D", "D#", "E", "F", "F#", "G", "G#", "A", "A#", "B" },
            0
        ));

        params.push_back(std::make_unique<juce::AudioParameterChoice>(
            tuningHz, "Tuning",
            juce::StringArray { "432 Hz", "440 Hz", "442 Hz" },
            1
        ));

        params.push_back(std::make_unique<juce::AudioParameterChoice>(
            vocalMode, "Vocal Mode",
            juce::StringArray { "Natural", "Mugham AI", "Modern Pop" },
            0
        ));

        params.push_back(std::make_unique<juce::AudioParameterFloat>(
            correctionStrength, "Strength",
            juce::NormalisableRange<float>(0.0f, 100.0f, 1.0f), 75.0f
        ));

        params.push_back(std::make_unique<juce::AudioParameterFloat>(
            toleranceCents, "Tolerance",
            juce::NormalisableRange<float>(5.0f, 60.0f, 1.0f), 15.0f
        ));

        params.push_back(std::make_unique<juce::AudioParameterFloat>(
            vibratoDepth, "Vibrato",
            juce::NormalisableRange<float>(0.0f, 100.0f, 1.0f), 65.0f
        ));

        params.push_back(std::make_unique<juce::AudioParameterFloat>(
            formantShift, "Formant",
            juce::NormalisableRange<float>(-12.0f, 12.0f, 0.1f), 0.0f
        ));

        params.push_back(std::make_unique<juce::AudioParameterFloat>(
            reverbMix, "Reverb",
            juce::NormalisableRange<float>(0.0f, 100.0f, 1.0f), 35.0f
        ));

        params.push_back(std::make_unique<juce::AudioParameterFloat>(
            delayMix, "Delay",
            juce::NormalisableRange<float>(0.0f, 100.0f, 1.0f), 25.0f
        ));

        params.push_back(std::make_unique<juce::AudioParameterFloat>(
            outputGain, "Master Gain",
            juce::NormalisableRange<float>(-24.0f, 18.0f, 0.5f), 0.0f
        ));

        params.push_back(std::make_unique<juce::AudioParameterBool>(
            naturalMode, "Natural Mode", true
        ));

        params.push_back(std::make_unique<juce::AudioParameterBool>(
            aiPitchEnabled, "AI Pitch", true
        ));

        params.push_back(std::make_unique<juce::AudioParameterBool>(
            aiAnalyzerEnabled, "AI Analyzer", true
        ));

        params.push_back(std::make_unique<juce::AudioParameterBool>(
            bypass, "Bypass", false
        ));

        return { params.begin(), params.end() };
    }
}
