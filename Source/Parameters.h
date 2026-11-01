#pragma once

#include <juce_audio_processors/juce_audio_processors.h>

namespace steso
{
// Parameter-ID:n. Ändra aldrig ett befintligt ID – då tappar sparade projekt sina värden.
namespace pid
{
    inline constexpr const char* osc1Wave   = "osc1Wave";
    inline constexpr const char* osc1Octave = "osc1Octave";
    inline constexpr const char* osc1PW     = "osc1PW";

    inline constexpr const char* osc2Wave   = "osc2Wave";
    inline constexpr const char* osc2Semi   = "osc2Semi";
    inline constexpr const char* osc2Fine   = "osc2Fine";
    inline constexpr const char* osc2PW     = "osc2PW";

    inline constexpr const char* mixOsc1    = "mixOsc1";
    inline constexpr const char* mixOsc2    = "mixOsc2";
    inline constexpr const char* mixSub     = "mixSub";
    inline constexpr const char* mixNoise   = "mixNoise";

    inline constexpr const char* cutoff     = "cutoff";
    inline constexpr const char* resonance  = "resonance";
    inline constexpr const char* drive      = "drive";
    inline constexpr const char* filterEnv  = "filterEnv";
    inline constexpr const char* keyTrack   = "keyTrack";
    inline constexpr const char* filterVel  = "filterVel";

    inline constexpr const char* fAttack    = "fAttack";
    inline constexpr const char* fDecay     = "fDecay";
    inline constexpr const char* fSustain   = "fSustain";
    inline constexpr const char* fRelease   = "fRelease";

    inline constexpr const char* aAttack    = "aAttack";
    inline constexpr const char* aDecay     = "aDecay";
    inline constexpr const char* aSustain   = "aSustain";
    inline constexpr const char* aRelease   = "aRelease";
    inline constexpr const char* ampVel     = "ampVel";

    inline constexpr const char* lfoWave    = "lfoWave";
    inline constexpr const char* lfoRate    = "lfoRate";
    inline constexpr const char* lfoFade    = "lfoFade";
    inline constexpr const char* lfoPitch   = "lfoPitch";
    inline constexpr const char* lfoFilter  = "lfoFilter";
    inline constexpr const char* lfoPW      = "lfoPW";

    inline constexpr const char* voiceMode  = "voiceMode";
    inline constexpr const char* glide      = "glide";
    inline constexpr const char* bendRange  = "bendRange";
    inline constexpr const char* drift      = "drift";
    inline constexpr const char* spread     = "spread";

    inline constexpr const char* chorus     = "chorus";
    inline constexpr const char* delayTime  = "delayTime";
    inline constexpr const char* delayFb    = "delayFb";
    inline constexpr const char* delayMix   = "delayMix";
    inline constexpr const char* reverbMix  = "reverbMix";
    inline constexpr const char* master     = "master";

    // v1.2 "Fat"
    inline constexpr const char* osc3Wave    = "osc3Wave";
    inline constexpr const char* osc3Semi    = "osc3Semi";
    inline constexpr const char* osc3Fine    = "osc3Fine";
    inline constexpr const char* mixOsc3     = "mixOsc3";
    inline constexpr const char* filterType  = "filterType";
    inline constexpr const char* feedback    = "feedback";
    inline constexpr const char* warmth      = "warmth";
    inline constexpr const char* unison      = "unison";
    inline constexpr const char* uniDetune   = "uniDetune";

    inline constexpr const char* seqMode     = "seqMode";
    inline constexpr const char* seqRate     = "seqRate";
    inline constexpr const char* seqSwing    = "seqSwing";
    inline constexpr const char* seqGate     = "seqGate";
    inline constexpr const char* arpOrder    = "arpOrder";
    inline constexpr const char* arpOctaves  = "arpOctaves";
    inline constexpr const char* seqLength   = "seqLength";
    inline constexpr const char* seqAccLen   = "seqAccLen";
    inline constexpr const char* seqGateLen  = "seqGateLen";
    inline constexpr const char* seqLatch    = "seqLatch";
    inline constexpr const char* seqMidiOut  = "seqMidiOut";

    // v1.7 Polyrytm. polyOn = Off som standard -> äldre projekt/presets påverkas inte.
    // v1.7 Undertoner (subharmoniska oscillatorer): nivå 0 som standard -> av.
    inline constexpr const char* osc1SubDiv  = "osc1SubDiv";
    inline constexpr const char* osc2SubDiv  = "osc2SubDiv";
    inline constexpr const char* mixSubh1    = "mixSubh1";
    inline constexpr const char* mixSubh2    = "mixSubh2";

