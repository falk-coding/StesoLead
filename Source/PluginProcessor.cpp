#include "PluginProcessor.h"
#include "PluginEditor.h"

StesoLeadProcessor::StesoLeadProcessor()
    : AudioProcessor (BusesProperties().withOutput ("Output", juce::AudioChannelSet::stereo(), true)),
      apvts (*this, nullptr, "StesoLead", steso::createParameterLayout())
{
    params.bind (apvts);
    work.setSize (2, workSize);
    genEvents.reserve (256);
    outEvents.reserve (512);
    for (auto& n : exportNotes)
        n = 60;
    exportCount = 1;

    // Första ljudet när pluginet öppnas: ett fett lead (värden kan alltid välja annat).
    const auto& presets = steso::getFactoryPresets();
    for (size_t i = 0; i < presets.size(); ++i)
        if (presets[i].name == "Three Osc Lead")
            setCurrentProgram ((int) i);
}

bool StesoLeadProcessor::isBusesLayoutSupported (const BusesLayout& layouts) const
{
    const auto out = layouts.getMainOutputChannelSet();
    if (out != juce::AudioChannelSet::stereo() && out != juce::AudioChannelSet::mono())
        return false;
    return layouts.getMainInputChannelSet().isDisabled();
}

void StesoLeadProcessor::prepareToPlay (double sr, int samplesPerBlock)
{
    sampleRate = sr;
    engine.prepare (sr, samplesPerBlock);
    chorus.prepare (sr);
    delay.prepare (sr);
    reverb.prepare (sr);
    dcL.prepare ((float) sr);
    dcR.prepare ((float) sr);
    masterGain.reset (sr, 0.05);
    masterGain.setCurrentAndTargetValue (juce::Decibels::decibelsToGain (params.master->load()));
    work.setSize (2, workSize, false, false, true);
    engineMidi.ensureSize (4096);
    seqOutMidi.ensureSize (4096);
    keyboardState.reset();
    reset();
}

void StesoLeadProcessor::reset()
{
    engine.reset();
    chorus.reset();
    delay.reset();
    reverb.reset();
    dcL.reset();
    dcR.reset();
    noteStream = {};
    physical = {};
    latched = {};
    hostWasPlaying = freeRunning = false;
    currentStep = -1;
}

//==============================================================================
steso::seq::Settings StesoLeadProcessor::currentSeqSettings() const
{
    using namespace steso::seq;
    Settings s;
    s.mode = (Mode) juce::jlimit (0, 2, (int) params.seqMode->load());
    s.stepPpq = rateToPpq ((int) params.seqRate->load());
    s.swing = params.seqSwing->load();
    s.gateScale = params.seqGate->load();
    s.arpOrder = (ArpOrder) juce::jlimit (0, 4, (int) params.arpOrder->load());
    s.arpOctaves = (int) params.arpOctaves->load();
    s.length = (int) params.seqLength->load();
    s.accentLength = (int) params.seqAccLen->load();
    s.gateLength = (int) params.seqGateLen->load();

    s.barPpq = hostTsNum.load() * 4.0 / juce::jmax (1, hostTsDen.load());
    s.polyBars = (int) params.polyBars->load();
    for (size_t l = 0; l < (size_t) polyLanes; ++l)
    {
        const auto& P = params.polyLane[l];
        auto& L = s.lanes[l];
        L.on = P.on->load() > 0.5f;
        L.type = (int) P.type->load();
        L.pulses = (int) P.pulses->load();
        L.hits = (int) P.hits->load();
        L.pitch = (int) P.pitch->load();
        L.steps = (int) P.steps->load();
    }
    if (params.polyOn->load() > 0.5f)
        s.mode = Mode::poly;
    return s;
}

int StesoLeadProcessor::effectiveSeqMode() const noexcept
{
    return params.polyOn->load() > 0.5f ? 3 : (int) params.seqMode->load();
}

StesoLeadProcessor::HostInfo StesoLeadProcessor::getHostInfo() const
{
    HostInfo h;
    h.hasLoop = hostHasLoop.load();
    h.loopStart = hostLoopStart.load();
    h.loopEnd = hostLoopEnd.load();
    h.timeSigNum = hostTsNum.load();
    h.timeSigDen = hostTsDen.load();
    h.bpm = hostBpmAtomic.load();
    return h;
}

steso::seq::Chord StesoLeadProcessor::getExportChord() const
{
    steso::seq::Chord c;
    c.count = juce::jlimit (1, steso::seq::maxHeld, exportCount.load());
    for (int i = 0; i < c.count; ++i)
        c.notes[(size_t) i] = exportNotes[(size_t) i].load();
    return c;
}

