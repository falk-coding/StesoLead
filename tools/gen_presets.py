#!/usr/bin/env python3
"""Genererar Source/PresetBank.inc – 100 extra fabrikspresets.

Kör:  python3 tools/gen_presets.py
Varje preset = mall (grundkaraktär) + egna avvikelser. Värden i riktiga enheter
(Hz, sekunder, 0..1, halvtoner, index för valparametrar). Lägg ALLTID nya presets
sist – programnumren används i sparade projekt.
"""
from pathlib import Path

# Index för valparametrar
SAW, SQ, TRI, SIN = 0, 1, 2, 3
POLY, MONO, LEG = 0, 1, 2
LADDER, PLP, PBP = 0, 1, 2
L_SINE, L_TRI, L_SAW, L_SQ, L_SH = 0, 1, 2, 3, 4
ARP, SEQ = 1, 2
R4, R8, R8T, R8D, R16, R16T, R16D, R32 = range(8)
UP, DOWN, UPDOWN, RANDOM, ASPLAYED = range(5)

# Mallar
MONO_BASS = dict(voiceMode=MONO, aSustain=1.0, aRelease=0.1, spread=0.0, keyTrack=0.4, drift=0.35)
LEAD = dict(voiceMode=LEG, aSustain=0.95, aRelease=0.3, spread=0.0, glide=0.05, delayMix=0.2, reverbMix=0.12)
KEYS = dict(aAttack=0.002, aDecay=1.8, aSustain=0.0, aRelease=0.5, ampVel=0.6, filterVel=0.5)
PAD = dict(aAttack=0.8, aSustain=0.9, aRelease=2.0, fAttack=0.8, fDecay=2.0, fSustain=0.5, ampVel=0.2,
           spread=0.7, reverbMix=0.35, filterEnv=0.2)
BRASS = dict(filterEnv=0.6, fAttack=0.05, fDecay=0.5, fSustain=0.45, fRelease=0.3, aAttack=0.02,
             aSustain=0.85, aRelease=0.3, chorus=1, spread=0.5, osc2Fine=-8)
PLUCK = dict(aAttack=0.001, aDecay=0.6, aSustain=0.0, aRelease=0.35, fDecay=0.25, fSustain=0.0,
             filterEnv=0.65, filterVel=0.5, reverbMix=0.12)
FX = dict(spread=0.5)

P = []  # (namn, kategori, mall, avvikelser, mönster)


def add(name, cat, base, pattern="", **kw):
    P.append((name, cat, base, kw, pattern))


# ---------------------------------------------------------------- Bas (22)
add("Deep Saw Bass", "Bass", MONO_BASS, cutoff=350, resonance=0.25, filterEnv=0.45, fDecay=0.3, fSustain=0.2,
    mixOsc3=0.6, drive=0.4, warmth=0.3)
add("Square Thump", "Bass", MONO_BASS, osc1Wave=SQ, osc2Wave=SQ, osc2Semi=-12, osc2Fine=0, cutoff=280,
    filterEnv=0.55, fDecay=0.18, fSustain=0.05, aDecay=0.4, aSustain=0.6)
add("Rubber Bass", "Bass", MONO_BASS, voiceMode=LEG, osc1Wave=TRI, osc2Wave=SQ, osc2Semi=-12, osc2Fine=0,
    mixOsc2=0.5, cutoff=500, resonance=0.45, filterEnv=0.5, fDecay=0.22, glide=0.04)
add("Pluck Ladder Bass", "Bass", MONO_BASS, aDecay=0.5, aSustain=0.3, cutoff=300, resonance=0.35, filterEnv=0.7,
    fDecay=0.2, fSustain=0.0, feedback=0.3, warmth=0.3)
add("Reese Ladder", "Bass", MONO_BASS, osc2Fine=25, unison=2, uniDetune=0.4, cutoff=600, resonance=0.2,
    filterEnv=0.15, warmth=0.4, drive=0.4, lfoFilter=0.1, lfoRate=0.2)
add("Sub Sine Bass", "Bass", MONO_BASS, osc1Wave=SIN, mixOsc2=0.0, mixSub=0.6, cutoff=600, resonance=0.0,
    filterEnv=0.0, warmth=0.3, master=-4.0)
add("Growl Wobble", "Bass", MONO_BASS, filterType=PLP, cutoff=220, resonance=0.7, filterEnv=0.0, lfoWave=L_TRI,
    lfoRate=2.0, lfoFilter=0.6, drive=0.7, feedback=0.3)
