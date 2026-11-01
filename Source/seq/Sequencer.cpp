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
    resetSteps();
    resetLanes();
}

void Pattern::resetLanes()
{
    for (auto& lane : lanePitch)
        for (auto& v : lane)
            v = 0;
}

void Pattern::lanesFromString (const juce::String& text)
{
    resetLanes();
    auto lanes = juce::StringArray::fromTokens (text, "|", "");
    for (int l = 0; l < std::min (lanes.size(), polyLanes); ++l)
    {
        auto tokens = juce::StringArray::fromTokens (lanes[l], " ", "");
        tokens.removeEmptyStrings();
        for (int i = 0; i < std::min (tokens.size(), polyMaxSteps); ++i)
            lanePitch[(size_t) l][(size_t) i] = juce::jlimit (-24, 24, tokens[i].getIntValue());
    }
}

void Pattern::resetSteps()
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
    for (size_t l = 0; l < (size_t) polyLanes; ++l)
        for (size_t i = 0; i < (size_t) polyMaxSteps; ++i)
            lanePitch[l][i] = o.lanePitch[l][i].load();
}

void Pattern::fromString (const juce::String& text)
{
    resetSteps();
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

    juce::String sLanes;
    for (size_t l = 0; l < (size_t) polyLanes; ++l)
    {
        if (l > 0) sLanes << "| ";
        for (size_t i = 0; i < (size_t) polyMaxSteps; ++i)
            sLanes << lanePitch[l][i].load() << ' ';
    }
    t.setProperty ("lanes", sLanes.trim(), nullptr);
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

    // Sparat före v1.7 saknar spår -> nollor.
    lanesFromString (t.getProperty ("lanes").toString());
}

//==============================================================================
namespace
{
    int64_t floorDiv (int64_t a, int64_t b) noexcept
    {
        const auto q = a / b;
        return (a % b != 0 && ((a < 0) != (b < 0))) ? q - 1 : q;
    }

    const Settings::Lane& laneOf (const Settings& s, int lane) noexcept
    {
        return s.lanes[(size_t) std::clamp (lane, 0, polyLanes - 1)];
    }

    int lanePulses (const Settings::Lane& l) noexcept { return std::clamp (l.pulses, 1, polyMaxPulses); }
} // namespace

// Antal spelade slag före slag n (räknas från låtens början) -> vilket tonhöjdssteg som gäller.
int64_t polyHitIndex (const Settings& s, int lane, int64_t n) noexcept
{
    const auto& l = laneOf (s, lane);
    if (l.type != 0)
        return n;
    const int p = lanePulses (l);
    const int h = std::clamp (l.hits, 1, p);
    const auto cycle = floorDiv (n, p);
    const int j = posMod (n, p);
    int before = 0;
    for (int k = 0; k < j; ++k)
        if ((k * h) % p < h)
            ++before;
    return cycle * h + before;
}

int64_t polyPulseAt (const Settings& s, int lane, double ppq) noexcept
{
    const double interval = polyIntervalPpq (s, lane);
    auto n = (int64_t) std::floor (ppq / interval) + 1;
    while (polyPulsePpq (s, lane, n) > ppq + 1.0e-9)
        --n;
    return n;
}

double polyIntervalPpq (const Settings& s, int lane) noexcept
{
    const auto& l = laneOf (s, lane);
    const int p = lanePulses (l);
    return l.type == 0 ? s.polyCyclePpq() / p : std::max (1.0 / 64.0, s.stepPpq) * p;
}

double polyPulsePpq (const Settings& s, int lane, int64_t n) noexcept
{
    const auto& l = laneOf (s, lane);
    if (l.type != 0)
        return (double) n * polyIntervalPpq (s, lane);

    // Räkna per cykel så att slagen ligger exakt på cykelns början (inga avrundningsfel över tid).
    const int p = lanePulses (l);
    const auto cycle = floorDiv (n, p);
    return (double) cycle * s.polyCyclePpq() + (double) posMod (n, p) * s.polyCyclePpq() / p;
}

