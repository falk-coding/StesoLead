// Automatiska tester för StesoLead.
// Kör: StesoLeadTests [--wav-dir <mapp>] [--screenshot <fil.png>]
// - Renderar varje preset (ackord + melodi + pitchbend + modhjul) i 44.1 och 96 kHz
// - Fel om: NaN/Inf, klipp över 1.0, tyst preset, röster som aldrig släpps, för hög CPU
// - Testar mono/legato, sustainpedal, all-notes-off och att sparat läge laddas tillbaka exakt
#include <juce_audio_utils/juce_audio_utils.h>
#include "../Source/PluginProcessor.h"
#include "../Source/PluginEditor.h"


namespace
{
int failures = 0;

void check (bool ok, const juce::String& what)
{
    if (! ok)
    {
        ++failures;
        std::cout << "  FEL: " << what << std::endl;
    }
}

struct RenderResult
{
    float peak = 0.0f, rms = 0.0f;
    bool finite = true;
    double cpuRatio = 0.0;   // renderingstid / realtid
    int voicesAtEnd = 0;
};

using Event = std::pair<double, juce::MidiMessage>;   // tid i sekunder

RenderResult render (StesoLeadProcessor& proc, double sr, int block, double seconds,
                     const std::vector<Event>& events, juce::AudioBuffer<float>* capture)
{
    proc.setRateAndBufferSizeDetails (sr, block);
    proc.prepareToPlay (sr, block);
    proc.reset();

    const int total = (int) (seconds * sr);
    if (capture != nullptr)
        capture->setSize (2, total);

    juce::AudioBuffer<float> buffer (2, block);
    juce::MidiBuffer midi;
    RenderResult res;
    double sumSq = 0.0;
    size_t nextEvent = 0;

    const auto t0 = juce::Time::getHighResolutionTicks();
    for (int pos = 0; pos < total; pos += block)
    {
        const int n = std::min (block, total - pos);
        buffer.setSize (2, n, false, false, true);
        buffer.clear();
        midi.clear();
        while (nextEvent < events.size() && (int) (events[nextEvent].first * sr) < pos + n)
        {
            const int at = std::max (0, (int) (events[nextEvent].first * sr) - pos);
            midi.addEvent (events[nextEvent].second, at);
            ++nextEvent;
        }

        proc.processBlock (buffer, midi);

        for (int ch = 0; ch < 2; ++ch)
        {
            const float* d = buffer.getReadPointer (ch);
            for (int i = 0; i < n; ++i)
            {
                const float x = d[i];
                if (! std::isfinite (x)) res.finite = false;
                res.peak = std::max (res.peak, std::abs (x));
                sumSq += (double) x * x;
            }
            if (capture != nullptr)
                capture->copyFrom (ch, pos, buffer, ch, 0, n);
        }
    }
    const auto t1 = juce::Time::getHighResolutionTicks();

    res.cpuRatio = juce::Time::highResolutionTicksToSeconds (t1 - t0) / seconds;
    res.rms = (float) std::sqrt (sumSq / (2.0 * total));
    res.voicesAtEnd = proc.getActiveVoiceCount();
    return res;
}

std::vector<Event> musicalPhrase()
{
    std::vector<Event> e;
    auto on  = [&e] (double t, int n, float v) { e.push_back ({ t, juce::MidiMessage::noteOn (1, n, v) }); };
    auto off = [&e] (double t, int n) { e.push_back ({ t, juce::MidiMessage::noteOff (1, n) }); };

    // Ackord C-moll
    on (0.0, 48, 0.9f); on (0.0, 55, 0.7f); on (0.0, 60, 0.8f); on (0.0, 63, 0.6f);
    off (1.5, 48); off (1.5, 55); off (1.5, 60); off (1.5, 63);

    // Legatomelodi med överlapp
    const int notes[] { 72, 74, 75, 79, 77, 75, 74, 72 };
    double t = 1.8;
    for (size_t i = 0; i < std::size (notes); ++i)
    {
        on (t, notes[i], 0.5f + 0.06f * (float) i);
        off (t + 0.3, notes[i]);
        t += 0.25;
    }

    // Pitchbend och modhjul på en lång ton
    on (4.0, 67, 1.0f);
    for (int i = 0; i <= 20; ++i)
    {
        e.push_back ({ 4.1 + i * 0.03, juce::MidiMessage::pitchWheel (1, 8192 + i * 300) });
        e.push_back ({ 4.1 + i * 0.03, juce::MidiMessage::controllerEvent (1, 1, i * 6) });
    }
    e.push_back ({ 5.0, juce::MidiMessage::pitchWheel (1, 8192) });
    e.push_back ({ 5.0, juce::MidiMessage::controllerEvent (1, 1, 0) });
    off (5.5, 67);

    // Fler toner än röster (röststöld)
    for (int i = 0; i < 12; ++i)
        on (6.0 + i * 0.01, 40 + i * 3, 0.8f);
    for (int i = 0; i < 12; ++i)
        off (6.8, 40 + i * 3);

    std::sort (e.begin(), e.end(), [] (const Event& a, const Event& b) { return a.first < b.first; });
    return e;
}

void writeWav (const juce::File& file, const juce::AudioBuffer<float>& buffer, double sr)
{
    file.deleteFile();
    juce::WavAudioFormat wav;
    std::unique_ptr<juce::OutputStream> stream (file.createOutputStream().release());
    if (stream == nullptr)
        return;
    auto options = juce::AudioFormatWriterOptions().withSampleRate (sr).withNumChannels (2).withBitsPerSample (24);
    if (auto writer = wav.createWriterFor (stream, options))
        writer->writeFromAudioSampleBuffer (buffer, 0, buffer.getNumSamples());
}

void testPresets (const juce::File& wavDir)
{
    std::cout << "Presets:" << std::endl;
    std::unique_ptr<StesoLeadProcessor> proc (dynamic_cast<StesoLeadProcessor*> (createPluginFilter()));
    const auto phrase = musicalPhrase();
    double worstCpu = 0.0;

    for (int i = 0; i < proc->getNumPrograms(); ++i)
    {
        proc->setCurrentProgram (i);
        const auto name = proc->getProgramName (i);

        juce::AudioBuffer<float> capture;
        // 14 s: frasen tar ~7 s, resten låter release/delay/reverb klinga ut.
        const auto r = render (*proc, 44100.0, 256, 14.0, phrase, wavDir.exists() ? &capture : nullptr);
        const auto r96 = render (*proc, 96000.0, 333, 8.0, phrase, nullptr);
        worstCpu = std::max ({ worstCpu, r.cpuRatio, r96.cpuRatio });

        std::cout << "  " << juce::String (i + 1).paddedLeft ('0', 2) << " " << name.paddedRight (' ', 20)
                  << " peak " << juce::String (r.peak, 3) << "  rms " << juce::String (r.rms, 4)
                  << "  cpu " << juce::String (r.cpuRatio * 100.0, 1) << "%" << std::endl;

        check (r.finite && r96.finite, name + ": NaN/Inf i utsignalen");
        check (r.peak <= 1.0f && r96.peak <= 1.0f, name + ": klipper över 0 dBFS");
        check (r.rms > 0.005f, name + ": för tyst (rms " + juce::String (r.rms) + ")");
        check (r.peak > 0.05f, name + ": för låg topp");
        check (r.voicesAtEnd == 0, name + ": " + juce::String (r.voicesAtEnd) + " röster ljuder fortfarande efter release");

        if (wavDir.exists())
            writeWav (wavDir.getChildFile (juce::String (i + 1).paddedLeft ('0', 2) + " " + name + ".wav"), capture, 44100.0);
    }

    std::cout << "  Värsta CPU (en kärna): " << juce::String (worstCpu * 100.0, 1) << "% av realtid" << std::endl;
    check (worstCpu < 0.5, "CPU-last över 50 % av realtid");
}

void testVoiceModesAndPedal()
{
    std::cout << "Röstlägen och sustainpedal:" << std::endl;
    std::unique_ptr<StesoLeadProcessor> proc (dynamic_cast<StesoLeadProcessor*> (createPluginFilter()));

    auto setParam = [&] (const char* id, float value)
    {
        auto* p = proc->apvts.getParameter (id);
        p->setValueNotifyingHost (p->convertTo0to1 (value));
    };

    for (float mode : { 1.0f, 2.0f })
    {
        setParam (steso::pid::voiceMode, mode);
        setParam (steso::pid::glide, 0.1f);
        std::vector<Event> e {
            { 0.0, juce::MidiMessage::noteOn (1, 60, 0.8f) },
            { 0.2, juce::MidiMessage::noteOn (1, 64, 0.8f) },
            { 0.4, juce::MidiMessage::noteOn (1, 67, 0.8f) },
            { 0.6, juce::MidiMessage::noteOff (1, 67) },
            { 0.8, juce::MidiMessage::noteOff (1, 64) },
            { 1.0, juce::MidiMessage::noteOff (1, 60) },
        };
        const auto r = render (*proc, 48000.0, 128, 4.0, e, nullptr);
        check (r.finite && r.peak > 0.05f && r.peak <= 1.0f, "monoläge " + juce::String (mode) + " låter inte rätt");
        check (r.voicesAtEnd == 0, "monoläge " + juce::String (mode) + ": rösten släpps inte");
    }

    setParam (steso::pid::voiceMode, 0.0f);
    {
        // Pedal ner, tona, släpp tangent -> ska fortfarande ljuda tills pedalen släpps.
        std::vector<Event> e {
            { 0.0, juce::MidiMessage::controllerEvent (1, 64, 127) },
            { 0.1, juce::MidiMessage::noteOn (1, 60, 0.8f) },
            { 0.3, juce::MidiMessage::noteOff (1, 60) },
        };
        auto r = render (*proc, 48000.0, 128, 2.0, e, nullptr);
        check (r.voicesAtEnd == 1, "sustainpedal håller inte tonen");

        e.push_back ({ 2.1, juce::MidiMessage::controllerEvent (1, 64, 0) });
        r = render (*proc, 48000.0, 128, 4.0, e, nullptr);
        check (r.voicesAtEnd == 0, "tonen släpps inte när pedalen släpps");
    }
    {
        std::vector<Event> e {
            { 0.0, juce::MidiMessage::noteOn (1, 60, 0.8f) },
            { 0.0, juce::MidiMessage::noteOn (1, 64, 0.8f) },
            { 0.5, juce::MidiMessage::allSoundOff (1) },
        };
        const auto r = render (*proc, 48000.0, 128, 0.7, e, nullptr);
        check (r.voicesAtEnd == 0, "All Sound Off stänger inte av rösterna");
    }
    std::cout << "  klart" << std::endl;
}

void testState()
{
    std::cout << "Spara/ladda läge:" << std::endl;
    std::unique_ptr<StesoLeadProcessor> a (dynamic_cast<StesoLeadProcessor*> (createPluginFilter()));
    std::unique_ptr<StesoLeadProcessor> b (dynamic_cast<StesoLeadProcessor*> (createPluginFilter()));

    a->setCurrentProgram (3);
    auto* cutoff = a->apvts.getParameter (steso::pid::cutoff);
    cutoff->setValueNotifyingHost (0.37f);

    juce::MemoryBlock state;
    a->getStateInformation (state);
    b->setStateInformation (state.getData(), (int) state.getSize());

    check (b->getCurrentProgram() == 3, "presetnummer följer inte med");
    for (auto* p : a->getParameters())
    {
        auto* rp = dynamic_cast<juce::RangedAudioParameter*> (p);
        auto* other = b->apvts.getParameter (rp->getParameterID());
        check (other != nullptr && std::abs (other->getValue() - rp->getValue()) < 1.0e-6f,
               "parametern " + rp->getParameterID() + " återställs inte");
    }
    // Som pluginval: slumpade (även "mellanliggande") normaliserade värden på alla parametrar.
    juce::Random rnd (1234);
    for (auto* p : a->getParameters())
        p->setValueNotifyingHost (rnd.nextFloat());
    std::vector<float> saved;
    for (auto* p : a->getParameters())
        saved.push_back (p->getValue());
    juce::MemoryBlock fuzzState;
    a->getStateInformation (fuzzState);
    for (auto* p : a->getParameters())
        p->setValueNotifyingHost (rnd.nextFloat());
    a->setStateInformation (fuzzState.getData(), (int) fuzzState.getSize());
    for (size_t i = 0; i < saved.size(); ++i)
    {
        auto* p = a->getParameters()[(int) i];
        check (std::abs (p->getValue() - saved[i]) < 0.1f, p->getName (32) + " återställs inte från sparat läge (som pluginval)");
    }

    std::cout << "  " << a->getParameters().size() << " parametrar kontrollerade" << std::endl;
}


struct TestPlayHead : juce::AudioPlayHead
{
    double ppq = 0.0, bpm = 120.0;
    bool playing = true;

