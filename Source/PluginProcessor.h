#pragma once

#include <juce_audio_processors/juce_audio_processors.h>
#include "Parameters.h"
#include "Presets.h"
#include "dsp/SynthEngine.h"
#include "dsp/Effects.h"

class StesoLeadProcessor : public juce::AudioProcessor
{
public:
    StesoLeadProcessor();
    ~StesoLeadProcessor() override = default;

    void prepareToPlay (double sampleRate, int samplesPerBlock) override;
    void releaseResources() override {}
    void reset() override;
    bool isBusesLayoutSupported (const BusesLayout& layouts) const override;
    void processBlock (juce::AudioBuffer<float>&, juce::MidiBuffer&) override;
    using AudioProcessor::processBlock;

    juce::AudioProcessorEditor* createEditor() override;
    bool hasEditor() const override { return true; }

    const juce::String getName() const override { return "StesoLead"; }
    bool acceptsMidi() const override { return true; }
    bool producesMidi() const override { return false; }
    bool isMidiEffect() const override { return false; }
    double getTailLengthSeconds() const override { return 3.0; }

    int getNumPrograms() override;
    int getCurrentProgram() override { return currentProgram; }
    void setCurrentProgram (int index) override;
    const juce::String getProgramName (int index) override;
    void changeProgramName (int, const juce::String&) override {}

    void getStateInformation (juce::MemoryBlock& destData) override;
    void setStateInformation (const void* data, int sizeInBytes) override;

    juce::AudioProcessorValueTreeState apvts;

    // Lås-fritt klaviatur-tillstånd som GUI:t visar och spelar på.
    juce::MidiKeyboardState keyboardState;

    int getActiveVoiceCount() const noexcept { return activeVoices.load(); }
    // Bumpas när en preset laddas, så att GUI:t kan uppdatera presetnamnet.
    std::atomic<int> programChangeCounter { 0 };

private:
    steso::ParamPointers params;
    steso::dsp::SynthEngine engine;
    steso::dsp::Chorus chorus;
    steso::dsp::StereoDelay delay;
    steso::dsp::Reverb reverb;
    steso::dsp::DcBlocker dcL, dcR;
    juce::SmoothedValue<float> masterGain;

    static constexpr int workSize = 512;
    juce::AudioBuffer<float> work;
    std::atomic<int> activeVoices { 0 };
    int currentProgram = 0;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (StesoLeadProcessor)
};
