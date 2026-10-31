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
    std::cout << "  " << a->getParameters().size() << " parametrar kontrollerade" << std::endl;
}

void screenshot (const juce::File& file)
{
    std::unique_ptr<StesoLeadProcessor> proc (dynamic_cast<StesoLeadProcessor*> (createPluginFilter()));
    proc->setCurrentProgram (1);
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
    if (shot != juce::File())
        screenshot (shot);

    std::cout << (failures == 0 ? "ALLA TESTER OK" : juce::String (failures) + " FEL") << std::endl;
    return failures == 0 ? 0 : 1;
}
