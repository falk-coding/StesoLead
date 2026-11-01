# Bakelit – manual

**Falk Instruments · version 1.8**

Bakelit is a polyphonic analog-style synthesizer with three oscillators, subharmonics, two filter types, a step sequencer, an arpeggiator and polyrhythms. This manual takes you from installation to your own sound in ten minutes.

The panel labels are in Swedish with English in parentheses, e.g. *FILTERKURVA (FILTER ENV)*. Knob names are in English.

---

## 1. Install

**Mac (VST3 + AU)**

1. Unzip `Bakelit-mac.zip`.
2. Right-click **Installera.command** → **Open** (first time only, as the app is not yet notarized).
3. Restart your DAW. Bakelit appears under *Falk Instruments*.

**Windows (VST3)**

1. Unzip `Bakelit-windows.zip`.
2. Double-click **Installera.bat** and accept the administrator prompt.
3. Restart your DAW.

**Reason:** find Bakelit in the browser under *Instruments → Plugins → Falk Instruments*. Also works in Ableton, Logic (AU), Cubase, FL Studio, Bitwig, Studio One and Reaper.

---

## 2. Up and running in a minute

1. Put Bakelit on an instrument track and play.
2. Browse sounds with **<** and **>** at the top right, or click the name to see all sounds by category.
3. Try the **Sequence**, **Polyrhythm** and **Subharmonic** categories – hold a key and press Play in your DAW.
4. Made something you like? **Spara (Save)**.

**Good to know**

- **Double-click** a knob = default value.
- Drag up/down on a knob to turn it. The value is shown while dragging.
- Hover over a control for a short explanation.
- Drag the bottom-right corner of the window to resize it.
- **Sekv. (Seq)** and **Klav. (Keys)** fold out the sequencer and keyboard. The lamp on Sekv. lights when the sequencer is playing while folded in.

---

## 3. Signal flow

```
OSC 1 ─┐
OSC 2 ─┼─► MIXER ─► FILTER ─► AMP ENV ─► EFFECTS ─► OUTPUT
OSC 3 ─┤            ▲             ▲
Sub-   ┘       FILTER ENV     velocity
harmonics           ▲
              LFO: pitch, filter, PW
```

---

## 4. The sections

### Oscillators (OSC 1–3)
**OSC 1 is the reference.** It plays exactly the note you press and sets the register with **Octave** (−2 to +2). **OSC 2 and 3 are tuned against OSC 1**: **Semi** sets intervals (+7 = fifth, ±12 = octave), **Fine** detunes slightly for a fat, beating sound.

| Knob | Does |
|---|---|
| Wave | Saw, Square, Triangle, Sine |
| PW | Pulse width for Square (50 % = square, narrower = thinner, nasal) |
| **Sub N Div** | Subharmonic divider: the oscillator's frequency ÷ 1–16 |
| **Sub N** | Subharmonic level (0 = off) |

**Subharmonics (UNDERTON / Sub 1–3)** are framed inside each oscillator. They follow their oscillator's pitch, waveform and PW exactly. ÷2 = one octave down, ÷3 = a fifth below that, ÷4 = two octaves down, ÷5, ÷6 … give ever darker, stranger tones. Try OSC 1 with Sub 1 Div = 3 and Sub 1 = 50 %.

### Mixer (NIVÅER)
Levels for **OSC 1–3**, **Sub Oct** (a classic square one octave below OSC 1) and **Noise**.

### Filter (VCF)
| Knob | Does |
|---|---|
| Type | **Ladder** (round, fat, 24 dB) · **Steppe LP** (aggressive, raw) · **Steppe BP** (band-pass, nasal) |
| Cutoff | Cutoff frequency – dark/bright |
| Reso | Resonance – squeal at the cutoff; self-oscillates at the top |
| Drive | Overdrive into the filter |
| Env | How much the filter envelope opens the filter |
| Key | Filter follows the keyboard (brighter higher up) |
| Velocity | Harder playing = brighter |
| Feedback | Filter output fed back in – dirtier, thicker |
| Warmth | Soft saturation after the filter |

### Filter env and Amp env (FILTERKURVA / VOLYMKURVA, ADSR)
**Attack**, **Decay**, **Sustain** (level while the key is held), **Release** (tail after letting go). The amp envelope's **Velocity** sets how much playing strength affects volume.

### Modulation (LFO)
A slow oscillator that moves the sound: **Wave**, **Rate**, **Fade In** (vibrato fades in), and amounts to **Pitch** (vibrato), **Filter** (wah) and **PW** (pulse-width modulation, all three oscillators). The mod wheel adds extra vibrato.