juce::File StesoLeadProcessor::writeMidiExport (double fromPpq, double toPpq, const juce::String& fileName) const
{
    auto settings = currentSeqSettings();
    if (settings.mode == steso::seq::Mode::off)
        settings.mode = polyBeforeExport ? steso::seq::Mode::poly
                                         : (steso::seq::Mode) juce::jlimit (1, 2, modeBeforeExport);

    const auto host = getHostInfo();
    const auto midiFile = steso::seq::renderMidiFile (pattern, settings, getExportChord(), fromPpq, toPpq,
                                                      host.bpm, host.timeSigNum, host.timeSigDen);

    auto dir = juce::File::getSpecialLocation (juce::File::tempDirectory).getChildFile ("Bakelit");
    dir.createDirectory();
    auto file = dir.getChildFile (juce::File::createLegalFileName (fileName));
    file.deleteFile();

    juce::FileOutputStream out (file);
    if (! out.openedOk() || ! midiFile.writeTo (out, 1))
        return {};
    out.flush();
    return file;
}

void StesoLeadProcessor::sequencerExported()
{
    if (effectiveSeqMode() == 0)
    {
        bypassedByExport = true;   // redan av (t.ex. andra dragningen i rad) – behåll det vi minns
        return;
    }

    const int mode = (int) params.seqMode->load();
    modeBeforeExport = mode;
    polyBeforeExport = params.polyOn->load() > 0.5f;

    for (auto* id : { steso::pid::seqMode, steso::pid::polyOn })
        if (auto* p = apvts.getParameter (id))
            p->setValueNotifyingHost (p->convertTo0to1 (0.0f));
    bypassedByExport = true;
}

void StesoLeadProcessor::reenableSequencer()
{
    if (polyBeforeExport)
    {
        if (auto* p = apvts.getParameter (steso::pid::polyOn))
            p->setValueNotifyingHost (1.0f);
        if (auto* p = apvts.getParameter (steso::pid::seqMode))
            p->setValueNotifyingHost (p->convertTo0to1 ((float) juce::jlimit (0, 2, modeBeforeExport)));
    }
    else if (auto* p = apvts.getParameter (steso::pid::seqMode))
    {
        p->setValueNotifyingHost (p->convertTo0to1 ((float) juce::jlimit (1, 2, modeBeforeExport)));
    }
    bypassedByExport = false;
}

//==============================================================================
void StesoLeadProcessor::notePressed (int note)
{
    if (physical.count == 0)
        latched.count = 0;   // nytt ackord ersätter det gamla

    // Ta bort dublett, lägg sist.
    int w = 0;
    for (int i = 0; i < physical.count; ++i)
        if (physical.notes[(size_t) i] != note)
            physical.notes[(size_t) w++] = physical.notes[(size_t) i];
    physical.count = w;
    if (physical.count < steso::seq::maxHeld)
        physical.notes[(size_t) physical.count++] = note;

    latched = physical;
    exportCount = physical.count;
    for (int i = 0; i < physical.count; ++i)
        exportNotes[(size_t) i] = physical.notes[(size_t) i];
}

void StesoLeadProcessor::noteReleased (int note)
{
    int w = 0;
    for (int i = 0; i < physical.count; ++i)
        if (physical.notes[(size_t) i] != note)
            physical.notes[(size_t) w++] = physical.notes[(size_t) i];
    physical.count = w;
}

void StesoLeadProcessor::flushSequencer (juce::MidiBuffer& toEngine, juce::MidiBuffer& toOut)
{
    outEvents.clear();
    noteStream.flushAll (0.0, outEvents);
    for (const auto& o : outEvents)
    {
        const auto m = juce::MidiMessage::noteOff (1, o.note);
        toEngine.addEvent (m, 0);
        toOut.addEvent (m, 0);
    }
}

