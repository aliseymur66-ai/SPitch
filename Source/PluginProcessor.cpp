#include "PluginProcessor.h"
#include "PluginEditor.h"

SPitchAudioProcessor::SPitchAudioProcessor()
    : AudioProcessor(BusesProperties()
          .withInput("Input", juce::AudioChannelSet::stereo(), true)
          .withOutput("Output", juce::AudioChannelSet::stereo(), true)),
      apvts(*this, nullptr, "SPITCH_PARAMS", createParameterLayout()) {
}

SPitchAudioProcessor::~SPitchAudioProcessor() = default;

juce::AudioProcessorValueTreeState::ParameterLayout SPitchAudioProcessor::createParameterLayout() {
    std::vector<std::unique_ptr<juce::RangedAudioParameter>> params;

    juce::StringArray ladChoices {
        "Şur", "Rast", "Segah", "Çahargah", "Bayatı-Şiraz",
        "Humayun", "Şüştər", "Nava", "Diğər", "Major", "Minor"
    };

    params.push_back(std::make_unique<juce::AudioParameterChoice>(
        "lad", "Lad Seçimi", ladChoices, 0));

    params.push_back(std::make_unique<juce::AudioParameterInt>(
        "rootMidi", "Mayə Notu", 48, 84, 62));

    params.push_back(std::make_unique<juce::AudioParameterFloat>(
        "strength", "Correction Strength", juce::NormalisableRange<float>(0.0f, 1.0f, 0.01f), 0.85f));

    params.push_back(std::make_unique<juce::AudioParameterFloat>(
        "tolerance", "Tolerance (ct)", juce::NormalisableRange<float>(5.0f, 50.0f, 1.0f), 15.0f));

    params.push_back(std::make_unique<juce::AudioParameterFloat>(
        "speedMs", "Speed (ms)", juce::NormalisableRange<float>(10.0f, 200.0f, 1.0f), 50.0f));

    params.push_back(std::make_unique<juce::AudioParameterFloat>(
        "transitionMs", "Transition (ms)", juce::NormalisableRange<float>(20.0f, 300.0f, 1.0f), 120.0f));

    params.push_back(std::make_unique<juce::AudioParameterFloat>(
        "vibrato", "Vibrato", juce::NormalisableRange<float>(0.0f, 1.0f, 0.01f), 0.60f));

    params.push_back(std::make_unique<juce::AudioParameterFloat>(
        "formant", "Formant (st)", juce::NormalisableRange<float>(-12.0f, 12.0f, 0.1f), 1.0f));

    params.push_back(std::make_unique<juce::AudioParameterFloat>(
        "drift", "Pitch Drift", juce::NormalisableRange<float>(0.0f, 1.0f, 0.01f), 0.30f));

    params.push_back(std::make_unique<juce::AudioParameterFloat>(
        "air4k", "4K Air", juce::NormalisableRange<float>(0.0f, 1.0f, 0.01f), 0.65f));

    params.push_back(std::make_unique<juce::AudioParameterFloat>(
        "warmth", "Warmth", juce::NormalisableRange<float>(0.0f, 1.0f, 0.01f), 0.35f));

    params.push_back(std::make_unique<juce::AudioParameterFloat>(
        "stereoWidth", "Stereo Width", juce::NormalisableRange<float>(0.0f, 1.0f, 0.01f), 0.55f));

    params.push_back(std::make_unique<juce::AudioParameterFloat>(
        "delayMix", "Delay Mix", juce::NormalisableRange<float>(0.0f, 1.0f, 0.01f), 0.25f));

    params.push_back(std::make_unique<juce::AudioParameterFloat>(
        "reverbMix", "Reverb Mix", juce::NormalisableRange<float>(0.0f, 1.0f, 0.01f), 0.40f));

    params.push_back(std::make_unique<juce::AudioParameterFloat>(
        "outputGainDb", "Output Gain (dB)", juce::NormalisableRange<float>(-12.0f, 12.0f, 0.1f), 1.5f));

    params.push_back(std::make_unique<juce::AudioParameterBool>(
        "naturalMode", "Natural Mode", true));

    params.push_back(std::make_unique<juce::AudioParameterBool>(
        "aiPitch", "AI Pitch", true));

    params.push_back(std::make_unique<juce::AudioParameterBool>(
        "aiAnalyzer", "AI Analyzer", true));

    params.push_back(std::make_unique<juce::AudioParameterBool>(
        "bypass", "Bypass", false));

    return { params.begin(), params.end() };
}