### Voice (STÄMMA)
| Knob | Does |
|---|---|
| Mode | **Poly** (chords) · **Mono** · **Legato** (glide only on tied notes) |
| Glide | Portamento between notes |
| Bend | Pitch bend range |
| Drift | Analog instability in tuning and filter |
| Spread | Spreads voices across the stereo field |
| Unison | 1–4 layers per voice |
| Detune | How far the unison layers are detuned |

### Effects and Output (EFFEKTER / UTGÅNG)
**Chorus** (Off/I/II), **Delay** (Time, Feedback, amount), **Reverb** (amount). **Volume** is the master level. *Stämmor* (voices) at the top right shows how many voices are playing.

---

## 5. Step sequencer and arpeggiator

Fold out with **Sekv. (Seq)**. Follows your DAW's tempo; without transport it runs on its own clock while you hold a key.

**Mode**

- **Arp** – plays the chord you hold. **Arp** sets the order (Up, Down, Up/Down, Random, As Played), **Octaves** 1–4.
- **Seq** – plays the pattern, transposed by the key you hold.

**The grid** (click/drag): **Step** on/off · **Accent** · **Tie** (tie to next step = glide) · **Pitch** (drag up/down, ±24) · **Prob** (probability) · **Gate** (note length). Double-click resets Pitch/Prob/Gate.

| Control | Does |
|---|---|
| Rate | Step length (1/4 … 1/32, triplets, dotted) |
| Swing | Delays every other step |
| Gate | Length of all notes |
| Steps / Acc steps / Gate steps | Separate lengths (1–32) → polymeter, e.g. 7 steps against 4/4 |
| Latch | Keeps playing after you release the keys |
| MIDI out | Sends the notes to your DAW (not Reason/Logic – drag MIDI instead) |
| Euclid | Spreads *Hits* evenly over the steps. **Init** resets the pattern |

**Drag MIDI to DAW** (DRA MIDI TILL DAW) – drag the box to a MIDI track to get exactly what is playing as a MIDI clip. The length comes from your DAW's loop locators or from **Från/Till** (from/to bar). The sequencer then switches off to avoid doubled notes; **Slå på (Re-enable)** turns it back on.

---

## 6. Polyrhythm (POLYRYTM)

Turn on **Polyrhythm** in the sequencer row. It takes over from Arp/Seq and plays from the key you hold. The rings show where the hits fall – where lanes meet you get an **accent**.

Four lanes, each with:

| Column | Does |
|---|---|
| Lane (1–4) | Lane on/off |
| Type | **Bar** = *Pulses* hits spread evenly over the cycle · **Div** = one hit every *Pulses* steps of Rate (clock divider) |
| Pulses | Hits per cycle (or the divider in Div) |
| Hits | How many of the pulses play, evenly spread (Euclidean) |
| Pitch | Lane pitch relative to the key, ±24 semitones |
| Steps + Notes | 1–8 pitches the lane steps through, one per hit. Drag the cells |

**Cycle bars** (1–4) = cycle length in bars for Bar lanes.

**Quick setups:** **4:3**, **3:2**, **5:4**, **7:4**, **Div** (÷2 ÷3 ÷5 ÷7), **Init**.

**Get started:** click **4:3**, hold a note and press Play. Then change *Pulses* on lane 2 to 5, or switch on lane 3.

---

## 7. Your own sounds

- **Spara (Save)** stores all knobs and patterns. Same name = overwrite (asks first).
- **Ta bort (Delete)** removes the selected user sound.
- User sounds are at the top of the list under **Egna (User)**.
- The files (`.bakelit`) live in **Documents/Falk Instruments/Bakelit/Presets** – copy the folder to back up or to move between Mac and Windows.
- Your DAW project remembers everything, including patterns and folded-out panels.

---

## 8. Troubleshooting

| Problem | Fix |
|---|---|
| Not showing in the DAW | Restart the DAW. Run the installer again. Let the DAW rescan plugins |
| Mac: "cannot be opened" | Right-click → Open, or System Settings → Privacy & Security → Open Anyway |
| Sequencer silent | Mode or Polyrhythm must be on, and a key held (or Latch) |
| Doubled notes after dragging MIDI | The sequencer should be off – it happens automatically; check that *Slå på* wasn't pressed |
| Crackling | Raise the DAW's buffer size, or lower Unison |

---

*Bakelit © Falk Instruments. VST is a trademark of Steinberg Media Technologies GmbH.*
