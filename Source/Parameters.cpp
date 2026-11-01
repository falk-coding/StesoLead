#include "Parameters.h"
#include "seq/Sequencer.h"

namespace steso
{
namespace
{
    using Range = juce::NormalisableRange<float>;
    using APF = juce::AudioParameterFloat;
    using APC = juce::AudioParameterChoice;
    using API = juce::AudioParameterInt;

    juce::AudioParameterFloatAttributes withLabel (const juce::String& label)
    {
        return juce::AudioParameterFloatAttributes().withLabel (label);
    }

    // Tid i sekunder, visas som ms under 1 s.
    juce::AudioParameterFloatAttributes timeAttr()
    {
        return juce::AudioParameterFloatAttributes()
            .withStringFromValueFunction ([] (float v, int)
            {
                return v < 1.0f ? juce::String (juce::roundToInt (v * 1000.0f)) + " ms"
                                : juce::String (v, 2) + " s";
            })
            .withValueFromStringFunction ([] (const juce::String& s)
            {
                auto v = s.getFloatValue();
                return s.containsIgnoreCase ("ms") ? v / 1000.0f : v;
            });
    }

    juce::AudioParameterFloatAttributes percentAttr()
    {
        return juce::AudioParameterFloatAttributes()
            .withStringFromValueFunction ([] (float v, int) { return juce::String (juce::roundToInt (v * 100.0f)) + " %"; })
            .withValueFromStringFunction ([] (const juce::String& s) { return s.getFloatValue() / 100.0f; });
    }

    juce::AudioParameterFloatAttributes bipolarPercentAttr()
    {
        return juce::AudioParameterFloatAttributes()
            .withStringFromValueFunction ([] (float v, int)
            {
                auto p = juce::roundToInt (v * 100.0f);
                return (p > 0 ? "+" : "") + juce::String (p) + " %";
            })
            .withValueFromStringFunction ([] (const juce::String& s) { return s.getFloatValue() / 100.0f; });
    }

    Range timeRange (float maxSeconds)
    {
        Range r (0.001f, maxSeconds);
        r.setSkewForCentre (maxSeconds < 3.0f ? 0.25f : 0.5f);
        return r;
    }

