#include "PluginProcessor.h"
#include "PluginEditor.h"

SPitchProcessor::SPitchProcessor()
    : AudioProcessor(BusesProperties()
                         .withInput("Input", juce::AudioChannelSet::stereo(), true)
                         .withOutput("Output", juce::AudioChannelSet::stereo(), true)),
      apvts(*this, &undoManager, "SPitchState", SPitchParams::createLayout())
{
}

void SPitchProcessor::prepareToPlay(double sampleRate, int samplesPerBlock)
{
    currentSampleRate = sampleRate;
    engine.prepareToPlay(sampleRate, samplesPerBlock);
    const int lat = engine.getLatencySamples();
    setLatencySamples(lat);
    latencyMs.store(1000.0f * static_cast<float>(lat) / static_cast<float>(sampleRate));
}

bool SPitchProcessor::isBusesLayoutSupported(const BusesLayout& layouts) const
{
    const auto& out = layouts.getMainOutputChannelSet();
    if (out != juce::AudioChannelSet::mono() && out != juce::AudioChannelSet::stereo()) return false;
    return out == layouts.getMainInputChannelSet();
}

void SPitchProcessor::processBlock(juce::AudioBuffer<float>& buffer, juce::MidiBuffer&)
{
    juce::ScopedNoDenormals noDenormals;
    const auto t0 = juce::Time::getHighResolutionTicks();

    for (int c = getTotalNumInputChannels(); c < getTotalNumOutputChannels(); ++c)
        buffer.clear(c, 0, buffer.getNumSamples());

    if (resetRequested.exchange(false)) engine.reset();

    double bpm = 120.0;
    if (auto* ph = getPlayHead())
        if (auto pos = ph->getPosition())
            if (auto b = pos->getBpm()) bpm = *b;

    engine.processBlock(buffer, SPitchParams::makeEngineParams(apvts, bpm));

    const float pk = buffer.getMagnitude(0, buffer.getNumSamples());
    outLevel.store(std::max(pk, outLevel.load() * 0.92f));

    const double secs = juce::Time::highResolutionTicksToSeconds(juce::Time::getHighResolutionTicks() - t0);
    const double budget = static_cast<double>(buffer.getNumSamples()) / currentSampleRate;
    cpuLoad.store(0.9f * cpuLoad.load() + 0.1f * static_cast<float>(secs / budget));
}

juce::AudioProcessorEditor* SPitchProcessor::createEditor() { return new SPitchEditor(*this); }

void SPitchProcessor::getStateInformation(juce::MemoryBlock& destData)
{
    if (auto xml = apvts.copyState().createXml()) copyXmlToBinary(*xml, destData);
}

void SPitchProcessor::setStateInformation(const void* data, int sizeInBytes)
{
    if (auto xml = getXmlFromBinary(data, sizeInBytes))
        if (xml->hasTagName(apvts.state.getType())) apvts.replaceState(juce::ValueTree::fromXml(*xml));
}

juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter() { return new SPitchProcessor(); }