add("Acid Line", "Bass", MONO_BASS, voiceMode=LEG, mixOsc2=0.0, cutoff=300, resonance=0.85, filterEnv=0.75,
    fDecay=0.15, fSustain=0.0, drive=0.6, glide=0.06, filterVel=0.6, delayMix=0.15)
add("Acid Square", "Bass", MONO_BASS, voiceMode=LEG, osc1Wave=SQ, mixOsc2=0.0, cutoff=350, resonance=0.8,
    filterEnv=0.7, fDecay=0.18, fSustain=0.05, drive=0.5, glide=0.05, filterVel=0.6)
add("Steppe Stab Bass", "Bass", MONO_BASS, filterType=PLP, cutoff=400, resonance=0.55, filterEnv=0.6,
    fDecay=0.2, aDecay=0.35, aSustain=0.4, drive=0.6)
add("Dirty Drive Bass", "Bass", MONO_BASS, drive=0.9, warmth=0.6, cutoff=700, resonance=0.2, filterEnv=0.3,
    feedback=0.5)
add("Fifth Bass", "Bass", MONO_BASS, osc2Semi=7, osc2Fine=0, mixOsc2=0.5, cutoff=450, filterEnv=0.4, fDecay=0.3)
add("Octave Saw Bass", "Bass", MONO_BASS, osc2Semi=-12, osc2Fine=3, mixOsc2=0.8, cutoff=380, filterEnv=0.5,
    fDecay=0.25, fSustain=0.15)
add("Soft Round Bass", "Bass", MONO_BASS, osc1Wave=TRI, osc2Wave=SIN, osc2Semi=-12, osc2Fine=0, cutoff=800,
    resonance=0.1, filterEnv=0.1, warmth=0.5)
add("Funk Pluck Bass", "Bass", MONO_BASS, osc1Wave=SQ, osc1PW=0.3, mixOsc2=0.4, cutoff=350, resonance=0.4,
    filterEnv=0.65, fDecay=0.12, fSustain=0.1, filterVel=0.7, ampVel=0.7)
add("Slide Bass", "Bass", MONO_BASS, voiceMode=LEG, glide=0.12, cutoff=420, resonance=0.4, filterEnv=0.4,
    mixOsc3=0.4)
add("Detuned Mono Bass", "Bass", MONO_BASS, unison=3, uniDetune=0.25, cutoff=500, resonance=0.25, filterEnv=0.4,
    warmth=0.3)
add("Feedback Bass", "Bass", MONO_BASS, feedback=0.7, resonance=0.45, cutoff=320, filterEnv=0.5, drive=0.5,
    fDecay=0.3)
add("Bandpass Bass", "Bass", MONO_BASS, filterType=PBP, cutoff=600, resonance=0.6, filterEnv=0.45, fDecay=0.25,
    drive=0.5)
add("Velocity Bass", "Bass", MONO_BASS, filterVel=0.9, ampVel=0.8, cutoff=250, filterEnv=0.6, fDecay=0.25)
add("Pulse Width Bass", "Bass", MONO_BASS, osc1Wave=SQ, osc2Wave=SQ, osc2Semi=-12, osc2Fine=0, lfoPW=0.4,
    lfoRate=0.3, cutoff=500, filterEnv=0.35)
add("Analog Low End", "Bass", MONO_BASS, osc2Wave=SQ, osc2Semi=-12, osc2Fine=0, mixSub=0.5, mixOsc3=0.4,
    osc3Wave=TRI, cutoff=380, warmth=0.5, drift=0.5, filterEnv=0.35)

# ---------------------------------------------------------------- Lead (22)
add("Soaring Lead", "Lead", LEAD, osc2Fine=9, mixOsc3=0.4, osc3Semi=12, osc3Fine=0, cutoff=2200, resonance=0.3,
    drive=0.4, lfoPitch=0.3, lfoFade=0.8, delayMix=0.3, reverbMix=0.2)
add("Square Whistle", "Lead", LEAD, osc1Wave=SQ, osc2Wave=SQ, osc2Semi=12, osc2Fine=0, mixOsc2=0.3,
    cutoff=4000, resonance=0.1, filterEnv=0.1, lfoPitch=0.2, lfoFade=0.6)