    juce::ParameterID id (const char* s) { return { s, 1 }; }
    // Parametrar som tillkommit efter första releasen får högre versionshint (krävs av AU-värdar).
    juce::ParameterID id2 (const juce::String& s) { return { s, 2 }; }
} // namespace

juce::AudioProcessorValueTreeState::ParameterLayout createParameterLayout()
{
    juce::AudioProcessorValueTreeState::ParameterLayout layout;

    auto osc1 = std::make_unique<juce::AudioProcessorParameterGroup> ("osc1", "Osc 1", "|");
    osc1->addChild (std::make_unique<APC> (id (pid::osc1Wave), "Osc1 Wave", waveNames, 0));
    osc1->addChild (std::make_unique<API> (id (pid::osc1Octave), "Osc1 Octave", -2, 2, 0));
    osc1->addChild (std::make_unique<APF> (id (pid::osc1PW), "Osc1 PW", Range (0.05f, 0.95f), 0.5f, percentAttr()));

    auto osc2 = std::make_unique<juce::AudioProcessorParameterGroup> ("osc2", "Osc 2", "|");
    osc2->addChild (std::make_unique<APC> (id (pid::osc2Wave), "Osc2 Wave", waveNames, 0));
    osc2->addChild (std::make_unique<API> (id (pid::osc2Semi), "Osc2 Semi", -24, 24, 0));
    osc2->addChild (std::make_unique<APF> (id (pid::osc2Fine), "Osc2 Fine", Range (-50.0f, 50.0f), 7.0f, withLabel ("ct")));
    osc2->addChild (std::make_unique<APF> (id (pid::osc2PW), "Osc2 PW", Range (0.05f, 0.95f), 0.5f, percentAttr()));

    auto mix = std::make_unique<juce::AudioProcessorParameterGroup> ("mix", "Mixer", "|");
    mix->addChild (std::make_unique<APF> (id (pid::mixOsc1), "Mix Osc1", Range (0.0f, 1.0f), 0.8f, percentAttr()));
    mix->addChild (std::make_unique<APF> (id (pid::mixOsc2), "Mix Osc2", Range (0.0f, 1.0f), 0.6f, percentAttr()));
    mix->addChild (std::make_unique<APF> (id (pid::mixSub), "Mix Sub", Range (0.0f, 1.0f), 0.0f, percentAttr()));
    mix->addChild (std::make_unique<APF> (id (pid::mixNoise), "Mix Noise", Range (0.0f, 1.0f), 0.0f, percentAttr()));

    auto filter = std::make_unique<juce::AudioProcessorParameterGroup> ("filter", "Filter", "|");
    {
        Range cutoffRange (20.0f, 20000.0f);
        cutoffRange.setSkewForCentre (800.0f);
        filter->addChild (std::make_unique<APF> (id (pid::cutoff), "Cutoff", cutoffRange, 2500.0f,
            juce::AudioParameterFloatAttributes()
                .withStringFromValueFunction ([] (float v, int)
                {
                    return v < 1000.0f ? juce::String (juce::roundToInt (v)) + " Hz" : juce::String (v / 1000.0f, 1) + " kHz";
                })
                .withValueFromStringFunction ([] (const juce::String& s)
                {
                    auto v = s.getFloatValue();
                    return s.containsIgnoreCase ("k") ? v * 1000.0f : v;
                })));
    }
    filter->addChild (std::make_unique<APF> (id (pid::resonance), "Resonance", Range (0.0f, 1.0f), 0.2f, percentAttr()));
    filter->addChild (std::make_unique<APF> (id (pid::drive), "Drive", Range (0.0f, 1.0f), 0.15f, percentAttr()));
    filter->addChild (std::make_unique<APF> (id (pid::filterEnv), "Filter Env", Range (-1.0f, 1.0f), 0.35f, bipolarPercentAttr()));
    filter->addChild (std::make_unique<APF> (id (pid::keyTrack), "Key Track", Range (0.0f, 1.0f), 0.5f, percentAttr()));
    filter->addChild (std::make_unique<APF> (id (pid::filterVel), "Filter Velocity", Range (0.0f, 1.0f), 0.3f, percentAttr()));

    auto fenv = std::make_unique<juce::AudioProcessorParameterGroup> ("fenv", "Filter Env", "|");
    fenv->addChild (std::make_unique<APF> (id (pid::fAttack), "Filter Attack", timeRange (10.0f), 0.005f, timeAttr()));
    fenv->addChild (std::make_unique<APF> (id (pid::fDecay), "Filter Decay", timeRange (10.0f), 0.6f, timeAttr()));
    fenv->addChild (std::make_unique<APF> (id (pid::fSustain), "Filter Sustain", Range (0.0f, 1.0f), 0.3f, percentAttr()));
    fenv->addChild (std::make_unique<APF> (id (pid::fRelease), "Filter Release", timeRange (10.0f), 0.4f, timeAttr()));

    auto aenv = std::make_unique<juce::AudioProcessorParameterGroup> ("aenv", "Amp Env", "|");
    aenv->addChild (std::make_unique<APF> (id (pid::aAttack), "Amp Attack", timeRange (10.0f), 0.003f, timeAttr()));
    aenv->addChild (std::make_unique<APF> (id (pid::aDecay), "Amp Decay", timeRange (10.0f), 0.5f, timeAttr()));
    aenv->addChild (std::make_unique<APF> (id (pid::aSustain), "Amp Sustain", Range (0.0f, 1.0f), 0.8f, percentAttr()));
    aenv->addChild (std::make_unique<APF> (id (pid::aRelease), "Amp Release", timeRange (10.0f), 0.3f, timeAttr()));
    aenv->addChild (std::make_unique<APF> (id (pid::ampVel), "Amp Velocity", Range (0.0f, 1.0f), 0.5f, percentAttr()));

    auto lfo = std::make_unique<juce::AudioProcessorParameterGroup> ("lfo", "LFO", "|");
    lfo->addChild (std::make_unique<APC> (id (pid::lfoWave), "LFO Wave", lfoWaveNames, 0));
    {
        Range rateRange (0.05f, 20.0f);
        rateRange.setSkewForCentre (3.0f);
        lfo->addChild (std::make_unique<APF> (id (pid::lfoRate), "LFO Rate", rateRange, 5.0f, withLabel ("Hz")));
    }
    lfo->addChild (std::make_unique<APF> (id (pid::lfoFade), "LFO Fade In", timeRange (5.0f), 0.001f, timeAttr()));
    {
        Range pitchRange (0.0f, 12.0f);
        pitchRange.setSkewForCentre (1.0f);
        lfo->addChild (std::make_unique<APF> (id (pid::lfoPitch), "LFO > Pitch", pitchRange, 0.0f, withLabel ("st")));
    }
    lfo->addChild (std::make_unique<APF> (id (pid::lfoFilter), "LFO > Filter", Range (0.0f, 1.0f), 0.0f, percentAttr()));
    lfo->addChild (std::make_unique<APF> (id (pid::lfoPW), "LFO > PW", Range (0.0f, 1.0f), 0.0f, percentAttr()));

    auto voice = std::make_unique<juce::AudioProcessorParameterGroup> ("voice", "Voice", "|");
    voice->addChild (std::make_unique<APC> (id (pid::voiceMode), "Voice Mode", voiceModeNames, 0));
    voice->addChild (std::make_unique<APF> (id (pid::glide), "Glide", timeRange (2.0f), 0.001f, timeAttr()));
    voice->addChild (std::make_unique<API> (id (pid::bendRange), "Bend Range", 1, 12, 2));
    voice->addChild (std::make_unique<APF> (id (pid::drift), "Analog Drift", Range (0.0f, 1.0f), 0.3f, percentAttr()));
    voice->addChild (std::make_unique<APF> (id (pid::spread), "Stereo Spread", Range (0.0f, 1.0f), 0.3f, percentAttr()));

    auto fx = std::make_unique<juce::AudioProcessorParameterGroup> ("fx", "Effects", "|");
    fx->addChild (std::make_unique<APC> (id (pid::chorus), "Chorus", chorusNames, 0));
    {
        Range dtRange (0.02f, 1.2f);
        dtRange.setSkewForCentre (0.35f);
        fx->addChild (std::make_unique<APF> (id (pid::delayTime), "Delay Time", dtRange, 0.375f, timeAttr()));
    }
    fx->addChild (std::make_unique<APF> (id (pid::delayFb), "Delay Feedback", Range (0.0f, 0.9f), 0.35f, percentAttr()));
    fx->addChild (std::make_unique<APF> (id (pid::delayMix), "Delay Mix", Range (0.0f, 1.0f), 0.0f, percentAttr()));
    fx->addChild (std::make_unique<APF> (id (pid::reverbMix), "Reverb Mix", Range (0.0f, 1.0f), 0.0f, percentAttr()));
    fx->addChild (std::make_unique<APF> (id (pid::master), "Master", Range (-36.0f, 6.0f), -6.0f, withLabel ("dB")));

    auto seqGroup = std::make_unique<juce::AudioProcessorParameterGroup> ("seq", "Sequencer", "|");
    seqGroup->addChild (std::make_unique<APC> (id (pid::seqMode), "Seq Mode", seq::modeNames, 0));
    seqGroup->addChild (std::make_unique<APC> (id (pid::seqRate), "Seq Rate", seq::rateNames, 4));
    seqGroup->addChild (std::make_unique<APF> (id (pid::seqSwing), "Seq Swing", Range (0.0f, 1.0f), 0.0f, percentAttr()));
    seqGroup->addChild (std::make_unique<APF> (id (pid::seqGate), "Seq Gate", Range (0.1f, 1.0f), 1.0f, percentAttr()));
    seqGroup->addChild (std::make_unique<APC> (id (pid::arpOrder), "Arp Order", seq::arpOrderNames, 0));
    seqGroup->addChild (std::make_unique<API> (id (pid::arpOctaves), "Arp Octaves", 1, 4, 1));
    seqGroup->addChild (std::make_unique<API> (id (pid::seqLength), "Seq Steps", 1, seq::maxSteps, 16));
    seqGroup->addChild (std::make_unique<API> (id (pid::seqAccLen), "Accent Steps", 1, seq::maxSteps, 16));
    seqGroup->addChild (std::make_unique<API> (id (pid::seqGateLen), "Gate Steps", 1, seq::maxSteps, 16));
    // Av/på som valparametrar: AudioParameterBool återställs inte exakt från sparat läge (pluginval).
    seqGroup->addChild (std::make_unique<APC> (id (pid::seqLatch), "Seq Latch", juce::StringArray { "Off", "On" }, 1));
    seqGroup->addChild (std::make_unique<APC> (id (pid::seqMidiOut), "Seq MIDI Out", juce::StringArray { "Off", "On" }, 0));

    // v1.2 "Fat": standardvärdena stänger av allt nytt så att äldre presets/projekt låter som förut.
    auto fat = std::make_unique<juce::AudioProcessorParameterGroup> ("fat", "Fat", "|");
    fat->addChild (std::make_unique<APC> (id (pid::osc3Wave), "Osc3 Wave", waveNames, 0));
    fat->addChild (std::make_unique<API> (id (pid::osc3Semi), "Osc3 Semi", -24, 24, -12));
    fat->addChild (std::make_unique<APF> (id (pid::osc3Fine), "Osc3 Fine", Range (-50.0f, 50.0f), -5.0f, withLabel ("ct")));
    fat->addChild (std::make_unique<APF> (id (pid::mixOsc3), "Mix Osc3", Range (0.0f, 1.0f), 0.0f, percentAttr()));
    fat->addChild (std::make_unique<APC> (id (pid::filterType), "Filter Type", filterTypeNames, 0));
    fat->addChild (std::make_unique<APF> (id (pid::feedback), "Feedback", Range (0.0f, 1.0f), 0.0f, percentAttr()));
    fat->addChild (std::make_unique<APF> (id (pid::warmth), "Warmth", Range (0.0f, 1.0f), 0.0f, percentAttr()));
    fat->addChild (std::make_unique<API> (id (pid::unison), "Unison", 1, 4, 1));
    fat->addChild (std::make_unique<APF> (id (pid::uniDetune), "Unison Detune", Range (0.0f, 1.0f), 0.3f, percentAttr()));

    // v1.7 Polyrytm: polyOn = Off som standard.
    auto poly = std::make_unique<juce::AudioProcessorParameterGroup> ("poly", "Polyrhythm", "|");
    poly->addChild (std::make_unique<APC> (id2 (pid::polyOn), "Polyrhythm", juce::StringArray { "Off", "On" }, 0));
    poly->addChild (std::make_unique<API> (id2 (pid::polyBars), "Poly Cycle Bars", 1, 4, 1));
    {
        // Standard när man slår på: 4 mot 3 (grundton + kvint). Spår 3–4 avstängda.
        constexpr int defPulses[] { 4, 3, 5, 2 };
        constexpr int defPitch[] { 0, 7, 12, -12 };
        for (int l = 0; l < seq::polyLanes; ++l)
        {
            const auto n = juce::String (l + 1);
            poly->addChild (std::make_unique<APC> (id2 (pid::polyLaneOn (l)), "Poly " + n + " On",
                                                   juce::StringArray { "Off", "On" }, l < 2 ? 1 : 0));
            poly->addChild (std::make_unique<APC> (id2 (pid::polyLaneType (l)), "Poly " + n + " Type", seq::polyTypeNames, 0));
            poly->addChild (std::make_unique<API> (id2 (pid::polyLanePulses (l)), "Poly " + n + " Pulses", 1, seq::polyMaxPulses, defPulses[l]));
            poly->addChild (std::make_unique<API> (id2 (pid::polyLaneHits (l)), "Poly " + n + " Hits", 1, seq::polyMaxPulses, seq::polyMaxPulses));
            poly->addChild (std::make_unique<API> (id2 (pid::polyLanePitch (l)), "Poly " + n + " Pitch", -24, 24, defPitch[l]));
            poly->addChild (std::make_unique<API> (id2 (pid::polyLaneSteps (l)), "Poly " + n + " Steps", 1, seq::polyMaxSteps, 1));
        }
    }

    // v1.7 Undertoner: nivåerna är 0 som standard -> äldre projekt/presets låter som förut.
    auto subh = std::make_unique<juce::AudioProcessorParameterGroup> ("subh", "Subharmonics", "|");
    subh->addChild (std::make_unique<API> (id2 (pid::osc1SubDiv), "Osc1 Subharmonic Div", 1, 16, 2));
    subh->addChild (std::make_unique<API> (id2 (pid::osc2SubDiv), "Osc2 Subharmonic Div", 1, 16, 3));
    subh->addChild (std::make_unique<APF> (id2 (pid::mixSubh1), "Mix Subharmonic 1", Range (0.0f, 1.0f), 0.0f, percentAttr()));
    subh->addChild (std::make_unique<APF> (id2 (pid::mixSubh2), "Mix Subharmonic 2", Range (0.0f, 1.0f), 0.0f, percentAttr()));
    subh->addChild (std::make_unique<API> (id2 (pid::osc3SubDiv), "Osc3 Subharmonic Div", 1, 16, 2));
    subh->addChild (std::make_unique<APF> (id2 (pid::mixSubh3), "Mix Subharmonic 3", Range (0.0f, 1.0f), 0.0f, percentAttr()));

    layout.add (std::move (osc1), std::move (osc2), std::move (mix), std::move (filter), std::move (fenv),
                std::move (aenv), std::move (lfo), std::move (voice), std::move (fx), std::move (seqGroup), std::move (fat),
                std::move (poly), std::move (subh));
    return layout;
}

void ParamPointers::bind (juce::AudioProcessorValueTreeState& s)
{
    auto p = [&s] (const char* name)
    {
        auto* v = s.getRawParameterValue (name);
        jassert (v != nullptr);
        return v;
    };

    osc1Wave = p (pid::osc1Wave); osc1Octave = p (pid::osc1Octave); osc1PW = p (pid::osc1PW);
    osc2Wave = p (pid::osc2Wave); osc2Semi = p (pid::osc2Semi); osc2Fine = p (pid::osc2Fine); osc2PW = p (pid::osc2PW);
    mixOsc1 = p (pid::mixOsc1); mixOsc2 = p (pid::mixOsc2); mixSub = p (pid::mixSub); mixNoise = p (pid::mixNoise);
    cutoff = p (pid::cutoff); resonance = p (pid::resonance); drive = p (pid::drive);
    filterEnv = p (pid::filterEnv); keyTrack = p (pid::keyTrack); filterVel = p (pid::filterVel);
    fAttack = p (pid::fAttack); fDecay = p (pid::fDecay); fSustain = p (pid::fSustain); fRelease = p (pid::fRelease);
    aAttack = p (pid::aAttack); aDecay = p (pid::aDecay); aSustain = p (pid::aSustain); aRelease = p (pid::aRelease);
    ampVel = p (pid::ampVel);
    lfoWave = p (pid::lfoWave); lfoRate = p (pid::lfoRate); lfoFade = p (pid::lfoFade);
    lfoPitch = p (pid::lfoPitch); lfoFilter = p (pid::lfoFilter); lfoPW = p (pid::lfoPW);
    voiceMode = p (pid::voiceMode); glide = p (pid::glide); bendRange = p (pid::bendRange);
    drift = p (pid::drift); spread = p (pid::spread);
    chorus = p (pid::chorus); delayTime = p (pid::delayTime); delayFb = p (pid::delayFb);
    delayMix = p (pid::delayMix); reverbMix = p (pid::reverbMix); master = p (pid::master);
    seqMode = p (pid::seqMode); seqRate = p (pid::seqRate); seqSwing = p (pid::seqSwing);
    seqGate = p (pid::seqGate); arpOrder = p (pid::arpOrder); arpOctaves = p (pid::arpOctaves);
    seqLength = p (pid::seqLength); seqAccLen = p (pid::seqAccLen); seqGateLen = p (pid::seqGateLen);
    seqLatch = p (pid::seqLatch); seqMidiOut = p (pid::seqMidiOut);
    osc3Wave = p (pid::osc3Wave); osc3Semi = p (pid::osc3Semi); osc3Fine = p (pid::osc3Fine);
    mixOsc3 = p (pid::mixOsc3); filterType = p (pid::filterType); feedback = p (pid::feedback);
    warmth = p (pid::warmth); unison = p (pid::unison); uniDetune = p (pid::uniDetune);
    polyOn = p (pid::polyOn); polyBars = p (pid::polyBars);
    osc1SubDiv = p (pid::osc1SubDiv); osc2SubDiv = p (pid::osc2SubDiv);
    mixSubh1 = p (pid::mixSubh1); mixSubh2 = p (pid::mixSubh2);
    osc3SubDiv = p (pid::osc3SubDiv); mixSubh3 = p (pid::mixSubh3);
    for (int l = 0; l < 4; ++l)
    {
        auto& L = polyLane[(size_t) l];
        L.on = p (pid::polyLaneOn (l)); L.type = p (pid::polyLaneType (l));
        L.pulses = p (pid::polyLanePulses (l)); L.hits = p (pid::polyLaneHits (l));
        L.pitch = p (pid::polyLanePitch (l)); L.steps = p (pid::polyLaneSteps (l));
    }
}
} // namespace steso
