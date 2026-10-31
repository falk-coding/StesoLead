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

        { "Juno Strings", "Pad", {
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
    };

    return presets;
}

void applyPreset (juce::AudioProcessorValueTreeState& apvts, const Preset& preset)
{
    for (auto* p : apvts.processor.getParameters())
        if (auto* rp = dynamic_cast<juce::RangedAudioParameter*> (p))
            rp->setValueNotifyingHost (rp->getDefaultValue());

    for (const auto& [id, value] : preset.values)
        if (auto* rp = apvts.getParameter (id))
            rp->setValueNotifyingHost (rp->convertTo0to1 (value));
}
} // namespace steso
