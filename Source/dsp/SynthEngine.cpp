#include "SynthEngine.h"

namespace steso::dsp
{
namespace
{
    // Panoreringsposition per röst (sprids över stereofältet med "Spread").
    constexpr float voicePans[SynthEngine::maxVoices] { -0.8f, 0.8f, -0.4f, 0.4f, -1.0f, 1.0f, -0.2f, 0.2f };
    constexpr int controlChunk = 64;
}

//==============================================================================
void Voice::prepare (float sr, uint32_t seed)
{
    sampleRate = sr;
    rng.seed (seed);
    reset();
}

void Voice::reset()
{
    ampEnv.reset();
    filterEnv.reset();
    filter.reset();
    held = false;
    driftValue1 = driftValue2 = driftTarget1 = driftTarget2 = driftCutoff = 0.0f;
}

void Voice::start (int newNote, float newVelocity, float startPitch, bool retrigger, bool glideFromPrevious)
{
    if (! isActive())
    {
        // Fri löpande oscillatorer med slumpad fas, som en analog synth.
        osc1.phase = rng.unipolar();
        osc2.phase = rng.unipolar();
        sub.phase = osc1.phase * 0.5f;
        filter.reset();
        driftCutoff = rng.bipolar();
    }

    note = newNote;
    velocity = newVelocity;
    targetPitch = (float) newNote;
    pitch = (glideFromPrevious && startPitch >= 0.0f) ? startPitch : targetPitch;
    held = true;

    if (retrigger)
    {
        ampEnv.noteOn();
        filterEnv.noteOn();
        lfoFade = 0.0f;
    }
}

void Voice::glideTo (int newNote)
{
    note = newNote;
    targetPitch = (float) newNote;
    held = true;
}

void Voice::release()
{
    held = false;
    ampEnv.noteOff();
    filterEnv.noteOff();
}

void Voice::kill()
{
    reset();
}

void Voice::render (float* left, float* right, int numSamples, const BlockParams& bp,
                    const float* lfo, const float* cutoffHz, float pitchBendSemis, float modWheel)
{
    if (! isActive())
        return;

    const float glideCoef = bp.glideSeconds > 0.002f ? std::exp (-3.0f / (bp.glideSeconds * sampleRate)) : 0.0f;
    const float fadeInc = bp.lfoFadeSeconds > 0.002f ? 1.0f / (bp.lfoFadeSeconds * sampleRate) : 1.0f;
    const float driftCents = bp.drift * 0.12f;      // upp till ±12 cent
    const float driftSmooth = 1.0f - std::exp (-1.0f / (0.25f * sampleRate));
    const float vibratoDepth = bp.lfoPitch + modWheel * 0.5f;
    const float ampScale = 1.0f - bp.ampVel + bp.ampVel * velocity;
    const float preGain = 1.0f + bp.drive * 4.0f;
    const float postGain = 1.0f / (1.0f + bp.drive * 1.8f);
    const float nyquistLimit = sampleRate * 0.45f;
    const float piOverFs = juce::MathConstants<float>::pi / sampleRate;

    const float p = std::clamp (pan * bp.spread, -1.0f, 1.0f);
    const float angle = (p + 1.0f) * juce::MathConstants<float>::pi * 0.25f;
    const float gainL = std::cos (angle) * 1.41421356f;
    const float gainR = std::sin (angle) * 1.41421356f;

    for (int i = 0; i < numSamples; ++i)
    {
        // Långsam drift: nya mål ungefär fyra gånger per sekund.
        if (--driftCounter <= 0)
        {
            driftCounter = (int) (sampleRate * (0.15f + 0.2f * rng.unipolar()));
            driftTarget1 = rng.bipolar();
            driftTarget2 = rng.bipolar();
        }
        driftValue1 += (driftTarget1 - driftValue1) * driftSmooth;
        driftValue2 += (driftTarget2 - driftValue2) * driftSmooth;

        pitch = targetPitch + (pitch - targetPitch) * glideCoef;
        lfoFade = std::min (1.0f, lfoFade + fadeInc);

        const float l = lfo[i] * lfoFade;
        const float base = pitch + pitchBendSemis + l * vibratoDepth;

        const float n1 = base + bp.osc1Octave * 12.0f + driftValue1 * driftCents;
        const float n2 = base + bp.osc2Semi + bp.osc2Fine * 0.01f + driftValue2 * driftCents;

        const float dt1 = std::min (midiToHz (n1) / sampleRate, 0.45f);
        const float dt2 = std::min (midiToHz (n2) / sampleRate, 0.45f);

        const float pwMod = l * bp.lfoPW * 0.45f;
        const float pw1 = std::clamp (bp.osc1PW + pwMod, 0.05f, 0.95f);
        const float pw2 = std::clamp (bp.osc2PW + pwMod, 0.05f, 0.95f);

        const float s1 = osc1.process (bp.osc1Wave, dt1, pw1);
        const float s2 = osc2.process (bp.osc2Wave, dt2, pw2);
        const float s3 = sub.process (Oscillator::square, dt1 * 0.5f, 0.5f);
        const float noise = rng.bipolar();

        const float mixed = (s1 * bp.mix1 + s2 * bp.mix2 + s3 * bp.mixSub + noise * bp.mixNoise * 0.7f) * 0.5f;

        const float fe = filterEnv.process();
        const float ae = ampEnv.process();

        const float octaves = bp.envAmount * 7.0f * fe
                            + bp.keyTrack * (pitch - 60.0f) * (1.0f / 12.0f)
                            + bp.filterVel * (velocity - 1.0f) * 3.0f
                            + l * bp.lfoFilter * 3.0f
                            + driftCutoff * bp.drift * 0.1f;

        const float fc = std::clamp (cutoffHz[i] * std::exp2 (octaves), 15.0f, nyquistLimit);
        const float g = std::tan (fc * piOverFs);

        const float y = filter.process (mixed * preGain, g, bp.resonanceK) * postGain;
        const float out = y * ae * ampScale;

        left[i] += out * gainL;
        right[i] += out * gainR;

        if (! ampEnv.isActive())
        {
            filterEnv.reset();
            held = false;
            break;
        }
    }
}

//==============================================================================
void SynthEngine::prepare (double sr, int)
{
    sampleRate = (float) sr;
    uint32_t seed = 0x1234567u;
    for (auto& v : voices)
        v.prepare (sampleRate, seed += 0x9E3779B9u);

    lfoRng.seed (0xC0FFEEu);
    cutoffSmoothed.reset (sr, 0.02);
    cutoffSmoothed.setCurrentAndTargetValue (2500.0f);
    reset();
}

void SynthEngine::reset()
{
    for (auto& v : voices)
        v.reset();

    stackSize = 0;
    sustainPedal = false;
    sustained.fill (false);
    keyDown.fill (false);
    pitchBend = 0.0f;
    modWheel = 0.0f;
    lfoPhase = 0.0f;
    lastPitch = -1.0f;
}

int SynthEngine::getActiveVoiceCount() const noexcept
{
    int n = 0;
    for (auto& v : voices)
        n += v.isActive() ? 1 : 0;
    return n;
}

void SynthEngine::updateBlockParams (const ParamPointers& p)
{
    bp.osc1Wave = (int) p.osc1Wave->load();
    bp.osc1Octave = p.osc1Octave->load();
    bp.osc1PW = p.osc1PW->load();
    bp.osc2Wave = (int) p.osc2Wave->load();
    bp.osc2Semi = p.osc2Semi->load();
    bp.osc2Fine = p.osc2Fine->load();
    bp.osc2PW = p.osc2PW->load();
    bp.mix1 = p.mixOsc1->load();
    bp.mix2 = p.mixOsc2->load();
    bp.mixSub = p.mixSub->load();
    bp.mixNoise = p.mixNoise->load();

    // Resonans 0..1 -> k 0..4.15 (självsvänger längst upp).
    bp.resonanceK = 4.15f * p.resonance->load();
    bp.drive = p.drive->load();
    bp.envAmount = p.filterEnv->load();
    bp.keyTrack = p.keyTrack->load();
    bp.filterVel = p.filterVel->load();

    bp.lfoPitch = p.lfoPitch->load();
    bp.lfoFilter = p.lfoFilter->load();
    bp.lfoPW = p.lfoPW->load();
    bp.lfoFadeSeconds = p.lfoFade->load();
    bp.ampVel = p.ampVel->load();
    bp.drift = p.drift->load();
    bp.spread = p.spread->load();
    bp.glideSeconds = p.glide->load();
    bp.bendRange = p.bendRange->load();

    lfoWave = (int) p.lfoWave->load();
    lfoRate = p.lfoRate->load();
    cutoffSmoothed.setTargetValue (p.cutoff->load());

    const auto newMode = (Mode) juce::jlimit (0, 2, (int) p.voiceMode->load());
    if (newMode != mode)
    {
        allNotesOff (false);
        mode = newMode;
    }

    for (auto& v : voices)
    {
        v.ampEnv.setParameters (sampleRate, p.aAttack->load(), p.aDecay->load(), p.aSustain->load(), p.aRelease->load());
        v.filterEnv.setParameters (sampleRate, p.fAttack->load(), p.fDecay->load(), p.fSustain->load(), p.fRelease->load());
    }
}

void SynthEngine::process (float* left, float* right, int numSamples, const juce::MidiBuffer& midi,
                           int midiOffset, const ParamPointers& params)
{
    updateBlockParams (params);

    int pos = 0;
    const int end = midiOffset + numSamples;

    for (auto it = midi.findNextSamplePosition (midiOffset); it != midi.cend(); ++it)
    {
        const auto meta = *it;
        if (meta.samplePosition >= end)
            break;

        const int eventPos = juce::jlimit (0, numSamples, meta.samplePosition - midiOffset);
        if (eventPos > pos)
        {
            renderSegment (left, right, pos, eventPos - pos);
            pos = eventPos;
        }
        handleMidi (meta.getMessage());
    }

    if (pos < numSamples)
        renderSegment (left, right, pos, numSamples - pos);
}

void SynthEngine::renderSegment (float* left, float* right, int start, int numSamples)
{
    float lfo[controlChunk];
    float cutoff[controlChunk];
    const float bendSemis = pitchBend * bp.bendRange;

    while (numSamples > 0)
    {
        const int n = std::min (numSamples, controlChunk);
        const float inc = lfoRate / sampleRate;

        for (int i = 0; i < n; ++i)
        {
            const float ph = lfoPhase;
            float v = 0.0f;
            switch (lfoWave)
            {
                case 0: v = std::sin (twoPi * ph); break;
                case 1: v = 1.0f - 4.0f * std::abs (ph - 0.5f); break;
                case 2: v = 2.0f * ph - 1.0f; break;
                case 3: v = ph < 0.5f ? 1.0f : -1.0f; break;
                default: v = lfoSH; break;
            }
            lfo[i] = v;

            lfoPhase += inc;
            if (lfoPhase >= 1.0f)
            {
                lfoPhase -= 1.0f;
                lfoSH = lfoRng.bipolar();
            }

            cutoff[i] = cutoffSmoothed.getNextValue();
        }

        for (auto& v : voices)
            v.render (left + start, right + start, n, bp, lfo, cutoff, bendSemis, modWheel);

        start += n;
        numSamples -= n;
    }
}

//==============================================================================
void SynthEngine::handleMidi (const juce::MidiMessage& m)
{
    if (m.isNoteOn())
        noteOn (m.getNoteNumber(), m.getFloatVelocity());
    else if (m.isNoteOff())
        noteOff (m.getNoteNumber());
    else if (m.isPitchWheel())
        pitchBend = juce::jlimit (-1.0f, 1.0f, (float) (m.getPitchWheelValue() - 8192) / 8192.0f);
    else if (m.isControllerOfType (1))
        modWheel = (float) m.getControllerValue() / 127.0f;
    else if (m.isSustainPedalOn())
        sustainPedal = true;
    else if (m.isSustainPedalOff())
    {
        sustainPedal = false;
        for (int n = 0; n < 128; ++n)
        {
            if (sustained[(size_t) n])
            {
                sustained[(size_t) n] = false;
                if (! keyDown[(size_t) n])
                    releaseNote (n);
            }
        }
    }
    else if (m.isAllNotesOff())
        allNotesOff (false);
    else if (m.isAllSoundOff())
        allNotesOff (true);
    else if (m.isResetAllControllers())
    {
        pitchBend = 0.0f;
        modWheel = 0.0f;
    }
}

Voice* SynthEngine::findVoiceFor (int note)
{
    // 1. Samma ton spelas redan -> återanvänd.
    for (auto& v : voices)
        if (v.isActive() && v.getNote() == note)
            return &v;

    // 2. Ledig röst (den som varit tyst längst).
    Voice* best = nullptr;
    for (auto& v : voices)
        if (! v.isActive() && (best == nullptr || v.getAge() < best->getAge()))
            best = &v;
    if (best != nullptr)
        return best;

    // 3. Släppt röst med lägst nivå.
    for (auto& v : voices)
        if (! v.isHeld() && (best == nullptr || v.getLevel() < best->getLevel()))
            best = &v;
    if (best != nullptr)
        return best;

    // 4. Äldsta nedtryckta rösten.
    for (auto& v : voices)
        if (best == nullptr || v.getAge() < best->getAge())
            best = &v;
    return best;
}

void SynthEngine::noteOn (int note, float velocity)
{
    keyDown[(size_t) note] = true;
    sustained[(size_t) note] = false;
    const bool glideOn = bp.glideSeconds > 0.002f;

    if (mode == Mode::poly)
    {
        auto* v = findVoiceFor (note);
        const auto index = (size_t) (v - voices.data());
        v->setPan (voicePans[index]);
        v->start (note, velocity, lastPitch, true, glideOn);
        v->setAge (++voiceCounter);
        lastPitch = (float) note;
        return;
    }

    // Mono / Legato: en röst och en tonstack (senast nedtryckta har prioritet).
    removeFromStack (note);
    if (stackSize < (int) noteStack.size())
    {
        noteStack[(size_t) stackSize] = note;
        noteVelocity[(size_t) stackSize] = velocity;
        ++stackSize;
    }

    auto& v = voices[0];
    v.setPan (0.0f);
    const bool otherNotesHeld = stackSize > 1 || (v.isActive() && v.isHeld());

    if (mode == Mode::legato && otherNotesHeld && v.isActive())
        v.glideTo (note);
    else
        v.start (note, velocity, v.isActive() ? v.getCurrentPitch() : lastPitch, true, glideOn && (v.isActive() || lastPitch >= 0.0f));

    v.setAge (++voiceCounter);
    lastPitch = (float) note;
}

void SynthEngine::noteOff (int note)
{
    keyDown[(size_t) note] = false;

    if (sustainPedal)
    {
        sustained[(size_t) note] = true;
        // I monoläge ska tonen ändå lämna stacken så att äldre toner kan ta över.
        if (mode != Mode::poly)
            removeFromStack (note);
        return;
    }

    releaseNote (note);
}

void SynthEngine::releaseNote (int note)
{
    if (mode == Mode::poly)
    {
        for (auto& v : voices)
            if (v.isActive() && v.isHeld() && v.getNote() == note)
                v.release();
        return;
    }

    removeFromStack (note);
    auto& v = voices[0];

    if (v.getNote() != note)
        return;

    if (stackSize > 0)
    {
        // Gå tillbaka till föregående nedtryckta ton utan att trigga om.
        const int previous = noteStack[(size_t) (stackSize - 1)];
        v.glideTo (previous);
        lastPitch = (float) previous;
    }
    else
    {
        v.release();
    }
}

void SynthEngine::removeFromStack (int note)
{
    int w = 0;
    for (int r = 0; r < stackSize; ++r)
    {
        if (noteStack[(size_t) r] != note)
        {
            noteStack[(size_t) w] = noteStack[(size_t) r];
            noteVelocity[(size_t) w] = noteVelocity[(size_t) r];
            ++w;
        }
    }
    stackSize = w;
}

void SynthEngine::allNotesOff (bool hard)
{
    for (auto& v : voices)
    {
        if (hard)
            v.kill();
        else
            v.release();
    }

    stackSize = 0;
    sustained.fill (false);
    keyDown.fill (false);
}
} // namespace steso::dsp
