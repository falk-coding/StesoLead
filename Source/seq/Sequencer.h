#pragma once

#include <juce_audio_basics/juce_audio_basics.h>
#include <juce_data_structures/juce_data_structures.h>
#include <array>
#include <atomic>
#include <vector>

// Sekvenser/arpeggiator. All tidsberäkning utgår från låtens position i PPQ (kvartsnoter från takt 1),
// så att samma mönster alltid hamnar på samma ställe – och så att en MIDI-export blir identisk med
// det som spelas. Samma kod används för realtid och export.
namespace steso::seq
{
inline constexpr int maxSteps = 32;
inline constexpr int maxHeld = 16;

enum class Mode { off = 0, arp, seq };
enum class ArpOrder { up = 0, down, upDown, random, asPlayed };

inline const juce::StringArray modeNames { "Off", "Arp", "Seq" };
inline const juce::StringArray rateNames { "1/4", "1/8", "1/8T", "1/8D", "1/16", "1/16T", "1/16D", "1/32" };
inline const juce::StringArray arpOrderNames { "Up", "Down", "Up/Down", "Random", "As Played" };

double rateToPpq (int rateIndex) noexcept;

// Stegdata. Atomics: GUI skriver, ljudtråden läser.
struct Pattern
{
    std::array<std::atomic<bool>, maxSteps> on, accent, tie;
    std::array<std::atomic<int>, maxSteps> pitch;      // -24..24 halvtoner
    std::array<std::atomic<float>, maxSteps> prob;     // 0..1
    std::array<std::atomic<float>, maxSteps> gate;     // 0.05..1 av steget

    Pattern() { setDefault(); }

    void setDefault();
    void copyFrom (const Pattern& other);

    // Kompakt textformat för presets: ett token per steg, separerade med mellanslag.
    //   "."      = av
    //   "7"      = på, +7 halvtoner          "-5" = på, -5 halvtoner
    //   "!"      = accent                    "~"  = tie/slide till nästa steg
    //   "?60"    = 60 % sannolikhet          "_"  = lång gate (95 %)   "'" = kort gate (20 %)
    // Exempel: "0! 0 12 . 3~ 5 . 7?50"
    void fromString (const juce::String& text);

    // Euklidisk rytm: sprider `hits` jämnt över `steps` steg, roterat `rotate` steg.
    void applyEuclid (int hits, int steps, int rotate);

    juce::ValueTree toTree() const;
    void fromTree (const juce::ValueTree& tree);
};

// Parametrar som gäller just nu (läses en gång per block / per export).
struct Settings
{
    Mode mode = Mode::off;
    double stepPpq = 0.25;
    float swing = 0.0f;        // 0..1, fördröjer varannan ton upp till 1/3 steg
    float gateScale = 1.0f;
    ArpOrder arpOrder = ArpOrder::up;
    int arpOctaves = 1;
    int length = 16, accentLength = 16, gateLength = 16;
};

// Ackordet/grundtonen som sekvensern spelar från.
struct Chord
{
    int count = 0;
    std::array<int, maxHeld> notes {};          // i tryckordning
};

struct NoteEvent
{
    double ppqOn = 0, ppqOff = 0;
    int note = 60;
    float velocity = 0.8f;
    bool tied = false;
    int64_t step = 0;
};

// Alla toner vars start ligger i [fromPpq, toPpq).
void generate (const Pattern& pattern, const Settings& settings, const Chord& chord,
               double fromPpq, double toPpq, std::vector<NoteEvent>& out);

// Håller reda på klingande toner: slår ihop tie på samma ton och kortar överlapp, så att
// realtid och export ger exakt samma noter.
class NoteStream
{
public:
    struct Out { double ppq; int note; float velocity; bool isOn; };

    void add (const NoteEvent& e, std::vector<Out>& out);
    // Släpp alla toner som slutar före `ppq`.
    void flushUntil (double ppq, std::vector<Out>& out);
    // Släpp allt nu (t.ex. när transporten stoppas). Tidpunkt = `ppq`.
    void flushAll (double ppq, std::vector<Out>& out);
    bool isEmpty() const noexcept { return count == 0; }

private:
    struct Pending { double off; int note; bool tied; };
    std::array<Pending, 64> pending {};
    int count = 0;
};

// Bygger en MIDI-fil (960 tick/kvartsnot) med mönstret från fromPpq till toPpq. Tid 0 i filen = fromPpq.
juce::MidiFile renderMidiFile (const Pattern& pattern, const Settings& settings, const Chord& chord,
                               double fromPpq, double toPpq, double bpm, int timeSigNum, int timeSigDen);
} // namespace steso::seq