    juce::Optional<PositionInfo> getPosition() const override
    {
        PositionInfo p;
        p.setPpqPosition (ppq);
        p.setBpm (bpm);
        p.setIsPlaying (playing);
        p.setTimeSignature (TimeSignature { 4, 4 });
        p.setLoopPoints (LoopPoints { 16.0, 48.0 });
        return p;
    }
};

struct SeqNote { double ppq; int note; };

// Kör processorn med transport från ppq 0. Returnerar MIDI-ut (note on) med position i ppq.
std::vector<SeqNote> runTransport (StesoLeadProcessor& proc, TestPlayHead& ph, double bars,
                                   const std::vector<int>& heldNotes, int& onCount, int& offCount)
{
    const double sr = 48000.0;
    const int block = 256;
    proc.setPlayHead (&ph);
    proc.setRateAndBufferSizeDetails (sr, block);
    proc.prepareToPlay (sr, block);

    const double ppqPerSample = ph.bpm / 60.0 / sr;
    const int total = (int) (bars * 4.0 / ppqPerSample);
    juce::AudioBuffer<float> buffer (2, block);
    juce::MidiBuffer midi;
    std::vector<SeqNote> result;
    onCount = offCount = 0;

    for (int pos = 0; pos < total; pos += block)
    {
        ph.ppq = pos * ppqPerSample;
        ph.playing = true;
        buffer.clear();
        midi.clear();
        if (pos == 0)
            for (int n : heldNotes)
                midi.addEvent (juce::MidiMessage::noteOn (1, n, 0.9f), 0);

        proc.processBlock (buffer, midi);
        for (const auto meta : midi)
        {
            const auto m = meta.getMessage();
            if (m.isNoteOn())
            {
                ++onCount;
                result.push_back ({ ph.ppq + meta.samplePosition * ppqPerSample, m.getNoteNumber() });
            }
            else if (m.isNoteOff())
                ++offCount;
        }
    }

    // Stoppa transporten och släpp tangenterna -> allt ska tystna.
    ph.playing = false;
    for (int i = 0; i < 400; ++i)
    {
        buffer.clear();
        midi.clear();
        if (i == 0)
            for (int n : heldNotes)
                midi.addEvent (juce::MidiMessage::noteOff (1, n), 0);
        proc.processBlock (buffer, midi);
        for (const auto meta : midi)
            if (meta.getMessage().isNoteOff())
                ++offCount;
    }
    proc.setPlayHead (nullptr);
    return result;
}

std::vector<SeqNote> readMidiFile (const juce::File& f, double offsetPpq)
{
    std::vector<SeqNote> out;
    juce::FileInputStream in (f);
    juce::MidiFile mf;
    if (! in.openedOk() || ! mf.readFrom (in))
        return out;
    const double tpq = mf.getTimeFormat();
    for (int t = 0; t < mf.getNumTracks(); ++t)
        for (const auto* e : *mf.getTrack (t))
            if (e->message.isNoteOn())
                out.push_back ({ offsetPpq + e->message.getTimeStamp() / tpq, e->message.getNoteNumber() });
    return out;
}

bool sameNotes (const std::vector<SeqNote>& a, const std::vector<SeqNote>& b, juce::String& why)
{
    if (a.size() != b.size())
    {
        why = "antal " + juce::String ((int) a.size()) + " mot " + juce::String ((int) b.size());
        return false;
    }
    for (size_t i = 0; i < a.size(); ++i)
    {
        if (a[i].note != b[i].note || std::abs (a[i].ppq - b[i].ppq) > 0.003)
        {
            why = "ton " + juce::String ((int) i) + ": " + juce::String (a[i].note) + "@" + juce::String (a[i].ppq, 4)
                + " mot " + juce::String (b[i].note) + "@" + juce::String (b[i].ppq, 4);
            return false;
        }
    }
    return true;
}

int findPreset (StesoLeadProcessor& p, const juce::String& name)
{
    for (int i = 0; i < p.getNumPrograms(); ++i)
        if (p.getProgramName (i) == name)
            return i;
    return -1;
}

void testSequencer()
{
    std::cout << "Sekvenser:" << std::endl;
    std::unique_ptr<StesoLeadProcessor> proc (dynamic_cast<StesoLeadProcessor*> (createPluginFilter()));
    TestPlayHead ph;

    auto setParam = [&] (const char* id, float value)
    {
        auto* p = proc->apvts.getParameter (id);
        p->setValueNotifyingHost (p->convertTo0to1 (value));
    };

    // 1. Seq-läge, 7 steg mot 4/4: MIDI ut ska vara identiskt med exporten.
    const int acid = findPreset (*proc, "Odd Seven Acid");
    check (acid >= 0, "presetet Odd Seven Acid saknas");
    proc->setCurrentProgram (acid);
    setParam (steso::pid::seqMidiOut, 1.0f);

    int ons = 0, offs = 0;
    const auto live = runTransport (*proc, ph, 8.0, { 48 }, ons, offs);
    check (ons > 20, "sekvensern spelar för få toner (" + juce::String (ons) + ")");
    check (ons == offs, "MIDI ut: " + juce::String (ons) + " note on men " + juce::String (offs) + " note off");
    check (proc->getActiveVoiceCount() == 0, "röster hänger kvar efter stopp");

    // Polymetri: steg 16 (takt 2, slag 1) = mönstersteg 16 % 7 = 2 -> +12 halvtoner.
    bool foundBar2 = false;
    for (const auto& n : live)
        if (std::abs (n.ppq - 4.0) < 0.003)
            foundBar2 = (n.note == 60);
    check (foundBar2, "takt 2 börjar inte på mönstersteg 3 (+12) – polymetrin stämmer inte");

    juce::String why;
    const auto fileAll = proc->writeMidiExport (0.0, 32.0, "test-alla.mid");
    check (fileAll.existsAsFile(), "MIDI-exporten skapade ingen fil");
    check (sameNotes (live, readMidiFile (fileAll, 0.0), why), "export takt 1-9 skiljer sig från uppspelningen: " + why);

    // Export mitt i låten (takt 5-9) ska matcha samma del av uppspelningen.
    std::vector<SeqNote> liveMid;
    for (const auto& n : live)
        if (n.ppq >= 16.0 - 1.0e-6 && n.ppq < 32.0)
            liveMid.push_back (n);
    const auto fileMid = proc->writeMidiExport (16.0, 32.0, "test-mitt.mid");
    check (sameNotes (liveMid, readMidiFile (fileMid, 16.0), why), "export takt 5-9 skiljer sig: " + why);

    // Locators från DAW:en läses av.
    const auto host = proc->getHostInfo();
    check (host.hasLoop && std::abs (host.loopStart - 16.0) < 1e-9 && std::abs (host.loopEnd - 48.0) < 1e-9,
           "locators från DAW:en lästes inte");

    // 2. Bypass efter dragning, och tillbaka.
    proc->sequencerExported();
    check ((int) proc->apvts.getRawParameterValue (steso::pid::seqMode)->load() == 0, "sekvensern stängs inte av efter export");
    check (proc->isBypassedByExport(), "bypass-flaggan sätts inte");
    proc->reenableSequencer();
    check ((int) proc->apvts.getRawParameterValue (steso::pid::seqMode)->load() == 2, "Slå på igen återställer inte Seq-läget");

    // I bypass ska inkommande toner spela synthen direkt (och inget MIDI ut).
    proc->sequencerExported();
    {
        int o = 0, f = 0;
        const auto direct = runTransport (*proc, ph, 1.0, { 60 }, o, f);
        check (o == 0, "MIDI ut skickas trots att sekvensern är av");
    }

    // 3. Arp, 5 steg upp/ner över 2 oktaver: bara ackordets toner.
    const int arp = findPreset (*proc, "Five Over Four Arp");
    proc->setCurrentProgram (arp);
    setParam (steso::pid::seqMidiOut, 1.0f);
    const auto arpNotes = runTransport (*proc, ph, 4.0, { 60, 64, 67 }, ons, offs);
    bool allInChord = ! arpNotes.empty();
    for (const auto& n : arpNotes)
    {
        const int pc = n.note % 12;
        allInChord = allInChord && (pc == 0 || pc == 4 || pc == 7) && n.note >= 60 && n.note < 84;
    }
    check (allInChord, "arpen spelar toner utanför ackordet");
    check (ons == offs && proc->getActiveVoiceCount() == 0, "arpen lämnar hängande toner");

    // 4. Mönstret sparas med projektet.
    proc->getPattern().pitch[3] = 5;
    proc->getPattern().on[4] = false;
    juce::MemoryBlock state;
    proc->getStateInformation (state);
    std::unique_ptr<StesoLeadProcessor> other (dynamic_cast<StesoLeadProcessor*> (createPluginFilter()));
    other->setStateInformation (state.getData(), (int) state.getSize());
    check (other->getPattern().pitch[3].load() == 5 && ! other->getPattern().on[4].load(), "mönstret sparas inte med projektet");

    // 5. Euklid 5 av 8 = 5 träffar.
    steso::seq::Pattern pat;
    pat.applyEuclid (5, 8, 0);
    int hits = 0;
    for (int i = 0; i < 8; ++i)
        hits += pat.on[(size_t) i].load() ? 1 : 0;
    check (hits == 5, "euklid 5/8 ger " + juce::String (hits) + " träffar");

    std::cout << "  " << live.size() << " toner i 8 takter, export = uppspelning" << std::endl;
}

void testStress()
{
    std::cout << "Stresstest (slumpade inställningar):" << std::endl;
    std::unique_ptr<StesoLeadProcessor> proc (dynamic_cast<StesoLeadProcessor*> (createPluginFilter()));
    juce::Random rnd (777);
    std::vector<Event> e;
    for (int n : { 28, 40, 52, 64, 76, 88 })
        e.push_back ({ 0.0, juce::MidiMessage::noteOn (1, n, 1.0f) });
    for (int i = 0; i <= 20; ++i)
        e.push_back ({ 0.05 + i * 0.05, juce::MidiMessage::pitchWheel (1, i * 780) });
    for (int n : { 28, 40, 52, 64, 76, 88 })
        e.push_back ({ 1.2, juce::MidiMessage::noteOff (1, n) });

    int worst = 0;
    double worstCpu = 0.0;
    for (int round = 0; round < 40; ++round)
    {
        for (auto* p : proc->getParameters())
            p->setValueNotifyingHost (rnd.nextFloat());
        // Ingen sekvenser här (testas separat), men extrema filter/feedback/unison.
        proc->apvts.getParameter (steso::pid::seqMode)->setValueNotifyingHost (0.0f);
        if (round % 3 == 0)
        {
            for (const char* id : { steso::pid::resonance, steso::pid::drive, steso::pid::feedback, steso::pid::warmth })
                proc->apvts.getParameter (id)->setValueNotifyingHost (1.0f);
            proc->apvts.getParameter (steso::pid::unison)->setValueNotifyingHost (1.0f);
        }

        const auto r = render (*proc, round % 2 == 0 ? 44100.0 : 96000.0, 197, 1.6, e, nullptr);
        worstCpu = std::max (worstCpu, r.cpuRatio);
        if (! r.finite || r.peak > 1.0001f)   // softClip når exakt 1.0 (+ flyttalsavrundning)
        {
            ++worst;
            check (false, "runda " + juce::String (round) + ": " + (r.finite ? "klipper" : "NaN/Inf"));
        }
    }
    std::cout << "  40 rundor, värsta CPU " << juce::String (worstCpu * 100.0, 1) << "% av realtid" << std::endl;
    check (worstCpu < 0.6, "CPU för hög i värsta fallet");
}

void screenshot (const juce::File& file)
{
    std::unique_ptr<StesoLeadProcessor> proc (dynamic_cast<StesoLeadProcessor*> (createPluginFilter()));
    proc->setCurrentProgram (findPreset (*proc, "Three Osc Lead"));
    std::unique_ptr<juce::AudioProcessorEditor> editor (proc->createEditor());
    editor->setSize (StesoLeadEditor::baseWidth, StesoLeadEditor::baseHeight);
    const auto image = editor->createComponentSnapshot (editor->getLocalBounds(), true, 1.0f);
    file.deleteFile();
    juce::FileOutputStream out (file);
    juce::PNGImageFormat png;
    check (out.openedOk() && png.writeImageToStream (image, out), "kunde inte spara skärmdump");
    std::cout << "Skärmdump: " << file.getFullPathName() << std::endl;
}
} // namespace

int main (int argc, char** argv)
{
    juce::ScopedJuceInitialiser_GUI gui;
    juce::File wavDir, shot;

    for (int i = 1; i + 1 < argc; ++i)
    {
        const juce::String a (argv[i]);
        if (a == "--wav-dir")    wavDir = juce::File::getCurrentWorkingDirectory().getChildFile (argv[++i]);
        if (a == "--screenshot") shot   = juce::File::getCurrentWorkingDirectory().getChildFile (argv[++i]);
    }
    if (wavDir != juce::File())
        wavDir.createDirectory();

    testPresets (wavDir);
    testVoiceModesAndPedal();
    testState();
    testSequencer();
    testStress();
    if (shot != juce::File())
        screenshot (shot);

    std::cout << (failures == 0 ? "ALLA TESTER OK" : juce::String (failures) + " FEL") << std::endl;
    return failures == 0 ? 0 : 1;
}
