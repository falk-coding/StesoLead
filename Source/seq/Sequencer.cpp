#include "Sequencer.h"
#include <algorithm>
#include <cmath>

namespace steso::seq
{
namespace
{
    uint64_t splitmix (uint64_t x) noexcept
    {
        x += 0x9E3779B97F4A7C15ull;
        x = (x ^ (x >> 30)) * 0xBF58476D1CE4E5B9ull;
        x = (x ^ (x >> 27)) * 0x94D049BB133111EBull;
        return x ^ (x >> 31);
    }

    // Deterministiskt "slumptal" 0..1 per steg – samma värde i realtid och export.
    double hash01 (int64_t n, uint64_t salt) noexcept
    {
        return (double) (splitmix ((uint64_t) n ^ salt) >> 11) * (1.0 / 9007199254740992.0);
    }

    int posMod (int64_t n, int m) noexcept
    {
        const auto r = (int) (n % m);
        return r < 0 ? r + m : r;
    }

    double swingOffset (int64_t n, const Settings& s) noexcept
    {
        return (n % 2 != 0) ? (double) s.swing * s.stepPpq / 3.0 : 0.0;
    }

    double stepStart (int64_t n, const Settings& s) noexcept
    {
        return (double) n * s.stepPpq + swingOffset (n, s);
    }

