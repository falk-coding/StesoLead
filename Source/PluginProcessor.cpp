#include "PluginProcessor.h"
#include "PluginEditor.h"

StesoLeadProcessor::StesoLeadProcessor()
    : AudioProcessor (BusesProperties().withOutput ("Output", juce::AudioChannelSet::stereo(), true)),
      apvts (*this, nullptr, "StesoLead", steso::createParameterLayout())
{
    params.bind (apvts);
    work.setSize (2, workSize);
}

bool StesoLeadProcessor::isBusesLayoutSupported (const BusesLayout& layouts) const
{
    const auto out = layouts.getMainOutputChannelSet();
    if (out != juce::AudioChannelSet::stereo() && out != juce::AudioChannelSet::mono())
        return false;
    return layouts.getMainInputChannelSet().isDisabled();
}

void StesoLeadProcessor::prepareToPlay (double sampleRate, int samplesPerBlock)
{
    engine.prepare (sampleRate, samplesPerBlock);
    chorus.prepare (sampleRate);
    delay.prepare (sampleRate);
    reverb.prepare (sampleRate);
    dcL.prepare ((float) sampleRate);
    dcR.prepare ((float) sampleRate);
    masterGain.reset (sampleRate, 0.05);
    masterGain.setCurrentAndTargetValue (juce::Decibels::decibelsToGain (params.master->load()));
    work.setSize (2, workSize, false, false, true);
    keyboardState.reset();
}

void StesoLeadProcessor::reset()
{
    engine.reset();
    chorus.reset();
    delay.reset();
    reverb.reset();
    dcL.reset();
    dcR.reset();
}

void StesoLeadProcessor::processBlock (juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midi)
{
    juce::ScopedNoDenormals noDenormals;
    const int numSamples = buffer.getNumSamples();
    const int numChannels = buffer.getNumChannels();

    // Lägg till toner från GUI-klaviaturen.
    keyboardState.processNextMidiBuffer (midi, 0, numSamples, true);

    masterGain.setTargetValue (juce::Decibels::decibelsToGain (params.master->load(), -60.0f));
    const int chorusMode = (int) params.chorus->load();
    const float delayTime = params.delayTime->load();
    const float delayFb = params.delayFb->load();
    const float delayMix = params.delayMix->load();
    const float reverbMix = params.reverbMix->load();

    for (int offset = 0; offset < numSamples; offset += workSize)
    {
        const int n = std::min (workSize, numSamples - offset);
        work.clear (0, n);
        float* l = work.getWritePointer (0);
        float* r = work.getWritePointer (1);

        engine.process (l, r, n, midi, offset, params);
        chorus.process (l, r, n, chorusMode);
        delay.process (l, r, n, delayTime, delayFb, delayMix);
        reverb.process (l, r, n, reverbMix);

        for (int i = 0; i < n; ++i)
        {
            const float g = masterGain.getNextValue();
            l[i] = steso::dsp::softClip (dcL.process (l[i]) * g);
            r[i] = steso::dsp::softClip (dcR.process (r[i]) * g);
        }

        if (numChannels == 1)
        {
            auto* out = buffer.getWritePointer (0, offset);
            for (int i = 0; i < n; ++i)
                out[i] = 0.5f * (l[i] + r[i]);
        }
        else if (numChannels >= 2)
        {
            buffer.copyFrom (0, offset, work, 0, 0, n);
            buffer.copyFrom (1, offset, work, 1, 0, n);
            for (int ch = 2; ch < numChannels; ++ch)
                buffer.clear (ch, offset, n);
        }
    }

    activeVoices.store (engine.getActiveVoiceCount());
    midi.clear();
}

juce::AudioProcessorEditor* StesoLeadProcessor::createEditor()
{
    return new StesoLeadEditor (*this);
}

//==============================================================================
int StesoLeadProcessor::getNumPrograms()
{
    return (int) steso::getFactoryPresets().size();
}

void StesoLeadProcessor::setCurrentProgram (int index)
{
    const auto& presets = steso::getFactoryPresets();
    if (! juce::isPositiveAndBelow (index, (int) presets.size()))
        return;

    currentProgram = index;
    steso::applyPreset (apvts, presets[(size_t) index]);
    ++programChangeCounter;
}

const juce::String StesoLeadProcessor::getProgramName (int index)
{
    const auto& presets = steso::getFactoryPresets();
    return juce::isPositiveAndBelow (index, (int) presets.size()) ? presets[(size_t) index].name : juce::String();
}

void StesoLeadProcessor::getStateInformation (juce::MemoryBlock& destData)
{
    auto state = apvts.copyState();
    state.setProperty ("program", currentProgram, nullptr);
    state.setProperty ("version", STESO_VERSION, nullptr);
    if (auto xml = state.createXml())
        copyXmlToBinary (*xml, destData);
}

void StesoLeadProcessor::setStateInformation (const void* data, int sizeInBytes)
{
    if (auto xml = getXmlFromBinary (data, sizeInBytes))
    {
        if (xml->hasTagName (apvts.state.getType()))
        {
            auto tree = juce::ValueTree::fromXml (*xml);
            currentProgram = juce::jlimit (0, getNumPrograms() - 1, (int) tree.getProperty ("program", 0));
            apvts.replaceState (tree);
            ++programChangeCounter;
        }
    }
}

juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new StesoLeadProcessor();
}
