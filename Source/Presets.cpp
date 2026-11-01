#include "Presets.h"
#include "Parameters.h"

namespace steso
{
namespace
{
    // Index för valparametrar
    constexpr float SAW = 0, SQUARE = 1, TRI = 2, SINE = 3;
    constexpr float L_SINE = 0, L_TRI = 1, L_SQUARE = 3, L_SH = 4;
    constexpr float POLY = 0, MONO = 1, LEGATO = 2;
    constexpr float CH_I = 1, CH_II = 2;
    constexpr float S_ARP = 1, S_SEQ = 2;
    constexpr float R_8T = 2, R_16 = 4;
    constexpr float A_UPDOWN = 2;
    constexpr float LADDER = 0, POLI_LP = 1, POLI_BP = 2;
    constexpr float ON = 1, DIV = 1;

    // Polyrytm-spår: { på, typ (0 = Bar, 1 = Div), pulser, hits, tonhöjd, steg }
    struct LaneDef { float on, type, pulses, hits, pitch, steps; };

    std::vector<std::pair<const char*, float>> withPoly (std::vector<std::pair<const char*, float>> values,
                                                         std::initializer_list<LaneDef> lanes, float bars = 1)
    {
        values.push_back ({ pid::polyOn, ON });
        values.push_back ({ pid::polyBars, bars });
        int l = 0;
        for (const auto& d : lanes)
        {
            values.push_back ({ pid::polyLaneOn (l), d.on });
            values.push_back ({ pid::polyLaneType (l), d.type });
            values.push_back ({ pid::polyLanePulses (l), d.pulses });
            values.push_back ({ pid::polyLaneHits (l), d.hits });
            values.push_back ({ pid::polyLanePitch (l), d.pitch });
            values.push_back ({ pid::polyLaneSteps (l), d.steps });
            ++l;
        }
        for (; l < 4; ++l)
            values.push_back ({ pid::polyLaneOn (l), 0 });
        return values;
    }
}

const std::vector<Preset>& getFactoryPresets()
{
    using namespace pid;

    static const std::vector<Preset> presets {
        { "Init Poly", "Basic", {} },

        { "Steso Lead", "Lead", {
            { voiceMode, LEGATO }, { osc2Wave, SQUARE }, { osc2Fine, 9 }, { mixOsc2, 0.55f }, { mixSub, 0.3f },
            { cutoff, 1800 }, { resonance, 0.35f }, { drive, 0.45f }, { filterEnv, 0.45f },
            { fDecay, 0.4f }, { fSustain, 0.45f }, { aAttack, 0.002f }, { aSustain, 0.9f }, { aRelease, 0.25f },
            { glide, 0.06f }, { lfoPitch, 0.25f }, { lfoFade, 0.6f }, { lfoRate, 5.5f },
            { delayMix, 0.25f }, { delayTime, 0.375f }, { delayFb, 0.35f }, { reverbMix, 0.15f }, { spread, 0.0f } } },

        { "Classic Mono Lead", "Lead", {
            { voiceMode, MONO }, { osc2Wave, SQUARE }, { osc2Semi, -12 }, { osc2Fine, 4 }, { mixOsc2, 0.7f },
            { cutoff, 1200 }, { resonance, 0.55f }, { drive, 0.5f }, { filterEnv, 0.55f },
            { fDecay, 0.35f }, { fSustain, 0.25f }, { aSustain, 1.0f }, { aRelease, 0.15f },
            { glide, 0.04f }, { spread, 0.0f } } },

        { "Screaming Reso", "Lead", {
            { voiceMode, LEGATO }, { mixOsc2, 0.4f }, { osc2Fine, 12 },
            { cutoff, 900 }, { resonance, 0.78f }, { drive, 0.6f }, { filterEnv, 0.6f }, { fDecay, 0.8f }, { fSustain, 0.4f },
            { lfoFilter, 0.15f }, { lfoRate, 6.0f }, { lfoFade, 0.4f }, { glide, 0.08f },
            { aSustain, 0.95f }, { delayMix, 0.3f }, { delayFb, 0.4f }, { reverbMix, 0.2f }, { spread, 0.0f } } },

        { "Square Lead", "Lead", {
            { voiceMode, MONO }, { osc1Wave, SQUARE }, { osc2Wave, SQUARE }, { osc2Semi, 12 }, { mixOsc2, 0.35f },
            { cutoff, 3000 }, { resonance, 0.15f }, { filterEnv, 0.2f },
            { lfoPitch, 0.2f }, { lfoFade, 0.8f }, { glide, 0.03f }, { aSustain, 1.0f },
            { delayMix, 0.2f }, { reverbMix, 0.15f }, { spread, 0.0f } } },

        { "Soft Sine Lead", "Lead", {
            { voiceMode, LEGATO }, { osc1Wave, TRI }, { osc2Wave, SINE }, { osc2Semi, 12 }, { osc2Fine, 0 }, { mixOsc2, 0.3f },
            { cutoff, 4000 }, { resonance, 0.1f }, { filterEnv, 0.1f }, { drive, 0.1f },
            { aAttack, 0.02f }, { aSustain, 1.0f }, { aRelease, 0.4f },
            { glide, 0.12f }, { lfoPitch, 0.3f }, { lfoFade, 0.7f }, { lfoRate, 5.0f },
            { reverbMix, 0.3f }, { delayMix, 0.2f }, { spread, 0.0f } } },

        { "Chorus Strings", "Pad", {
            { osc2Fine, 12 }, { mixOsc2, 0.5f }, { cutoff, 3200 }, { resonance, 0.1f }, { filterEnv, 0.15f },
            { fAttack, 0.6f }, { fDecay, 1.5f }, { fSustain, 0.6f }, { fRelease, 1.2f },
            { aAttack, 0.5f }, { aDecay, 1.0f }, { aSustain, 0.85f }, { aRelease, 1.2f }, { ampVel, 0.2f },
            { chorus, CH_II }, { reverbMix, 0.3f }, { spread, 0.6f } } },

        { "Warm Pad", "Pad", {
            { osc2Wave, SQUARE }, { osc2PW, 0.35f }, { osc2Fine, 5 }, { lfoPW, 0.3f }, { lfoRate, 0.6f },
            { cutoff, 1200 }, { resonance, 0.2f }, { filterEnv, 0.2f },
            { fAttack, 1.2f }, { fDecay, 2.0f }, { fSustain, 0.5f }, { fRelease, 2.0f },
            { aAttack, 0.9f }, { aSustain, 0.9f }, { aRelease, 2.2f }, { ampVel, 0.2f },
            { chorus, CH_I }, { reverbMix, 0.45f }, { spread, 0.7f } } },

        { "PWM Brass", "Brass", {
            { osc1Wave, SQUARE }, { osc1PW, 0.35f }, { osc2Fine, -8 }, { lfoPW, 0.25f }, { lfoRate, 0.8f },
            { cutoff, 700 }, { resonance, 0.15f }, { filterEnv, 0.55f },
            { fAttack, 0.06f }, { fDecay, 0.5f }, { fSustain, 0.45f }, { fRelease, 0.3f },
            { aAttack, 0.03f }, { aSustain, 0.85f }, { aRelease, 0.3f }, { chorus, CH_I }, { spread, 0.5f } } },

        { "Poly Brass Stab", "Brass", {
            { cutoff, 600 }, { resonance, 0.2f }, { filterEnv, 0.65f }, { filterVel, 0.6f },
            { fAttack, 0.02f }, { fDecay, 0.35f }, { fSustain, 0.3f }, { fRelease, 0.2f },
            { aAttack, 0.005f }, { aDecay, 0.4f }, { aSustain, 0.6f }, { aRelease, 0.2f }, { ampVel, 0.7f },
            { chorus, CH_I }, { reverbMix, 0.15f } } },

        { "Fat Bass", "Bass", {
            { voiceMode, MONO }, { osc2Wave, SQUARE }, { osc2Semi, -12 }, { mixOsc2, 0.7f }, { mixSub, 0.6f },
            { cutoff, 300 }, { resonance, 0.3f }, { filterEnv, 0.5f }, { fDecay, 0.25f }, { fSustain, 0.15f },
            { drive, 0.5f }, { keyTrack, 0.3f }, { aSustain, 1.0f }, { aRelease, 0.12f }, { spread, 0.0f } } },

        { "Acid Mono", "Bass", {
            { voiceMode, LEGATO }, { mixOsc2, 0.0f }, { cutoff, 350 }, { resonance, 0.82f }, { filterEnv, 0.7f },
            { fDecay, 0.18f }, { fSustain, 0.05f }, { drive, 0.55f }, { glide, 0.05f }, { filterVel, 0.5f },
            { aAttack, 0.001f }, { aDecay, 0.3f }, { aSustain, 0.9f }, { aRelease, 0.08f },
            { delayMix, 0.15f }, { spread, 0.0f } } },

        { "Sub Bass", "Bass", {
            { voiceMode, MONO }, { osc1Wave, SINE }, { osc2Wave, TRI }, { osc2Semi, 12 }, { osc2Fine, 0 }, { mixOsc2, 0.25f },
            { mixSub, 0.4f }, { cutoff, 900 }, { resonance, 0.0f }, { filterEnv, 0.0f }, { drive, 0.3f },
            { aSustain, 1.0f }, { aRelease, 0.1f }, { drift, 0.1f }, { spread, 0.0f } } },

        { "Pluck", "Keys", {
            { osc2Wave, SQUARE }, { cutoff, 400 }, { resonance, 0.3f }, { filterEnv, 0.7f }, { filterVel, 0.5f },
            { fDecay, 0.3f }, { fSustain, 0.0f }, { fRelease, 0.3f },
            { aAttack, 0.001f }, { aDecay, 0.9f }, { aSustain, 0.0f }, { aRelease, 0.5f },
            { delayMix, 0.25f }, { chorus, CH_I }, { reverbMix, 0.15f } } },

        { "Analog Keys", "Keys", {
            { osc1Wave, SQUARE }, { osc1PW, 0.45f }, { osc2Semi, 12 }, { mixOsc2, 0.3f },
            { cutoff, 1500 }, { filterEnv, 0.4f }, { fDecay, 1.2f }, { fSustain, 0.2f },
            { aDecay, 2.5f }, { aSustain, 0.0f }, { aRelease, 0.6f }, { chorus, CH_I }, { reverbMix, 0.2f } } },

        { "Bell Tri", "Keys", {
            { osc1Wave, TRI }, { osc2Wave, SINE }, { osc2Semi, 19 }, { osc2Fine, 0 }, { mixOsc2, 0.5f },
            { cutoff, 6000 }, { resonance, 0.0f }, { filterEnv, 0.0f }, { drive, 0.0f },
            { aAttack, 0.001f }, { aDecay, 2.0f }, { aSustain, 0.0f }, { aRelease, 1.5f },
            { reverbMix, 0.35f }, { delayMix, 0.2f }, { spread, 0.5f } } },

        { "Wobble", "FX", {
            { voiceMode, MONO }, { mixSub, 0.5f }, { cutoff, 250 }, { resonance, 0.6f }, { filterEnv, 0.0f },
            { lfoWave, L_TRI }, { lfoRate, 3.0f }, { lfoFilter, 0.7f }, { drive, 0.6f },
            { aSustain, 1.0f }, { aRelease, 0.15f }, { spread, 0.0f } } },

        { "S&H Bleeps", "FX", {
            { osc1Wave, SQUARE }, { mixOsc2, 0.0f }, { cutoff, 800 }, { resonance, 0.7f }, { filterEnv, 0.1f },
            { lfoWave, L_SH }, { lfoRate, 8.0f }, { lfoFilter, 0.8f },
            { delayMix, 0.35f }, { delayFb, 0.55f }, { reverbMix, 0.2f } } },

        { "Noise Sweep", "FX", {
            { mixOsc1, 0.2f }, { mixOsc2, 0.0f }, { mixNoise, 0.9f }, { cutoff, 300 }, { resonance, 0.75f }, { filterEnv, 0.7f },
            { fAttack, 3.0f }, { fDecay, 4.0f }, { fSustain, 0.2f }, { fRelease, 3.0f },
            { aAttack, 1.5f }, { aSustain, 0.9f }, { aRelease, 3.0f }, { reverbMix, 0.5f }, { spread, 0.8f } } },

        { "Wide Detune", "Pad", {
            { osc2Fine, 18 }, { drift, 0.7f }, { spread, 0.9f }, { chorus, CH_II },
            { cutoff, 5000 }, { resonance, 0.1f }, { filterEnv, 0.15f },
            { aAttack, 0.01f }, { aRelease, 0.5f }, { reverbMix, 0.25f } } },

        // Sekvenser: håll en tangent (eller lägg en lång ton i DAW:en) och tryck play.
        { "Odd Seven Acid", "Sequence", {
            { voiceMode, LEGATO }, { mixOsc2, 0.0f }, { cutoff, 400 }, { resonance, 0.8f }, { filterEnv, 0.65f },
            { fDecay, 0.2f }, { fSustain, 0.05f }, { drive, 0.55f }, { glide, 0.05f }, { filterVel, 0.6f },
            { aAttack, 0.001f }, { aDecay, 0.3f }, { aSustain, 0.9f }, { aRelease, 0.08f },
            { delayMix, 0.2f }, { spread, 0.0f },
            { seqMode, S_SEQ }, { seqRate, R_16 }, { seqLength, 7 }, { seqAccLen, 5 }, { seqGateLen, 3 } },
          "0! 0 12~ 0 3 . 7?70" },

        { "Five Over Four Arp", "Sequence", {
            { osc2Wave, SQUARE }, { cutoff, 900 }, { resonance, 0.3f }, { filterEnv, 0.6f }, { filterVel, 0.5f },
            { fDecay, 0.25f }, { fSustain, 0.0f }, { aAttack, 0.001f }, { aDecay, 0.6f }, { aSustain, 0.0f },
            { aRelease, 0.3f }, { delayMix, 0.3f }, { delayFb, 0.45f }, { chorus, CH_I }, { reverbMix, 0.15f },
            { seqMode, S_ARP }, { seqRate, R_16 }, { arpOrder, A_UPDOWN }, { arpOctaves, 2 },
            { seqLength, 5 }, { seqAccLen, 4 } },
          "0! 0 . 0 0'" },

        { "Euclid 5/8 Lead", "Sequence", {
            { voiceMode, MONO }, { osc1Wave, SQUARE }, { osc2Wave, SQUARE }, { osc2Semi, 12 }, { mixOsc2, 0.35f },
            { cutoff, 2200 }, { resonance, 0.25f }, { filterEnv, 0.35f }, { fDecay, 0.3f }, { fSustain, 0.2f },
            { aSustain, 1.0f }, { aRelease, 0.1f }, { delayMix, 0.25f }, { spread, 0.0f },
            { seqMode, S_SEQ }, { seqRate, R_16 }, { seqSwing, 0.35f }, { seqLength, 8 }, { seqAccLen, 3 } },
          "0! . 0 . 7 0 . 12" },

        { "Triplet Drift Arp", "Sequence", {
            { osc2Fine, 12 }, { cutoff, 2400 }, { resonance, 0.15f }, { filterEnv, 0.3f }, { fDecay, 0.4f },
            { aAttack, 0.005f }, { aDecay, 0.5f }, { aSustain, 0.3f }, { aRelease, 0.4f },
            { chorus, CH_II }, { reverbMix, 0.35f }, { delayMix, 0.2f }, { spread, 0.7f },
            { seqMode, S_ARP }, { seqRate, R_8T }, { arpOctaves, 3 }, { seqLength, 11 } },
          "0! 0?60 0 0?50 0_ 0?70" },

        // v1.2 "Fat": tre oscillatorer, feedback-loop, warmth, unison och Polivoks-filter.
        { "Fat Ladder Bass", "Fat Bass", {
            { voiceMode, MONO }, { osc2Fine, 6 }, { mixOsc1, 0.9f }, { mixOsc2, 0.7f },
            { osc3Wave, SQUARE }, { osc3Semi, -12 }, { osc3Fine, -4 }, { mixOsc3, 0.8f },
            { cutoff, 260 }, { resonance, 0.3f }, { drive, 0.55f }, { filterEnv, 0.55f }, { fDecay, 0.3f }, { fSustain, 0.2f },
            { keyTrack, 0.4f }, { feedback, 0.35f }, { warmth, 0.45f }, { filterType, LADDER },
            { aSustain, 1.0f }, { aRelease, 0.12f }, { glide, 0.02f }, { drift, 0.35f }, { spread, 0.0f }, { master, -4.0f } } },

        { "Steppe Growl", "Fat Bass", {
            { voiceMode, MONO }, { osc2Wave, SQUARE }, { osc2Semi, -12 }, { osc2Fine, 0 }, { mixOsc2, 0.75f },
            { osc3Semi, 0 }, { osc3Fine, 9 }, { mixOsc3, 0.5f },
            { filterType, POLI_LP }, { cutoff, 300 }, { resonance, 0.65f }, { drive, 0.7f }, { filterEnv, 0.6f },
            { fDecay, 0.22f }, { fSustain, 0.15f }, { filterVel, 0.5f }, { feedback, 0.25f }, { warmth, 0.3f },
            { aSustain, 1.0f }, { aRelease, 0.1f }, { spread, 0.0f } } },

        { "Russian Squelch", "Fat Bass", {
            { voiceMode, LEGATO }, { mixOsc2, 0.0f }, { filterType, POLI_BP }, { cutoff, 500 }, { resonance, 0.82f },
            { filterEnv, 0.55f }, { fDecay, 0.25f }, { fSustain, 0.1f }, { drive, 0.6f }, { glide, 0.06f },
            { warmth, 0.25f }, { aSustain, 1.0f }, { aRelease, 0.1f }, { delayMix, 0.2f }, { spread, 0.0f } } },

        { "Sub Monster", "Fat Bass", {
            { voiceMode, MONO }, { osc1Wave, TRI }, { osc2Wave, SQUARE }, { osc2Semi, -12 }, { osc2Fine, 0 }, { mixOsc2, 0.6f },
            { mixSub, 0.7f }, { osc3Semi, -12 }, { mixOsc3, 0.3f },
            { cutoff, 420 }, { resonance, 0.15f }, { filterEnv, 0.25f }, { drive, 0.5f }, { feedback, 0.45f }, { warmth, 0.7f },
            { aSustain, 1.0f }, { aRelease, 0.1f }, { spread, 0.0f }, { master, -5.0f } } },

        { "Three Osc Lead", "Fat Lead", {
            { voiceMode, LEGATO }, { osc2Fine, 7 }, { osc3Semi, -12 }, { osc3Fine, -6 }, { mixOsc3, 0.7f },
            { cutoff, 1600 }, { resonance, 0.3f }, { drive, 0.5f }, { filterEnv, 0.45f }, { fDecay, 0.5f }, { fSustain, 0.45f },
            { feedback, 0.2f }, { warmth, 0.35f }, { glide, 0.07f },
            { lfoPitch, 0.25f }, { lfoFade, 0.7f }, { lfoRate, 5.5f },
            { aSustain, 0.95f }, { aRelease, 0.3f }, { delayMix, 0.25f }, { delayFb, 0.35f }, { reverbMix, 0.15f }, { spread, 0.0f } } },

        { "Thick Unison Lead", "Fat Lead", {
            { voiceMode, LEGATO }, { unison, 4 }, { uniDetune, 0.35f }, { osc2Wave, SQUARE }, { osc2Fine, 5 },
            { osc3Semi, 12 }, { osc3Fine, 0 }, { mixOsc3, 0.3f },
            { cutoff, 2400 }, { resonance, 0.25f }, { drive, 0.45f }, { filterEnv, 0.35f }, { warmth, 0.4f }, { glide, 0.05f },
            { aSustain, 0.95f }, { aRelease, 0.35f }, { delayMix, 0.3f }, { reverbMix, 0.2f }, { spread, 0.0f } } },

        { "Steppe Lead", "Fat Lead", {
            { voiceMode, LEGATO }, { osc2Wave, SQUARE }, { osc2Semi, 12 }, { osc2Fine, 0 }, { mixOsc2, 0.4f },
            { filterType, POLI_LP }, { cutoff, 1300 }, { resonance, 0.6f }, { drive, 0.55f }, { filterEnv, 0.5f },
            { fDecay, 0.6f }, { fSustain, 0.5f }, { glide, 0.08f }, { lfoPitch, 0.3f }, { lfoFade, 0.6f },
            { aSustain, 1.0f }, { aRelease, 0.25f }, { delayMix, 0.25f }, { reverbMix, 0.15f }, { spread, 0.0f } } },

        { "Feedback Scream", "Fat Lead", {
            { voiceMode, MONO }, { cutoff, 1100 }, { resonance, 0.55f }, { drive, 0.7f }, { feedback, 0.85f }, { warmth, 0.5f },
            { filterEnv, 0.4f }, { fDecay, 0.4f }, { fSustain, 0.4f }, { lfoPitch, 0.2f }, { lfoFade, 0.5f },
            { aSustain, 1.0f }, { aRelease, 0.2f }, { delayMix, 0.3f }, { spread, 0.0f } } },

        { "Fat Brass Poly", "Fat Poly", {
            { unison, 2 }, { uniDetune, 0.3f }, { osc2Fine, -8 }, { cutoff, 650 }, { resonance, 0.15f }, { filterEnv, 0.6f },
            { fAttack, 0.04f }, { fDecay, 0.6f }, { fSustain, 0.45f }, { aAttack, 0.02f }, { aSustain, 0.85f },
            { warmth, 0.35f }, { chorus, CH_I }, { reverbMix, 0.15f }, { spread, 0.6f } } },

        { "Fat Poly Pad", "Fat Poly", {
            { unison, 3 }, { uniDetune, 0.5f }, { osc3Semi, 12 }, { osc3Fine, 0 }, { mixOsc3, 0.3f },
            { cutoff, 1500 }, { resonance, 0.15f }, { filterEnv, 0.2f }, { fAttack, 1.0f }, { fDecay, 2.0f },
            { aAttack, 0.8f }, { aSustain, 0.9f }, { aRelease, 2.0f }, { ampVel, 0.2f },
            { warmth, 0.3f }, { chorus, CH_II }, { reverbMix, 0.4f }, { spread, 0.8f } } },

        { "Steppe Nine Seq", "Sequence", {
            { voiceMode, LEGATO }, { osc2Wave, SQUARE }, { osc2Semi, -12 }, { osc2Fine, 0 }, { mixOsc2, 0.5f },
            { filterType, POLI_LP }, { cutoff, 380 }, { resonance, 0.75f }, { drive, 0.65f }, { filterEnv, 0.6f },
            { fDecay, 0.18f }, { fSustain, 0.05f }, { filterVel, 0.6f }, { warmth, 0.3f }, { glide, 0.05f },
            { aSustain, 0.9f }, { aRelease, 0.08f }, { delayMix, 0.2f }, { spread, 0.0f },
            { seqMode, S_SEQ }, { seqRate, R_16 }, { seqLength, 9 }, { seqAccLen, 4 }, { seqGateLen, 5 } },
          "0! 12 0 0~ 3 . 0 10?60 -2" },

        // v1.3: 100 presets till (genereras av tools/gen_presets.py).
        #include "PresetBank.inc"

        // v1.7: Polyrytm (läggs sist – programnumren sparas i projekt).
        { "Four Against Three", "Polyrhythm", withPoly ({
            { aDecay, 0.35f }, { aSustain, 0.0f }, { aRelease, 0.3f }, { filterEnv, 0.6f }, { fDecay, 0.25f },
            { fSustain, 0.0f }, { cutoff, 650 }, { resonance, 0.3f }, { osc2Fine, 7 }, { delayMix, 0.2f },
            { reverbMix, 0.15f }, { spread, 0.5f }, { master, -4.7f } },
            { { ON, 0, 4, 16, 0, 1 }, { ON, 0, 3, 16, 7, 1 } }) },

        { "Five Over Four Bells", "Polyrhythm", withPoly ({
            { osc1Wave, TRI }, { osc2Wave, SINE }, { osc2Semi, 12 }, { osc2Fine, 3 }, { mixOsc2, 0.5f },
            { aDecay, 0.9f }, { aSustain, 0.0f }, { aRelease, 0.8f }, { cutoff, 3200 }, { filterEnv, 0.3f },
            { fDecay, 0.4f }, { fSustain, 0.0f }, { reverbMix, 0.35f }, { delayMix, 0.25f }, { spread, 0.6f },
            { master, -3.8f } },
            { { ON, 0, 4, 16, 0, 1 }, { ON, 0, 5, 16, 12, 5 } }), {}, "| 0 3 7 10 5" },

        { "Gear Divider Drone", "Polyrhythm", withPoly ({
            { seqRate, R_16 }, { filterType, POLI_LP }, { cutoff, 450 }, { resonance, 0.65f }, { drive, 0.55f },
            { filterEnv, 0.5f }, { fDecay, 0.3f }, { fSustain, 0.1f }, { aDecay, 0.6f }, { aSustain, 0.3f },
            { aRelease, 0.4f }, { mixSub, 0.4f }, { warmth, 0.4f }, { reverbMix, 0.2f }, { spread, 0.5f },
            { master, -10.0f } },
            { { ON, DIV, 2, 16, 0, 4 }, { ON, DIV, 3, 16, 7, 3 }, { ON, DIV, 5, 16, -12, 1 }, { ON, DIV, 7, 16, 12, 2 } }),
          {}, "0 3 0 -2 | 0 5 3 | 0 | 0 -5" },

        { "Seven Over Four Pulse", "Polyrhythm", withPoly ({
            { osc1Wave, SQUARE }, { osc1PW, 0.3f }, { mixOsc2, 0.3f }, { osc2Semi, 12 }, { aDecay, 0.25f },
            { aSustain, 0.0f }, { aRelease, 0.2f }, { cutoff, 900 }, { resonance, 0.4f }, { filterEnv, 0.55f },
            { fDecay, 0.18f }, { fSustain, 0.0f }, { delayMix, 0.25f }, { spread, 0.5f }, { master, -5.5f } },
            { { ON, 0, 4, 16, -12, 1 }, { ON, 0, 7, 16, 0, 3 } }), {}, "| 0 7 12" },

        { "Euclid Ring Bass", "Polyrhythm", withPoly ({
            { voiceMode, MONO }, { cutoff, 320 }, { resonance, 0.45f }, { filterEnv, 0.6f }, { fDecay, 0.2f },
            { fSustain, 0.05f }, { aSustain, 0.8f }, { aRelease, 0.1f }, { drive, 0.5f }, { mixSub, 0.4f },
            { spread, 0.0f }, { glide, 0.03f }, { master, -2.0f } },
            { { ON, 0, 16, 5, -12, 4 }, { ON, 0, 3, 16, 0, 1 } }), {}, "0 0 3 -2" },

        { "Triplet Cascade", "Polyrhythm", withPoly ({
            { osc1Wave, SAW }, { osc2Wave, TRI }, { osc2Semi, 7 }, { mixOsc2, 0.4f }, { aDecay, 0.4f },
            { aSustain, 0.0f }, { aRelease, 0.5f }, { cutoff, 1400 }, { filterEnv, 0.4f }, { fDecay, 0.3f },
            { fSustain, 0.0f }, { chorus, CH_I }, { delayMix, 0.3f }, { reverbMix, 0.3f }, { spread, 0.6f },
            { master, -5.9f } },
            { { ON, 0, 3, 16, 0, 2 }, { ON, 0, 4, 16, 12, 1 }, { ON, 0, 5, 16, 7, 3 }, { ON, 0, 6, 4, 19, 2 } }, 2),
          {}, "0 -5 | 0 | 0 3 5 | 0 -2" },

        { "Clockwork Acid", "Polyrhythm", withPoly ({
            { voiceMode, LEGATO }, { seqRate, R_16 }, { mixOsc2, 0.0f }, { cutoff, 300 }, { resonance, 0.82f },
            { filterEnv, 0.75f }, { fDecay, 0.16f }, { fSustain, 0.0f }, { drive, 0.6f }, { glide, 0.05f },
            { filterVel, 0.6f }, { delayMix, 0.15f }, { spread, 0.0f }, { master, -0.5f } },
            { { ON, DIV, 1, 16, 0, 7 }, { ON, DIV, 3, 16, 12, 1 } }), {}, "0 0 12 3 0 10 7" },

        { "Phase Pluck", "Polyrhythm", withPoly ({
            { osc1Wave, SQUARE }, { osc2Wave, SAW }, { osc2Fine, 5 }, { aDecay, 0.3f }, { aSustain, 0.0f },
            { aRelease, 0.3f }, { cutoff, 800 }, { filterEnv, 0.55f }, { fDecay, 0.2f }, { fSustain, 0.0f },
            { delayMix, 0.3f }, { delayFb, 0.45f }, { reverbMix, 0.2f }, { spread, 0.7f }, { master, -5.3f } },
            { { ON, 0, 7, 16, 0, 7 }, { ON, 0, 8, 16, 12, 8 } }, 2), {}, "0 3 5 7 10 7 5 | 0 -2 3 5 7 5 3 -2" },

        { "Steppe Gears Lead", "Polyrhythm", withPoly ({
            { voiceMode, LEGATO }, { filterType, POLI_LP }, { cutoff, 600 }, { resonance, 0.7f }, { drive, 0.65f },
            { filterEnv, 0.55f }, { fDecay, 0.2f }, { fSustain, 0.15f }, { aSustain, 0.9f }, { aRelease, 0.15f },
            { glide, 0.06f }, { warmth, 0.3f }, { delayMix, 0.25f }, { spread, 0.0f }, { master, -2.0f } },
            { { ON, 0, 5, 16, 0, 5 }, { ON, 0, 3, 16, 12, 3 } }), {}, "0 3 7 5 10 | 0 -2 3" },

        { "Slow Orbit Pad", "Polyrhythm", withPoly ({
            { aAttack, 0.4f }, { aDecay, 1.5f }, { aSustain, 0.4f }, { aRelease, 1.2f }, { fAttack, 0.5f },
            { fDecay, 1.5f }, { fSustain, 0.4f }, { filterEnv, 0.3f }, { cutoff, 1100 }, { osc2Fine, -9 },
            { unison, 2 }, { uniDetune, 0.35f }, { chorus, CH_II }, { reverbMix, 0.5f }, { spread, 0.8f },
            { master, -9.2f } },
            { { ON, 0, 1, 16, -12, 1 }, { ON, 0, 3, 16, 0, 3 }, { ON, 0, 5, 16, 7, 1 }, { ON, 0, 2, 16, 12, 2 } }, 4),
          {}, "| 0 3 5 | | 0 -2" },

        // v1.7: Undertoner (subharmoniska oscillatorer). Läggs sist.
        { "Undertone Bass", "Subharmonic", {
            { voiceMode, MONO }, { osc2Wave, SQUARE }, { osc2Fine, 6 }, { mixOsc2, 0.4f },
            { mixSubh1, 0.7f }, { osc1SubDiv, 2 }, { mixSubh2, 0.35f }, { osc2SubDiv, 3 },
            { cutoff, 420 }, { resonance, 0.3f }, { filterEnv, 0.5f }, { fDecay, 0.25f }, { fSustain, 0.15f },
            { drive, 0.5f }, { warmth, 0.35f }, { aSustain, 0.85f }, { aRelease, 0.12f }, { glide, 0.03f },
            { spread, 0.0f }, { master, -0.6f } } },

        { "Divided Fifths Drone", "Subharmonic", withPoly ({
            { seqRate, R_16 }, { osc2Semi, 7 }, { osc2Fine, 0 }, { mixOsc2, 0.5f },
            { mixSubh1, 0.6f }, { osc1SubDiv, 3 }, { mixSubh2, 0.5f }, { osc2SubDiv, 4 },
            { cutoff, 520 }, { resonance, 0.55f }, { filterEnv, 0.5f }, { fDecay, 0.35f }, { fSustain, 0.1f },
            { aDecay, 0.7f }, { aSustain, 0.25f }, { aRelease, 0.5f }, { feedback, 0.25f }, { warmth, 0.4f },
            { delayMix, 0.2f }, { reverbMix, 0.2f }, { spread, 0.5f }, { master, -7.7f } },
            { { ON, DIV, 4, 16, 0, 4 }, { ON, DIV, 3, 16, 0, 3 } }), {}, "0 -2 3 0 | 0 5 -5" },

        { "Subharmonic Choir", "Subharmonic", {
            { osc1Wave, SAW }, { osc2Wave, SAW }, { osc2Semi, 0 }, { osc2Fine, -8 },
            { mixSubh1, 0.5f }, { osc1SubDiv, 5 }, { mixSubh2, 0.45f }, { osc2SubDiv, 6 },
            { aAttack, 0.6f }, { aSustain, 0.9f }, { aRelease, 1.4f }, { fAttack, 0.6f }, { fDecay, 1.5f },
            { fSustain, 0.5f }, { filterEnv, 0.25f }, { cutoff, 900 }, { resonance, 0.2f },
            { chorus, CH_I }, { reverbMix, 0.4f }, { spread, 0.7f }, { master, -11.8f } } },

        { "Gearbox Sequence", "Subharmonic", withPoly ({
            { voiceMode, LEGATO }, { seqRate, R_16 }, { osc1Wave, SQUARE }, { osc2Wave, SAW }, { osc2Fine, 7 },
            { mixSubh1, 0.55f }, { osc1SubDiv, 2 }, { mixSubh2, 0.45f }, { osc2SubDiv, 5 },
            { cutoff, 350 }, { resonance, 0.7f }, { filterEnv, 0.65f }, { fDecay, 0.18f }, { fSustain, 0.0f },
            { drive, 0.55f }, { feedback, 0.3f }, { glide, 0.04f }, { delayMix, 0.2f }, { spread, 0.0f },
            { master, -4.6f } },
            { { ON, DIV, 2, 16, 0, 5 }, { ON, DIV, 3, 16, 12, 2 } }), {}, "0 3 7 -2 5 | 0 -5" },
    };

    return presets;
}

void applyPreset (juce::AudioProcessorValueTreeState& apvts, const Preset& preset, seq::Pattern& pattern)
{
    if (preset.pattern.isEmpty())
        pattern.setDefault();
    else
        pattern.fromString (preset.pattern);
    pattern.lanesFromString (preset.lanes);

    for (auto* p : apvts.processor.getParameters())
        if (auto* rp = dynamic_cast<juce::RangedAudioParameter*> (p))
            rp->setValueNotifyingHost (rp->getDefaultValue());

    for (const auto& [id, value] : preset.values)
        if (auto* rp = apvts.getParameter (id))
            rp->setValueNotifyingHost (rp->convertTo0to1 (value));
}
} // namespace steso