add("Steppe Scream", "Lead", LEAD, filterType=PLP, resonance=0.75, drive=0.7, cutoff=1500, filterEnv=0.4,
    delayMix=0.3)
add("Bandpass Lead", "Lead", LEAD, filterType=PBP, resonance=0.6, cutoff=1200, filterEnv=0.5, fDecay=0.4)
add("Fifth Lead", "Lead", LEAD, osc2Semi=7, osc2Fine=0, mixOsc2=0.7, cutoff=2000, resonance=0.3, filterEnv=0.3,
    glide=0.06)
add("Octave Lead", "Lead", LEAD, osc2Semi=12, osc2Fine=2, mixOsc2=0.6, mixOsc3=0.5, cutoff=2400)
add("Hollow Tri Lead", "Lead", LEAD, osc1Wave=TRI, osc2Wave=TRI, osc2Semi=12, osc2Fine=0, cutoff=5000,
    resonance=0.0, filterEnv=0.0, lfoPitch=0.25, lfoFade=0.7, reverbMix=0.25)
add("Wide Saw Lead", "Lead", LEAD, unison=4, uniDetune=0.5, chorus=2, cutoff=3000, warmth=0.3)
add("Gritty Mono Lead", "Lead", LEAD, voiceMode=MONO, drive=0.8, warmth=0.5, feedback=0.4, cutoff=1800,
    resonance=0.35)
add("Portamento Lead", "Lead", LEAD, glide=0.25, cutoff=2500, resonance=0.25, mixOsc3=0.3)
add("Vibrato Flute", "Lead", LEAD, osc1Wave=TRI, mixOsc2=0.0, mixNoise=0.15, cutoff=3000, resonance=0.1,
    lfoPitch=0.35, lfoFade=1.0, aAttack=0.05, reverbMix=0.3, filterEnv=0.1)
add("Brassy Lead", "Lead", LEAD, filterEnv=0.6, fAttack=0.04, fDecay=0.5, fSustain=0.45, cutoff=900,
    resonance=0.2, osc2Fine=-8)
add("Thin Pulse Lead", "Lead", LEAD, osc1Wave=SQ, osc1PW=0.15, osc2Wave=SQ, osc2PW=0.2, osc2Fine=6, cutoff=3500)
add("Resonant Sweep Lead", "Lead", LEAD, cutoff=600, resonance=0.7, filterEnv=0.7, fAttack=0.3, fDecay=1.0,
    fSustain=0.4)
add("Feedback Howl", "Lead", LEAD, filterType=PLP, feedback=0.9, resonance=0.6, drive=0.6, cutoff=1400)
add("Detuned Lead", "Lead", LEAD, osc2Fine=25, cutoff=2600, warmth=0.2)
add("S&H Lead", "Lead", LEAD, lfoWave=L_SH, lfoRate=6.0, lfoFilter=0.4, resonance=0.5, cutoff=1500)
add("Echo Lead", "Lead", LEAD, delayMix=0.45, delayFb=0.55, delayTime=0.375, cutoff=2400, reverbMix=0.2)
add("Dark Lead", "Lead", LEAD, cutoff=900, resonance=0.2, filterEnv=0.2, warmth=0.5, mixOsc3=0.5)
add("Bright Saw Lead", "Lead", LEAD, cutoff=6000, resonance=0.15, filterEnv=0.1, drive=0.3)
add("Talking Lead", "Lead", LEAD, filterType=PBP, lfoFilter=0.5, lfoRate=3.0, lfoWave=L_TRI, resonance=0.55,
    cutoff=900)
add("Iron Curtain Lead", "Lead", LEAD, filterType=PLP, resonance=0.65, drive=0.75, warmth=0.4, feedback=0.3,
    cutoff=1300, mixOsc3=0.5)

# ---------------------------------------------------------------- Keys (11)
add("Soft Keys", "Keys", KEYS, osc1Wave=TRI, osc2Wave=SIN, osc2Semi=12, osc2Fine=0, mixOsc2=0.4, cutoff=3000,
    filterEnv=0.2, fDecay=1.0, chorus=1)
add("Funky Clav", "Keys", KEYS, osc1Wave=SQ, osc1PW=0.25, mixOsc2=0.0, cutoff=1500, resonance=0.3, filterEnv=0.5,
    fDecay=0.2, fSustain=0.0, aDecay=0.5, aRelease=0.1, filterVel=0.7)