bool polyPulseIsHit (const Settings& s, int lane, int64_t n) noexcept
{
    const auto& l = laneOf (s, lane);
    if (l.type != 0)
        return true;
    const int p = lanePulses (l);
    const int h = std::clamp (l.hits, 1, p);
    return (posMod (n, p) * h) % p < h;
}

namespace
{
    void generatePoly (const Pattern& pattern, const Settings& s, const Chord& chord,
                       double fromPpq, double toPpq, std::vector<NoteEvent>& out)
    {
        struct Hit { NoteEvent e; int lane; };
        std::array<Hit, 256> hits;
        int count = 0;
        const int root = chord.notes[(size_t) (chord.count - 1)];   // senast nedtryckta = grundton

        for (int lane = 0; lane < polyLanes; ++lane)
        {
            const auto& l = s.lanes[(size_t) lane];
            if (! l.on)
                continue;

            const double interval = polyIntervalPpq (s, lane);
            const auto first = (int64_t) std::floor (fromPpq / interval) - 1;
            const auto last  = (int64_t) std::floor (toPpq / interval) + 1;

            for (int64_t n = first; n <= last && count < (int) hits.size(); ++n)
            {
                const double on = polyPulsePpq (s, lane, n);
                if (on < fromPpq || on >= toPpq || ! polyPulseIsHit (s, lane, n))
                    continue;

                const int steps = std::clamp (l.steps, 1, polyMaxSteps);
                const auto stepIdx = (size_t) posMod (polyHitIndex (s, lane, n), steps);

                NoteEvent e;
                e.step = n;
                e.ppqOn = on;
                e.ppqOff = on + std::max (0.02, 0.5 * (double) s.gateScale * interval);
                e.note = std::clamp (root + l.pitch + pattern.lanePitch[(size_t) lane][stepIdx].load(), 0, 127);
                e.velocity = 0.78f;
                hits[(size_t) count++] = { e, lane };
            }
        }

        std::sort (hits.begin(), hits.begin() + count, [] (const Hit& a, const Hit& b)
        {
            return a.e.ppqOn != b.e.ppqOn ? a.e.ppqOn < b.e.ppqOn : a.lane < b.lane;
        });

        // Slag från flera spår på samma tid = accent ("där rytmerna möts"). Samma ton på samma tid spelas en gång.
        constexpr double eps = 1.0e-6;
        for (int i = 0; i < count;)
        {
            int j = i + 1;
            while (j < count && hits[(size_t) j].e.ppqOn - hits[(size_t) i].e.ppqOn < eps)
                ++j;

            const bool accent = j - i > 1;
            for (int k = i; k < j; ++k)
            {
                auto e = hits[(size_t) k].e;
                e.ppqOn = hits[(size_t) i].e.ppqOn;
                bool duplicate = false;
                for (int m = i; m < k; ++m)
                    duplicate = duplicate || hits[(size_t) m].e.note == e.note;
                if (duplicate)
                    continue;
                for (int m = k + 1; m < j; ++m)
                    if (hits[(size_t) m].e.note == e.note)
                        e.ppqOff = std::max (e.ppqOff, hits[(size_t) m].e.ppqOff);
                if (accent)
                    e.velocity = 1.0f;
                out.push_back (e);
            }
            i = j;
        }
    }
}

//==============================================================================
void generate (const Pattern& pattern, const Settings& s, const Chord& chord,
               double fromPpq, double toPpq, std::vector<NoteEvent>& out)
{
    if (s.mode == Mode::off || chord.count <= 0 || s.stepPpq <= 0.0 || toPpq <= fromPpq)
        return;

    if (s.mode == Mode::poly)
    {
        generatePoly (pattern, s, chord, fromPpq, toPpq, out);
        return;
    }

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