    inline constexpr const char* polyOn      = "polyOn";
    inline constexpr const char* polyBars    = "polyBars";
    // Per spår 1..4 (t.ex. "polyPulses1").
    inline constexpr const char* polyOnIds[]     { "polyOn1", "polyOn2", "polyOn3", "polyOn4" };
    inline constexpr const char* polyTypeIds[]   { "polyType1", "polyType2", "polyType3", "polyType4" };
    inline constexpr const char* polyPulsesIds[] { "polyPulses1", "polyPulses2", "polyPulses3", "polyPulses4" };
    inline constexpr const char* polyHitsIds[]   { "polyHits1", "polyHits2", "polyHits3", "polyHits4" };
    inline constexpr const char* polyPitchIds[]  { "polyPitch1", "polyPitch2", "polyPitch3", "polyPitch4" };
    inline constexpr const char* polyStepsIds[]  { "polySteps1", "polySteps2", "polySteps3", "polySteps4" };
    inline const char* polyLaneOn (int lane)     { return polyOnIds[lane & 3]; }
    inline const char* polyLaneType (int lane)   { return polyTypeIds[lane & 3]; }
    inline const char* polyLanePulses (int lane) { return polyPulsesIds[lane & 3]; }
    inline const char* polyLaneHits (int lane)   { return polyHitsIds[lane & 3]; }
    inline const char* polyLanePitch (int lane)  { return polyPitchIds[lane & 3]; }
    inline const char* polyLaneSteps (int lane)  { return polyStepsIds[lane & 3]; }
} // namespace pid

inline const juce::StringArray waveNames { "Saw", "Square", "Triangle", "Sine" };
inline const juce::StringArray lfoWaveNames { "Sine", "Triangle", "Saw", "Square", "S&H" };
inline const juce::StringArray voiceModeNames { "Poly", "Mono", "Legato" };
inline const juce::StringArray chorusNames { "Off", "I", "II" };
inline const juce::StringArray filterTypeNames { "Ladder", "Steppe LP", "Steppe BP" };

juce::AudioProcessorValueTreeState::ParameterLayout createParameterLayout();

// Alla parametrar som motorn läser, som råa pekare (snabbt och trådsäkert).
struct ParamPointers
{
    std::atomic<float>* osc1Wave {}; std::atomic<float>* osc1Octave {}; std::atomic<float>* osc1PW {};
    std::atomic<float>* osc2Wave {}; std::atomic<float>* osc2Semi {}; std::atomic<float>* osc2Fine {}; std::atomic<float>* osc2PW {};
    std::atomic<float>* mixOsc1 {}; std::atomic<float>* mixOsc2 {}; std::atomic<float>* mixSub {}; std::atomic<float>* mixNoise {};
    std::atomic<float>* cutoff {}; std::atomic<float>* resonance {}; std::atomic<float>* drive {};
    std::atomic<float>* filterEnv {}; std::atomic<float>* keyTrack {}; std::atomic<float>* filterVel {};
    std::atomic<float>* fAttack {}; std::atomic<float>* fDecay {}; std::atomic<float>* fSustain {}; std::atomic<float>* fRelease {};
    std::atomic<float>* aAttack {}; std::atomic<float>* aDecay {}; std::atomic<float>* aSustain {}; std::atomic<float>* aRelease {};
    std::atomic<float>* ampVel {};
    std::atomic<float>* lfoWave {}; std::atomic<float>* lfoRate {}; std::atomic<float>* lfoFade {};
    std::atomic<float>* lfoPitch {}; std::atomic<float>* lfoFilter {}; std::atomic<float>* lfoPW {};
    std::atomic<float>* voiceMode {}; std::atomic<float>* glide {}; std::atomic<float>* bendRange {};
    std::atomic<float>* drift {}; std::atomic<float>* spread {};
    std::atomic<float>* chorus {}; std::atomic<float>* delayTime {}; std::atomic<float>* delayFb {};
    std::atomic<float>* delayMix {}; std::atomic<float>* reverbMix {}; std::atomic<float>* master {};
    std::atomic<float>* seqMode {}; std::atomic<float>* seqRate {}; std::atomic<float>* seqSwing {};
    std::atomic<float>* seqGate {}; std::atomic<float>* arpOrder {}; std::atomic<float>* arpOctaves {};
    std::atomic<float>* seqLength {}; std::atomic<float>* seqAccLen {}; std::atomic<float>* seqGateLen {};
    std::atomic<float>* seqLatch {}; std::atomic<float>* seqMidiOut {};
    std::atomic<float>* osc3Wave {}; std::atomic<float>* osc3Semi {}; std::atomic<float>* osc3Fine {};
    std::atomic<float>* mixOsc3 {}; std::atomic<float>* filterType {}; std::atomic<float>* feedback {};
    std::atomic<float>* warmth {}; std::atomic<float>* unison {}; std::atomic<float>* uniDetune {};
    std::atomic<float>* polyOn {}; std::atomic<float>* polyBars {};
    std::atomic<float>* osc1SubDiv {}; std::atomic<float>* osc2SubDiv {};
    std::atomic<float>* mixSubh1 {}; std::atomic<float>* mixSubh2 {};
    struct PolyLane
    {
        std::atomic<float>* on {}; std::atomic<float>* type {}; std::atomic<float>* pulses {};
        std::atomic<float>* hits {}; std::atomic<float>* pitch {}; std::atomic<float>* steps {};
    };
    std::array<PolyLane, 4> polyLane {};

    void bind (juce::AudioProcessorValueTreeState& apvts);
};
} // namespace steso