add("Square Organ", "Keys", KEYS, osc1Wave=SQ, osc2Wave=SQ, osc2Semi=12, osc2Fine=0, mixOsc2=0.5, mixOsc3=0.4,
    osc3Wave=SQ, osc3Semi=19, osc3Fine=0, cutoff=6000, filterEnv=0.0, aAttack=0.005, aDecay=0.1, aSustain=1.0,
    aRelease=0.05, chorus=2, ampVel=0.1)
add("Analog Piano", "Keys", KEYS, osc2Wave=TRI, osc2Semi=12, cutoff=1800, filterEnv=0.4, fDecay=1.5,
    fSustain=0.1, filterVel=0.6)
add("Toy Keys", "Keys", KEYS, osc1Wave=SQ, osc2Wave=SQ, osc2Semi=24, osc2Fine=0, mixOsc2=0.3, aDecay=0.8,
    cutoff=5000, reverbMix=0.2)
add("Muted Keys", "Keys", KEYS, cutoff=600, filterEnv=0.3, fDecay=0.3, aDecay=1.0)
add("Chord Stab", "Keys", KEYS, aDecay=0.25, aRelease=0.15, cutoff=1200, filterEnv=0.5, fDecay=0.2, chorus=1,
    reverbMix=0.2)
add("Glass Keys", "Keys", KEYS, osc1Wave=SIN, osc2Wave=SIN, osc2Semi=24, osc2Fine=0, mixOsc2=0.5, mixOsc3=0.3,
    osc3Wave=TRI, osc3Semi=19, osc3Fine=0, aDecay=2.0, reverbMix=0.35, cutoff=8000, filterEnv=0.0)
add("Warm Tine Keys", "Keys", KEYS, osc1Wave=SIN, osc2Wave=TRI, osc2Semi=12, osc2Fine=0, mixOsc2=0.3,
    warmth=0.4, aDecay=2.5, aRelease=0.6, chorus=1, cutoff=2500)
add("Harpsi Pulse", "Keys", KEYS, osc1Wave=SQ, osc1PW=0.15, osc2Wave=SQ, osc2Semi=12, osc2PW=0.3, osc2Fine=0,
    aDecay=0.9, cutoff=4000, filterEnv=0.2)
add("Bell Keys", "Keys", KEYS, osc1Wave=TRI, osc2Wave=SIN, osc2Semi=19, osc2Fine=0, mixOsc2=0.6, aDecay=2.5,
    reverbMix=0.3, cutoff=7000, filterEnv=0.0)

# ---------------------------------------------------------------- Pad (13)
add("Analog Strings", "Pad", PAD, osc2Fine=10, chorus=2, cutoff=2800)
add("Dark Pad", "Pad", PAD, cutoff=600, resonance=0.2, warmth=0.3)
add("Sweep Pad", "Pad", PAD, cutoff=400, resonance=0.5, filterEnv=0.6, fAttack=3.0, fDecay=4.0, reverbMix=0.5)
add("Breathing Pad", "Pad", PAD, lfoFilter=0.3, lfoRate=0.15, cutoff=1000)
add("PWM Choir", "Pad", PAD, osc1Wave=SQ, osc2Wave=SQ, osc2Fine=5, lfoPW=0.5, lfoRate=0.4, chorus=1, cutoff=1800)
add("Steppe Drone", "Pad", PAD, filterType=PLP, resonance=0.6, cutoff=500, drive=0.5, aAttack=2.0,
    lfoFilter=0.2, lfoRate=0.1)
add("Glass Pad", "Pad", PAD, osc1Wave=TRI, osc2Wave=SIN, osc2Semi=12, osc2Fine=0, reverbMix=0.5, cutoff=4000)
add("Unison Pad", "Pad", PAD, unison=4, uniDetune=0.45, chorus=2, cutoff=2000)
add("Wind Pad", "Pad", PAD, mixNoise=0.6, mixOsc1=0.3, mixOsc2=0.0, filterType=PBP, resonance=0.6, cutoff=800,
    lfoFilter=0.4, lfoRate=0.1)
add("Warm Blanket", "Pad", PAD, cutoff=900, warmth=0.6, filterEnv=0.1, mixOsc3=0.3)
add("Fifth Pad", "Pad", PAD, osc2Semi=7, osc2Fine=0, cutoff=1600, chorus=1)
add("Shimmer Pad", "Pad", PAD, osc2Semi=12, osc2Fine=3, mixOsc3=0.4, osc3Semi=24, osc3Fine=0, delayMix=0.3,
    reverbMix=0.5, cutoff=3500)