    int pickNote (int64_t n, const Settings& s, const Chord& chord) noexcept
    {
        if (s.mode == Mode::seq)
            return chord.notes[(size_t) (chord.count - 1)];   // senast nedtryckta = grundton

        // Arp: bygg ton-listan (sorterad eller i tryckordning) över valt antal oktaver.
        std::array<int, maxHeld> base {};
        const int count = std::min (chord.count, maxHeld);
        std::copy (chord.notes.begin(), chord.notes.begin() + count, base.begin());
        if (s.arpOrder != ArpOrder::asPlayed)
            std::sort (base.begin(), base.begin() + count);

        const int octaves = std::clamp (s.arpOctaves, 1, 4);
        const int total = count * octaves;
        auto at = [&] (int idx) { return base[(size_t) (idx % count)] + 12 * (idx / count); };

        switch (s.arpOrder)
        {
            case ArpOrder::down:
                return at (total - 1 - posMod (n, total));
            case ArpOrder::upDown:
            {
                if (total == 1) return at (0);
                const int period = 2 * total - 2;
                const int m = posMod (n, period);
                return at (m < total ? m : period - m);
            }
            case ArpOrder::random:
                return at ((int) (hash01 (n, 0xA5A5A5A5ull) * total) % total);
            case ArpOrder::up:
            case ArpOrder::asPlayed:
            default:
                return at (posMod (n, total));
        }
    }
} // namespace

double rateToPpq (int rateIndex) noexcept
{
    static constexpr double values[] { 1.0, 0.5, 1.0 / 3.0, 0.75, 0.25, 1.0 / 6.0, 0.375, 0.125 };
    return values[std::clamp (rateIndex, 0, (int) std::size (values) - 1)];
}

//==============================================================================
void Pattern::setDefault()
{
    for (int i = 0; i < maxSteps; ++i)
    {
        on[(size_t) i] = true;
        accent[(size_t) i] = (i % 4) == 0;
        tie[(size_t) i] = false;
        pitch[(size_t) i] = 0;
        prob[(size_t) i] = 1.0f;
        gate[(size_t) i] = 0.5f;
    }
}

void Pattern::copyFrom (const Pattern& o)
{
    for (size_t i = 0; i < (size_t) maxSteps; ++i)
    {
        on[i] = o.on[i].load();
        accent[i] = o.accent[i].load();
        tie[i] = o.tie[i].load();
        pitch[i] = o.pitch[i].load();
        prob[i] = o.prob[i].load();
        gate[i] = o.gate[i].load();
    }
}

void Pattern::fromString (const juce::String& text)
{
    setDefault();
    for (auto& a : accent) a = false;

    auto tokens = juce::StringArray::fromTokens (text, " ", "");
    tokens.removeEmptyStrings();

    for (int i = 0; i < maxSteps; ++i)
    {
        const auto idx = (size_t) i;
        if (tokens.isEmpty())
            break;

        const auto t = tokens[i % tokens.size()];
        if (t.startsWithChar ('.'))
        {
            on[idx] = false;
            continue;
        }

        on[idx] = true;
        accent[idx] = t.containsChar ('!');
        tie[idx] = t.containsChar ('~');
        gate[idx] = t.containsChar ('_') ? 0.95f : (t.containsChar ('\'') ? 0.2f : 0.5f);

        const auto q = t.indexOfChar ('?');
        prob[idx] = q >= 0 ? juce::jlimit (0.0f, 1.0f, t.substring (q + 1).getFloatValue() / 100.0f) : 1.0f;

        const auto num = t.initialSectionContainingOnly ("-0123456789");
        pitch[idx] = juce::jlimit (-24, 24, num.getIntValue());
    }
}

void Pattern::applyEuclid (int hits, int steps, int rotate)
{
    steps = std::clamp (steps, 1, maxSteps);
    hits = std::clamp (hits, 0, steps);
    for (int i = 0; i < steps; ++i)
    {
        const int r = posMod ((int64_t) i - rotate, steps);
        on[(size_t) i] = ((r * hits) % steps) < hits;
    }
}

juce::ValueTree Pattern::toTree() const
{
    juce::String sOn, sAcc, sTie, sPitch, sProb, sGate;
    for (size_t i = 0; i < (size_t) maxSteps; ++i)
    {
        sOn << (on[i].load() ? '1' : '0');
        sAcc << (accent[i].load() ? '1' : '0');
        sTie << (tie[i].load() ? '1' : '0');
        sPitch << pitch[i].load() << ' ';
        sProb << juce::roundToInt (prob[i].load() * 100.0f) << ' ';
        sGate << juce::roundToInt (gate[i].load() * 100.0f) << ' ';
    }

    juce::ValueTree t ("Pattern");
    t.setProperty ("on", sOn, nullptr);
    t.setProperty ("accent", sAcc, nullptr);
    t.setProperty ("tie", sTie, nullptr);
    t.setProperty ("pitch", sPitch.trim(), nullptr);
    t.setProperty ("prob", sProb.trim(), nullptr);
    t.setProperty ("gate", sGate.trim(), nullptr);
    return t;
}

void Pattern::fromTree (const juce::ValueTree& t)
{
    if (! t.isValid())
        return;

    const auto sOn = t.getProperty ("on").toString();
    const auto sAcc = t.getProperty ("accent").toString();
    const auto sTie = t.getProperty ("tie").toString();
    const auto pitches = juce::StringArray::fromTokens (t.getProperty ("pitch").toString(), " ", "");
    const auto probs = juce::StringArray::fromTokens (t.getProperty ("prob").toString(), " ", "");
    const auto gates = juce::StringArray::fromTokens (t.getProperty ("gate").toString(), " ", "");

    for (int i = 0; i < maxSteps; ++i)
    {
        const auto idx = (size_t) i;
        if (i < sOn.length())  on[idx] = sOn[i] == '1';
        if (i < sAcc.length()) accent[idx] = sAcc[i] == '1';
        if (i < sTie.length()) tie[idx] = sTie[i] == '1';
        if (i < pitches.size()) pitch[idx] = juce::jlimit (-24, 24, pitches[i].getIntValue());
        if (i < probs.size())   prob[idx] = juce::jlimit (0.0f, 1.0f, probs[i].getFloatValue() / 100.0f);
        if (i < gates.size())   gate[idx] = juce::jlimit (0.05f, 1.0f, gates[i].getFloatValue() / 100.0f);
    }
}

//==============================================================================
void generate (const Pattern& pattern, const Settings& s, const Chord& chord,
               double fromPpq, double toPpq, std::vector<NoteEvent>& out)
{
    if (s.mode == Mode::off || chord.count <= 0 || s.stepPpq <= 0.0 || toPpq <= fromPpq)
        return;

    const int len = std::clamp (s.length, 1, maxSteps);
    const int accLen = std::clamp (s.accentLength, 1, maxSteps);
    const int gateLen = std::clamp (s.gateLength, 1, maxSteps);

    const auto first = (int64_t) std::floor (fromPpq / s.stepPpq) - 1;
    const auto last  = (int64_t) std::floor (toPpq / s.stepPpq) + 1;

    for (int64_t n = first; n <= last; ++n)
    {
        const double on = stepStart (n, s);
        if (on < fromPpq || on >= toPpq)
            continue;

        const auto i = (size_t) posMod (n, len);
        if (! pattern.on[i].load())
            continue;

        const float p = pattern.prob[i].load();
        if (p < 1.0f && hash01 (n, 0x5EED5EEDull) >= (double) p)
            continue;

        NoteEvent e;
        e.step = n;
        e.ppqOn = on;
        e.note = std::clamp (pickNote (n, s, chord) + pattern.pitch[i].load(), 0, 127);
        e.velocity = pattern.accent[(size_t) posMod (n, accLen)].load() ? 1.0f : 0.78f;
        e.tied = pattern.tie[i].load();

        if (e.tied)
        {
            // Håll till nästa steg med lite överlapp -> legato/glide i synthen.
            e.ppqOff = stepStart (n + 1, s) + 0.02 * s.stepPpq;
        }
        else
        {
            const double g = (double) pattern.gate[(size_t) posMod (n, gateLen)].load() * (double) s.gateScale;
            e.ppqOff = on + std::max (0.02, g * s.stepPpq);
        }

        out.push_back (e);
    }
}

//==============================================================================
void NoteStream::add (const NoteEvent& e, std::vector<Out>& out)
{
    for (int j = 0; j < count; ++j)
    {
        auto& p = pending[(size_t) j];
        if (p.note != e.note)
            continue;

        if (p.tied && p.off >= e.ppqOn - 1.0e-9)
        {
            // Tie till samma ton: förläng i stället för att slå an igen.
            p.off = std::max (p.off, e.ppqOff);
            p.tied = e.tied;
            return;
        }

        if (p.off > e.ppqOn)
        {
            // Överlapp på samma ton: avsluta den gamla precis innan den nya.
            out.push_back ({ e.ppqOn, p.note, 0.0f, false });
            pending[(size_t) j] = pending[(size_t) (count - 1)];
            --count;
        }
        break;
    }

    out.push_back ({ e.ppqOn, e.note, e.velocity, true });

    if (count < (int) pending.size())
        pending[(size_t) count++] = { e.ppqOff, e.note, e.tied };
}

void NoteStream::flushUntil (double ppq, std::vector<Out>& out)
{
    // Släpp i tidsordning.
    for (;;)
    {
        int best = -1;
        for (int j = 0; j < count; ++j)
            if (pending[(size_t) j].off <= ppq && (best < 0 || pending[(size_t) j].off < pending[(size_t) best].off))
                best = j;

        if (best < 0)
            return;

        out.push_back ({ pending[(size_t) best].off, pending[(size_t) best].note, 0.0f, false });
        pending[(size_t) best] = pending[(size_t) (count - 1)];
        --count;
    }
}

void NoteStream::flushAll (double ppq, std::vector<Out>& out)
{
    for (int j = 0; j < count; ++j)
        out.push_back ({ ppq, pending[(size_t) j].note, 0.0f, false });
    count = 0;
}

//==============================================================================
juce::MidiFile renderMidiFile (const Pattern& pattern, const Settings& settings, const Chord& chord,
                               double fromPpq, double toPpq, double bpm, int timeSigNum, int timeSigDen)
{
    constexpr int ticksPerQuarter = 960;

    std::vector<NoteEvent> events;
    generate (pattern, settings, chord, fromPpq, toPpq, events);

    std::vector<NoteStream::Out> out;
    NoteStream stream;
    for (const auto& e : events)
    {
        stream.flushUntil (e.ppqOn, out);
        stream.add (e, out);
    }
    stream.flushUntil (toPpq, out);
    stream.flushAll (toPpq, out);   // toner som sträcker sig förbi slutet kortas av vid slutet

    juce::MidiMessageSequence track;
    auto meta = juce::MidiMessage::tempoMetaEvent (juce::roundToInt (60000000.0 / std::max (1.0, bpm)));
    meta.setTimeStamp (0);
    track.addEvent (meta);
    auto sig = juce::MidiMessage::timeSignatureMetaEvent (timeSigNum, timeSigDen);
    sig.setTimeStamp (0);
    track.addEvent (sig);

    for (const auto& o : out)
    {
        const double ticks = std::round ((o.ppq - fromPpq) * ticksPerQuarter);
        auto m = o.isOn ? juce::MidiMessage::noteOn (1, o.note, o.velocity)
                        : juce::MidiMessage::noteOff (1, o.note);
        m.setTimeStamp (ticks);
        track.addEvent (m);
    }

    auto end = juce::MidiMessage::endOfTrack();
    end.setTimeStamp (std::round ((toPpq - fromPpq) * ticksPerQuarter));
    track.addEvent (end);
    track.updateMatchedPairs();

    juce::MidiFile file;
    file.setTicksPerQuarterNote (ticksPerQuarter);
    file.addTrack (track);
    return file;
}
} // namespace steso::seq