SPitchEngine::EngineParams SPitchAudioProcessor::collectEngineParams() const noexcept {
    SPitchEngine::EngineParams p;

    const int ladIdx = static_cast<int>(apvts.getRawParameterValue("lad")->load());
    p.lad = static_cast<SPitchEngine::LadType>(juce::jlimit(0, 10, ladIdx));
    p.rootMidiNote = static_cast<int>(apvts.getRawParameterValue("rootMidi")->load());
    p.correctionStrength = apvts.getRawParameterValue("strength")->load();
    p.toleranceCents = apvts.getRawParameterValue("tolerance")->load();
    p.speedMs = apvts.getRawParameterValue("speedMs")->load();
    p.transitionMs = apvts.getRawParameterValue("transitionMs")->load();
    p.vibratoDepth = apvts.getRawParameterValue("vibrato")->load();
    p.formantShiftSt = apvts.getRawParameterValue("formant")->load();
    p.pitchDrift = apvts.getRawParameterValue("drift")->load();
    p.air4K = apvts.getRawParameterValue("air4k")->load();
    p.warmth = apvts.getRawParameterValue("warmth")->load();
    p.stereoWidth = apvts.getRawParameterValue("stereoWidth")->load();
    p.delayMix = apvts.getRawParameterValue("delayMix")->load();
    p.reverbMix = apvts.getRawParameterValue("reverbMix")->load();
    p.outputGainDb = apvts.getRawParameterValue("outputGainDb")->load();
    p.naturalMode = apvts.getRawParameterValue("naturalMode")->load() > 0.5f;
    p.aiPitchEnabled = apvts.getRawParameterValue("aiPitch")->load() > 0.5f;
    p.aiAnalyzerEnabled = apvts.getRawParameterValue("aiAnalyzer")->load() > 0.5f;
    p.bypassed = apvts.getRawParameterValue("bypass")->load() > 0.5f;

    return p;
}

void SPitchAudioProcessor::prepareToPlay(double sampleRate, int samplesPerBlock) {
    pitchEngine.prepareToPlay(sampleRate, samplesPerBlock);
}

void SPitchAudioProcessor::releaseResources() {
}

bool SPitchAudioProcessor::isBusesLayoutSupported(const BusesLayout& layouts) const {
    if (layouts.getMainOutputChannelSet() != juce::AudioChannelSet::mono()
        && layouts.getMainOutputChannelSet() != juce::AudioChannelSet::stereo()) {
        return false;
    }

    if (layouts.getMainInputChannelSet() != layouts.getMainOutputChannelSet()) {
        return false;
    }

    return true;
}

void SPitchAudioProcessor::processBlock(juce::AudioBuffer<float>& buffer, juce::MidiBuffer&) {
    juce::ScopedNoDenormals noDenormals;

    const int totalNumInputChannels = getTotalNumInputChannels();
    const int totalNumOutputChannels = getTotalNumOutputChannels();

    for (int i = totalNumInputChannels; i < totalNumOutputChannels; ++i) {
        buffer.clear(i, 0, buffer.getNumSamples());
    }

    pitchEngine.processBlock(buffer, collectEngineParams());
}

juce::AudioProcessorEditor* SPitchAudioProcessor::createEditor() {
    return new SPitchAudioProcessorEditor(*this);
}

void SPitchAudioProcessor::getStateInformation(juce::MemoryBlock& destData) {
    auto state = apvts.copyState();
    std::unique_ptr<juce::XmlElement> xml(state.createXml());
    copyXmlToBinary(*xml, destData);
}

void SPitchAudioProcessor::setStateInformation(const void* data, int sizeInBytes) {
    std::unique_ptr<juce::XmlElement> xmlState(getXmlFromBinary(data, sizeInBytes));
    if (xmlState != nullptr && xmlState->hasTagName(apvts.state.getType())) {
        apvts.replaceState(juce::ValueTree::fromXml(*xmlState));
    }
}

juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter() {
    return new SPitchAudioProcessor();
}