add("Space Pad", "Pad", PAD, aRelease=3.0, reverbMix=0.6, delayMix=0.3, lfoPitch=0.1, lfoRate=0.2, cutoff=1400)

# ---------------------------------------------------------------- Brass (7)
add("Poly Brass", "Brass", BRASS, cutoff=700)
add("Bright Brass", "Brass", BRASS, cutoff=1400, filterEnv=0.5)
add("Unison Brass", "Brass", BRASS, unison=3, uniDetune=0.3, cutoff=800)
add("Fanfare", "Brass", BRASS, osc2Semi=7, osc2Fine=0, mixOsc2=0.5, cutoff=900)
add("Soft Horns", "Brass", BRASS, fAttack=0.15, aAttack=0.1, cutoff=500, filterEnv=0.4, reverbMix=0.25)
add("Steppe Brass", "Brass", BRASS, filterType=PLP, resonance=0.4, cutoff=600, drive=0.4)
add("Sweeping Brass", "Brass", BRASS, fAttack=0.4, fDecay=1.2, filterEnv=0.7, cutoff=400)

# ---------------------------------------------------------------- Pluck (9)
add("Basic Pluck", "Pluck", PLUCK, cutoff=500, resonance=0.3)
add("Square Pluck", "Pluck", PLUCK, osc1Wave=SQ, osc2Wave=SQ, osc2Semi=12, osc2Fine=0, mixOsc2=0.4, cutoff=600)
add("Resonant Pluck", "Pluck", PLUCK, resonance=0.7, cutoff=400)
add("Echo Pluck", "Pluck", PLUCK, delayMix=0.4, delayFb=0.5, cutoff=700)
add("Wood Pluck", "Pluck", PLUCK, osc1Wave=TRI, osc2Wave=SIN, osc2Semi=12, osc2Fine=0, mixOsc2=0.3, aDecay=0.4,
    cutoff=1500, filterEnv=0.4)
add("Harp Pluck", "Pluck", PLUCK, osc1Wave=TRI, osc2Semi=12, mixOsc2=0.3, aDecay=1.2, aRelease=1.0,
    reverbMix=0.35, cutoff=1200)
add("Steppe Pluck", "Pluck", PLUCK, filterType=PBP, resonance=0.6, cutoff=700)
add("Thumb Piano", "Pluck", PLUCK, osc1Wave=SIN, osc2Wave=TRI, osc2Semi=24, osc2Fine=0, mixOsc2=0.3, aDecay=0.7,
    cutoff=3000, filterEnv=0.2, reverbMix=0.2)
add("Chord Pluck", "Pluck", PLUCK, chorus=2, reverbMix=0.3, unison=2, cutoff=900)

# ---------------------------------------------------------------- FX (7)
add("Laser Zap", "FX", FX, osc1Wave=SQ, cutoff=6000, resonance=0.7, filterEnv=-0.8, fDecay=0.3, fSustain=0.0,
    aDecay=0.4, aSustain=0.0)
add("Siren", "FX", FX, voiceMode=MONO, lfoPitch=7.0, lfoRate=0.5, lfoWave=L_TRI, cutoff=3000, aSustain=1.0)
add("Robot Talk", "FX", FX, filterType=PBP, lfoWave=L_SH, lfoRate=9.0, lfoFilter=0.6, resonance=0.7,
    cutoff=1000, aSustain=1.0)
add("Noise Riser", "FX", FX, mixNoise=0.9, mixOsc1=0.0, mixOsc2=0.0, cutoff=200, resonance=0.7, filterEnv=0.9,
    fAttack=4.0, fDecay=1.0, fSustain=1.0, aAttack=3.0, aSustain=1.0, reverbMix=0.4)
add("Alarm", "FX", FX, osc1Wave=SQ, osc2Wave=SQ, osc2Semi=1, osc2Fine=0, mixOsc2=0.8, lfoPitch=2.0,
    lfoWave=L_SQ, lfoRate=4.0, aSustain=1.0)
add("Feedback Chaos", "FX", FX, feedback=1.0, resonance=0.8, drive=1.0, warmth=0.7, lfoFilter=0.5, lfoWave=L_SH,
    lfoRate=3.0, aSustain=1.0)
