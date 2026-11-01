#pragma once

#include <juce_audio_processors/juce_audio_processors.h>
#include "Parameters.h"
#include "Presets.h"
#include "dsp/SynthEngine.h"
#include "dsp/Effects.h"
#include "seq/Sequencer.h"

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

    const juce::String getName() const override { return "Bakelit"; }
    bool acceptsMidi() const override { return true; }
    bool producesMidi() const override { return true; }
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
    // Bumpas när en preset eller ett sparat läge laddas, så att GUI:t kan uppdatera sig.
    std::atomic<int> programChangeCounter { 0 };

    //==============================================================================
    // Sekvenser
    steso::seq::Pattern& getPattern() noexcept { return pattern; }
    steso::seq::Settings currentSeqSettings() const;

    // Info från DAW:en (för GUI:t).
    struct HostInfo
    {
        bool hasLoop = false;
        double loopStart = 0, loopEnd = 0;
        int timeSigNum = 4, timeSigDen = 4;
        double bpm = 120;
    };
    HostInfo getHostInfo() const;

    // Aktuellt globalt stegnummer (för spelhuvudet i GUI:t), -1 = står still.
    int64_t getCurrentStep() const noexcept { return currentStep.load(); }
    // Aktuell position i PPQ (för Polyrytm-ringarna), < 0 = står still.
    double getCurrentPpq() const noexcept { return currentPpq.load(); }

    // Skapar en MIDI-fil för exportområdet [fromPpq, toPpq). Returnerar en temporär fil.
    juce::File writeMidiExport (double fromPpq, double toPpq, const juce::String& fileName) const;
    // Ackordet/grundtonen som exporten använder (senast spelade).
    steso::seq::Chord getExportChord() const;

    // Efter en dragning till DAW:en: stäng av sekvensern så att det inte låter dubbelt.
    void sequencerExported();
    void reenableSequencer();
    bool isBypassedByExport() const noexcept { return bypassedByExport.load(); }

    //==============================================================================
    // Egna förinställningar (filer i Dokument/Falk Instruments/Bakelit/Presets, filändelse .bakelit;
    // äldre .stesolead-filer läses också)
    static juce::File getUserPresetFolder();
    static void setUserPresetFolder (const juce::File& folder);   // för tester
    static juce::Array<juce::File> getUserPresetFiles();
    static inline const juce::String userPresetExtension { ".bakelit" };
    static inline const juce::String legacyPresetExtension { ".stesolead" };

    // Sparar aktuella inställningar + mönster. Skriver över om namnet finns.
    juce::File saveUserPreset (const juce::String& name);
    bool loadUserPreset (const juce::File& file);
    // Namn på laddat eget preset, tomt om ett fabrikspreset gäller.
    juce::String getUserPresetName() const { return userPresetName; }
    void forgetUserPreset() { userPresetName = {}; ++programChangeCounter; }

private:
    void runSequencer (int numSamples, juce::MidiBuffer& engineMidi, juce::MidiBuffer& seqOut);
    void flushSequencer (juce::MidiBuffer& engineMidi, juce::MidiBuffer& seqOut);
    void notePressed (int note);
    void noteReleased (int note);
    // 0 = av, 1 = arp, 2 = seq, 3 = Polyrytm (polyOn går före seqMode).
    int effectiveSeqMode() const noexcept;

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
    double sampleRate = 44100.0;

    // Sekvenserns tillstånd (ljudtråden)
    steso::seq::Pattern pattern;
    steso::seq::NoteStream noteStream;
    steso::seq::Chord physical, latched;
    std::vector<steso::seq::NoteEvent> genEvents;
    std::vector<steso::seq::NoteStream::Out> outEvents;
    juce::MidiBuffer engineMidi, seqOutMidi;
    int lastSeqMode = 0;
    bool hostWasPlaying = false, freeRunning = false;
    double expectedNextPpq = 0, freePpq = 0;
    bool hostPlaying = false;
    juce::Optional<double> hostPpq;
    double hostBpm = 120.0;

    std::atomic<int64_t> currentStep { -1 };
    std::atomic<double> currentPpq { -1.0 };
    std::atomic<bool> hostHasLoop { false };
    std::atomic<double> hostLoopStart { 0 }, hostLoopEnd { 0 }, hostBpmAtomic { 120 };
    std::atomic<int> hostTsNum { 4 }, hostTsDen { 4 };
    std::array<std::atomic<int>, steso::seq::maxHeld> exportNotes {};
    std::atomic<int> exportCount { 0 };

    std::atomic<bool> bypassedByExport { false };
    juce::String userPresetName;
    int modeBeforeExport = 2;
    bool polyBeforeExport = false;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (StesoLeadProcessor)
};