void StesoLeadProcessor::runSequencer (int numSamples, juce::MidiBuffer& toEngine, juce::MidiBuffer& toOut)
{
    const double ppqPerSample = hostBpm / 60.0 / sampleRate;
    const bool latch = params.seqLatch->load() > 0.5f;
    double start = 0.0;
    bool run = false, continuous = false;

    if (hostPlaying && hostPpq.hasValue())
    {
        start = *hostPpq;
        // Hopp i transporten (loop, flytt av spelhuvudet) -> släpp klingande toner.
        if (! hostWasPlaying || std::abs (start - expectedNextPpq) > 1.0e-3)
            flushSequencer (toEngine, toOut);
        else
            continuous = true;
        freeRunning = false;
        run = true;
    }
    else
    {
        if (hostWasPlaying)
            flushSequencer (toEngine, toOut);

        // Ingen transport: kör på egen klocka så länge tangenter hålls ner.
        if (physical.count > 0)
        {
            if (! freeRunning)
            {
                freeRunning = true;
                freePpq = 0.0;
                flushSequencer (toEngine, toOut);
            }
            start = freePpq;
            run = true;
        }
        else if (freeRunning)
        {
            freeRunning = false;
        }
    }
    hostWasPlaying = hostPlaying;

    if (! run)
    {
        // Låt redan startade toner få sitt slut även när klockan står.
        flushSequencer (toEngine, toOut);
        currentStep = -1;
        currentPpq = -1.0;
        return;
    }

    const double end = start + numSamples * ppqPerSample;
    // Generera kant i kant med förra blocket: värdens position kan avrunda en aning olika från block
    // till block, och ett slag precis på gränsen får varken spelas två gånger eller tappas.
    const double genStart = continuous ? expectedNextPpq : start;
    expectedNextPpq = end;
    if (freeRunning)
        freePpq = end;

    const auto settings = currentSeqSettings();
    const steso::seq::Chord& chord = physical.count > 0 ? physical
                                   : ((latch && hostPlaying) ? latched : steso::seq::Chord {});

    genEvents.clear();
    outEvents.clear();
    steso::seq::generate (pattern, settings, chord, genStart, end, genEvents);
    for (const auto& e : genEvents)
    {
        noteStream.flushUntil (e.ppqOn, outEvents);
        noteStream.add (e, outEvents);
    }
    noteStream.flushUntil (end, outEvents);

    for (const auto& o : outEvents)
    {
        const int offset = juce::jlimit (0, numSamples - 1, (int) std::floor ((o.ppq - start) / ppqPerSample));
        const auto m = o.isOn ? juce::MidiMessage::noteOn (1, o.note, o.velocity) : juce::MidiMessage::noteOff (1, o.note);
        toEngine.addEvent (m, offset);
        toOut.addEvent (m, offset);
    }

    currentStep = (int64_t) std::floor (start / settings.stepPpq);
    currentPpq = start;
}

//==============================================================================
void StesoLeadProcessor::processBlock (juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midi)
{
    juce::ScopedNoDenormals noDenormals;
    const int numSamples = buffer.getNumSamples();
    const int numChannels = buffer.getNumChannels();

    // Lägg till toner från GUI-klaviaturen.
    keyboardState.processNextMidiBuffer (midi, 0, numSamples, true);

    // Transport och tempo från DAW:en.
    hostPlaying = false;
    hostPpq = {};
    if (auto* ph = getPlayHead())
    {
        if (auto pos = ph->getPosition())
        {
            hostPlaying = pos->getIsPlaying();
            hostPpq = pos->getPpqPosition();
            hostBpm = pos->getBpm().orFallback (hostBpm);
            if (auto loop = pos->getLoopPoints(); loop.hasValue() && loop->ppqEnd > loop->ppqStart)
            {
                hostLoopStart = loop->ppqStart;
                hostLoopEnd = loop->ppqEnd;
                hostHasLoop = true;
            }
            if (auto ts = pos->getTimeSignature())
            {
                hostTsNum = juce::jmax (1, ts->numerator);
                hostTsDen = juce::jmax (1, ts->denominator);
            }
        }
    }
    hostBpm = juce::jlimit (20.0, 999.0, hostBpm);
    hostBpmAtomic = hostBpm;

    const int seqMode = effectiveSeqMode();
    engineMidi.clear();
    seqOutMidi.clear();

    if (seqMode != lastSeqMode)
    {
        // Byte av läge: släpp allt så att inga toner hänger kvar.
        flushSequencer (engineMidi, seqOutMidi);
        engineMidi.addEvent (juce::MidiMessage::allNotesOff (1), 0);
        freeRunning = false;
        hostWasPlaying = false;
        lastSeqMode = seqMode;
    }

    for (const auto meta : midi)
    {
        const auto m = meta.getMessage();
        if (m.isNoteOn())
        {
            notePressed (m.getNoteNumber());
            if (seqMode == 0)
                engineMidi.addEvent (m, meta.samplePosition);
        }
        else if (m.isNoteOff())
        {
            noteReleased (m.getNoteNumber());
            if (seqMode == 0)
                engineMidi.addEvent (m, meta.samplePosition);
        }
        else
        {
            engineMidi.addEvent (m, meta.samplePosition);
        }
    }

    if (seqMode != 0)
        runSequencer (numSamples, engineMidi, seqOutMidi);
    else
    {
        currentStep = -1;
        currentPpq = -1.0;
    }

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

        engine.process (l, r, n, engineMidi, offset, params);
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

    // MIDI ut: sekvenserns toner (för DAW:ar som kan spela in MIDI från instrument).
    midi.clear();
    if (seqMode != 0 && params.seqMidiOut->load() > 0.5f)
        midi.swapWith (seqOutMidi);
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
    steso::applyPreset (apvts, presets[(size_t) index], pattern);
    bypassedByExport = false;
    userPresetName = {};
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
    state.setProperty ("userPreset", userPresetName, nullptr);
    state.removeChild (state.getChildWithName ("Pattern"), nullptr);
    state.appendChild (pattern.toTree(), nullptr);
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
            userPresetName = tree.getProperty ("userPreset", juce::String()).toString();
            const auto patternTree = tree.getChildWithName ("Pattern");
            if (patternTree.isValid())
                pattern.fromTree (patternTree);
            else
                pattern.setDefault();
            tree.removeChild (patternTree, nullptr);
            apvts.replaceState (tree);
            ++programChangeCounter;
        }
    }
}

