#pragma once

#include <juce_audio_basics/juce_audio_basics.h>
#include <array>
#include "DspUtils.h"
#include "../Parameters.h"

namespace steso::dsp
{
// Värden som läses en gång per block och delas av alla röster.
struct BlockParams
{
    int osc1Wave = 0, osc2Wave = 0;
    float osc1Octave = 0, osc1PW = 0.5f;
    float osc2Semi = 0, osc2Fine = 0, osc2PW = 0.5f;
    float mix1 = 0.8f, mix2 = 0.6f, mixSub = 0, mixNoise = 0;
    float resonanceK = 0, drive = 0, envAmount = 0, keyTrack = 0, filterVel = 0;
    float lfoPitch = 0, lfoFilter = 0, lfoPW = 0, lfoFadeSeconds = 0;
    float ampVel = 0.5f, drift = 0, spread = 0;
    float glideSeconds = 0, bendRange = 2;

    // v1.2 "Fat"
    int osc3Wave = 0;
    float osc3Semi = -12, osc3Fine = -5, mix3 = 0;
    int filterType = 0;          // 0 ladder, 1 Polivoks LP, 2 Polivoks BP
    float resonance = 0;         // 0..1 (Polivoks)
    float feedback = 0, warmth = 0;
    int unison = 1;
    std::array<float, 4> uniRatios { 1, 1, 1, 1 };

    // v1.7 Undertoner: frekvens = oscillatorns / div, samma vågform som oscillatorn.
    int subDiv1 = 2, subDiv2 = 3;
    float mixSubh1 = 0, mixSubh2 = 0;
    int subDiv3 = 2;
    float mixSubh3 = 0;
};

class Voice
{
public:
    void prepare (float sampleRate, uint32_t seed);
    void reset();

    void start (int note, float velocity, float startPitch, bool retrigger, bool glideFromPrevious);
    void glideTo (int note);
    void release();
    void kill();

    bool isActive() const noexcept { return ampEnv.isActive(); }
    bool isHeld() const noexcept { return held; }
    int getNote() const noexcept { return note; }
    float getLevel() const noexcept { return ampEnv.level; }
    float getCurrentPitch() const noexcept { return pitch; }
    uint64_t getAge() const noexcept { return age; }
    void setAge (uint64_t a) noexcept { age = a; }
    void setHeld (bool h) noexcept { held = h; }
    void setPan (float p) noexcept { pan = p; }

    Envelope ampEnv, filterEnv;

    // Renderar i 2x-takt och lägger till i left2x/right2x (2 * numSamples sampel).
    // lfo/cutoffHz har ett värde per utsampel (1x).
    void render (float* left2x, float* right2x, int numSamples, const BlockParams& bp,
                 const float* lfo, const float* cutoffHz, float pitchBendSemis, float modWheel);

private:
    float sampleRate = 44100.0f;
    int note = 60;
    float velocity = 1.0f;
    float pitch = 60.0f, targetPitch = 60.0f;
    bool held = false;
    uint64_t age = 0;
    float pan = 0.0f;
    float lfoFade = 1.0f;

    std::array<Oscillator, 4> osc1, osc2, osc3;   // en per unison-stack
    Oscillator sub, subh1, subh2, subh3;
    LadderFilter ladder;
    PolivoksFilter polivoks;
    float feedbackState = 0.0f;
    FastRandom rng;

    // Analog drift: långsam slumpvandring per oscillator.
    float driftValue1 = 0, driftValue2 = 0, driftValue3 = 0;
    float driftTarget1 = 0, driftTarget2 = 0, driftTarget3 = 0, driftCutoff = 0;
    int driftCounter = 0;
};

class SynthEngine
{
public:
    static constexpr int maxVoices = 8;

    void prepare (double sampleRate, int maxBlockSize);
    void reset();

    // Lägger till numSamples sampel i left/right (ska vara nollställda). MIDI-händelser i
    // [midiOffset, midiOffset + numSamples) spelas sampel-exakt.
    void process (float* left, float* right, int numSamples, const juce::MidiBuffer& midi,
                  int midiOffset, const ParamPointers& params);

    int getActiveVoiceCount() const noexcept;

private:
    enum class Mode { poly = 0, mono, legato };

    void handleMidi (const juce::MidiMessage& m);
    void noteOn (int note, float velocity);
    void noteOff (int note);
    void releaseNote (int note);
    void removeFromStack (int note);
    void allNotesOff (bool hard);
    void renderSegment (float* left, float* right, int start, int numSamples);
    void updateBlockParams (const ParamPointers& p);
    Voice* findVoiceFor (int note);

    std::array<Voice, maxVoices> voices;
    BlockParams bp;
    Mode mode = Mode::poly;

    float sampleRate = 44100.0f;
    uint64_t voiceCounter = 0;
    float lastPitch = -1.0f;

    // Monoläge: stack med nedtryckta toner.
    std::array<int, 128> noteStack {};
    std::array<float, 128> noteVelocity {};
    int stackSize = 0;

    bool sustainPedal = false;
    std::array<bool, 128> sustained {};
    std::array<bool, 128> keyDown {};

    float pitchBend = 0.0f;      // -1..1
    float modWheel = 0.0f;       // 0..1

    // LFO (globalt)
    float lfoPhase = 0.0f, lfoRate = 5.0f, lfoSH = 0.0f;
    int lfoWave = 0;
    FastRandom lfoRng;

    juce::SmoothedValue<float, juce::ValueSmoothingTypes::Multiplicative> cutoffSmoothed;
    HalfbandDecimator decimatorL, decimatorR;
};
} // namespace steso::dsp
