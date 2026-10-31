# StesoLead

Polyfon analog-inspirerad synth som VST3/AU-plugin (macOS och Windows) byggd med [JUCE](https://juce.com).

## Ladda ner och installera
Senaste versionen finns under **Releases** (höger sida på repots startsida).

- **Mac:** `StesoLead-mac.zip` → packa upp → dubbelklicka på `Installera.command` (första gången: högerklicka → Öppna).
- **Windows:** `StesoLead-windows.zip` → packa upp → dubbelklicka på `Installera.bat`.

Starta sedan om Reason/din DAW → Instruments → Falk → StesoLead.
Detaljer: `packaging/mac/INSTALLERA.txt` och `packaging/windows/INSTALLERA.txt`.

Ny release: **Actions → Build → Run workflow** med `release_tag` (t.ex. `v1.0.1`), eller pusha en tagg `v*`.

## Funktioner (v1.0)
- 8 röster, Poly / Mono / Legato, glide, sustainpedal, pitchbend (1–12 st), modhjul → vibrato
- 2 bandbegränsade oscillatorer (saw, square med PWM, triangle, sine) + sub-oktav + brus
- 24 dB ladder-lågpass (Moog-stil) med resonans upp till självsvängning och drive
- Exponentiella ADSR för filter och amp, velocity till filter och amp
- LFO (sine, triangle, saw, square, S&H) med fade in → pitch, filter, PWM
- Analog drift per röst, stereospridning
- Chorus (Juno-stil I/II), stereodelay med lågpass i återkopplingen, reverb
- 20 fabrikspresets, skalbart GUI, inbyggd klaviatur

## Utveckling
```bash
cmake -S . -B build -G Ninja -DCMAKE_BUILD_TYPE=Release   # -DJUCE_DIR=<lokal JUCE> för att slippa nedladdning
cmake --build build
xvfb-run -a build/StesoLeadTests_artefacts/Release/StesoLeadTests --wav-dir ljudprov --screenshot shot.png
```
Testerna renderar alla presets (44.1 och 96 kHz) och kontrollerar NaN, klipp, tystnad, hängande röster,
CPU, röstlägen, sustainpedal och att sparat läge laddas exakt. CI kör dessutom `pluginval` (strikthet 10) och `auval`.

| Fil | Innehåll |
|---|---|
| `Source/Parameters.*` | Alla parametrar (ändra aldrig ett ID – sparade projekt tappar värdet) |
| `Source/Presets.cpp` | Fabrikspresets |
| `Source/dsp/DspUtils.h` | Oscillator, envelope, ladderfilter |
| `Source/dsp/SynthEngine.*` | Röster, röststöld, MIDI, LFO |
| `Source/dsp/Effects.*` | Chorus, delay, reverb |
| `Source/PluginEditor.*` | GUI |

## Licens
JUCE används under JUCE 8-licensen (gratis för privat bruk/låg omsättning). VST är ett varumärke som tillhör Steinberg Media Technologies GmbH.