add("Drip", "FX", FX, osc1Wave=SIN, mixOsc2=0.0, cutoff=800, resonance=0.9, filterEnv=0.8, fDecay=0.08,
    fSustain=0.0, aDecay=0.2, aSustain=0.0, delayMix=0.4)

# ---------------------------------------------------------------- Sekvenser (9)
add("Odd Five Bass", "Sequence", MONO_BASS, "0! 0 12 . 7~",
    seqMode=SEQ, seqRate=R16, seqLength=5, seqAccLen=3, cutoff=380, resonance=0.5, filterEnv=0.55, fDecay=0.2,
    fSustain=0.1, drive=0.5, voiceMode=LEG, glide=0.04)
add("Thirteen Steps Lead", "Sequence", LEAD, "0! 3 7 12 . 10 7 3~ 5 . 0 -5 7?60",
    seqMode=SEQ, seqRate=R16, seqLength=13, seqAccLen=4, cutoff=1800, resonance=0.35, delayMix=0.3)
add("Euclid 3 of 8 Bass", "Sequence", MONO_BASS, "0! . . 0 . . 12 .",
    seqMode=SEQ, seqRate=R16, seqLength=8, seqAccLen=2, cutoff=300, resonance=0.4, filterEnv=0.6, fDecay=0.2,
    feedback=0.3, warmth=0.3)
add("Triplet Arp Up", "Sequence", KEYS, seqMode=ARP, seqRate=R8T, arpOctaves=2, seqLength=12, aDecay=0.6,
    delayMix=0.25)
add("Random Arp", "Sequence", PLUCK, "0 0?50 0 0?70",
    seqMode=ARP, seqRate=R16, arpOrder=RANDOM, arpOctaves=3, seqLength=16, delayMix=0.3, reverbMix=0.25)
add("Down Arp Seven", "Sequence", PLUCK, seqMode=ARP, seqRate=R16, arpOrder=DOWN, arpOctaves=2, seqLength=7,
    seqAccLen=4)
add("Swing Seq Bass", "Sequence", MONO_BASS, "0! . 0 12 . 0 . 7 0! . 3 . 0 12 . 10",
    seqMode=SEQ, seqRate=R16, seqSwing=0.5, seqLength=16, cutoff=420, resonance=0.4, filterEnv=0.5, fDecay=0.2,
    voiceMode=LEG)
add("Probability Plucks", "Sequence", PLUCK, "0?80 7?40 12?60 0?90 3?50 . 5?70 10?30 0! 7 .",
    seqMode=SEQ, seqRate=R16, seqLength=11, seqAccLen=4, delayMix=0.3)
add("Steppe Gallop", "Sequence", MONO_BASS, "0! 0 0 0! 0 0",
    seqMode=SEQ, seqRate=R16, seqLength=6, seqAccLen=3, filterType=PLP, cutoff=350, resonance=0.6,
    filterEnv=0.55, fDecay=0.15, fSustain=0.05, drive=0.6, aDecay=0.3, aSustain=0.5)

assert len(P) == 100, len(P)
names = [p[0] for p in P]
assert len(set(names)) == len(names), "dubbla namn"


def fmt(v):
    if isinstance(v, int):
        return str(v)
    s = repr(float(v))
    return s + "f"


# Nivåkalibrering: dB-justering av Master per preset (mätt med testprogrammet, topp ~0.7).
import json
LEVELS_FILE = Path(__file__).resolve().parent / "levels.json"
LEVELS = json.loads(LEVELS_FILE.read_text(encoding="utf-8")) if LEVELS_FILE.exists() else {}

lines = ["// GENERERAD AV tools/gen_presets.py – redigera skriptet, inte den här filen.\n"]
for name, cat, base, kw, pattern in P:
    vals = dict(base)
    vals.update(kw)
    if name in LEVELS:
        vals["master"] = round(max(-30.0, min(3.0, vals.get("master", -6.0) + LEVELS[name])), 1)
    body = ", ".join("{ %s, %s }" % (k, fmt(v)) for k, v in vals.items())
    pat = ', "%s"' % pattern if pattern else ""
    lines.append('        { "%s", "%s", { %s }%s },\n' % (name, cat, body, pat))

out = Path(__file__).resolve().parent.parent / "Source" / "PresetBank.inc"
out.write_text("".join(lines), encoding="utf-8")
print(f"{len(P)} presets -> {out}")