//==============================================================================
namespace
{
    juce::File& userPresetFolderOverride()
    {
        static juce::File folder;
        return folder;
    }
}

juce::File StesoLeadProcessor::getUserPresetFolder()
{
    if (userPresetFolderOverride() != juce::File())
        return userPresetFolderOverride();
    const auto docs = juce::File::getSpecialLocation (juce::File::userDocumentsDirectory);
    auto folder = docs.getChildFile ("Falk Instruments").getChildFile ("Bakelit").getChildFile ("Presets");

    // Flytta med egna förinställningar från tiden då pluginet hette StesoLead (görs en gång).
    const auto legacy = docs.getChildFile ("StesoLead").getChildFile ("Presets");
    if (! folder.isDirectory() && legacy.isDirectory())
        legacy.copyDirectoryTo (folder);
    return folder;
}

void StesoLeadProcessor::setUserPresetFolder (const juce::File& folder)
{
    userPresetFolderOverride() = folder;
}

juce::Array<juce::File> StesoLeadProcessor::getUserPresetFiles()
{
    auto files = getUserPresetFolder().findChildFiles (juce::File::findFiles, false,
                                                       "*" + userPresetExtension + ";*" + legacyPresetExtension);
    std::sort (files.begin(), files.end(), [] (const juce::File& a, const juce::File& b)
    {
        return a.getFileNameWithoutExtension().compareNatural (b.getFileNameWithoutExtension()) < 0;
    });
    return files;
}

juce::File StesoLeadProcessor::saveUserPreset (const juce::String& rawName)
{
    const auto name = juce::File::createLegalFileName (rawName.trim());
    if (name.isEmpty())
        return {};

    auto folder = getUserPresetFolder();
    if (! folder.createDirectory())
        return {};

    // Samma format som projektets sparade läge, utan exportinställningar.
    juce::ValueTree preset ("BakelitPreset");
    preset.setProperty ("name", rawName.trim(), nullptr);
    preset.setProperty ("version", STESO_VERSION, nullptr);
    for (auto* p : getParameters())
    {
        if (auto* rp = dynamic_cast<juce::RangedAudioParameter*> (p))
        {
            juce::ValueTree param ("PARAM");
            param.setProperty ("id", rp->getParameterID(), nullptr);
            param.setProperty ("value", rp->convertFrom0to1 (rp->getValue()), nullptr);
            preset.appendChild (param, nullptr);
        }
    }
    preset.appendChild (pattern.toTree(), nullptr);

    auto file = folder.getChildFile (name + userPresetExtension);
    folder.getChildFile (name + legacyPresetExtension).deleteFile();
    auto xml = preset.createXml();
    if (xml == nullptr || ! xml->writeTo (file))
        return {};

    userPresetName = rawName.trim();
    ++programChangeCounter;
    return file;
}

bool StesoLeadProcessor::loadUserPreset (const juce::File& file)
{
    auto xml = juce::XmlDocument::parse (file);
    if (xml == nullptr || ! (xml->hasTagName ("BakelitPreset") || xml->hasTagName ("StesoLeadPreset")))
        return false;

    const auto preset = juce::ValueTree::fromXml (*xml);
    for (auto* p : getParameters())
    {
        auto* rp = dynamic_cast<juce::RangedAudioParameter*> (p);
        if (rp == nullptr)
            continue;
        // Parametrar som saknas (äldre preset) får sitt standardvärde.
        const auto param = preset.getChildWithProperty ("id", rp->getParameterID());
        const float value = param.isValid() ? rp->convertTo0to1 ((float) param.getProperty ("value"))
                                            : rp->getDefaultValue();
        rp->setValueNotifyingHost (value);
    }

    const auto patternTree = preset.getChildWithName ("Pattern");
    if (patternTree.isValid())
        pattern.fromTree (patternTree);
    else
        pattern.setDefault();

    userPresetName = preset.getProperty ("name", file.getFileNameWithoutExtension()).toString();
    bypassedByExport = false;
    ++programChangeCounter;
    return true;
}

juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new StesoLeadProcessor();
}
