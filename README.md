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
- 35 fabrikspresets (varav 5 med sekvenser), skalbart GUI, inbyggd klaviatur

## Fat-motorn (v1.2)
- **Tre oscillatorer** (som på Minimoog) + sub-oktav + brus. Osc 3 står på -12 halvtoner som standard.
- **Filtertyp:** *Ladder* (24 dB, Moog-stil), *Polivoks LP* och *Polivoks BP* (12 dB med mättande integratorer
  – rått, skrikigt resonansläge inspirerat av den ryska Polivoks).
- **Feedback:** filterutgången tillbaka in i filtret (Moog-loopen) – morr och tjocklek.
- **Warmth:** asymmetrisk rörliknande mättning efter filtret (jämna övertoner).
- **Unison** 1–4 stackar per röst med **Detune** upp till ±25 cent.
- **2x oversampling** för oscillatorer, filter, feedback och mättning – mindre digital aliasing, varmare drive.
- 11 nya presets i *Fat Bass*, *Fat Lead*, *Fat Poly* och *Sequence*. Pluginet startar på **Three Osc Lead**.
- Nya funktioner är avstängda som standard, så äldre presets och projekt låter som förut.
- **Skin:** sliten sovjetisk apparat – grå plåt med repor, bakelitrattar, kyrilliska sektionsnamn, signallampor i sekvensern.

## Sekvenser / arpeggiator (v1.1)
- **Seq:** stegen spelas relativt tangenten du håller (transponera live). **Arp:** spelar ackordet du håller
  (Up, Down, Up/Down, Random, As Played, 1–4 oktaver). **Latch** håller kvar ackordet när du släpper tangenterna.
- **1–32 steg, oberoende av takten.** Steps, Acc steps och Gate steps har egna längder → polymetri (t.ex. 7 mot 5 mot 4/4).
- Per steg: på/av, accent, tie/slide, tonhöjd ±24, sannolikhet och gate. Euclid-knappen sprider *Hits* jämnt över *Steps*.
- Synkad mot DAW:ns tempo. Positionen räknas från takt 1, så mönstret ligger alltid på samma ställe i låten.
- **Dra MIDI till DAW:** dra rutan till ett MIDI-spår. Längd = DAW:ns locators (om DAW:n skickar dem) eller Från/Till takt.
  Klippet är exakt det som spelas – även sannolikhetsstegen. Sekvensern stängs sedan av automatiskt så att det
  inte låter dubbelt (**Slå på igen** finns i panelen). Släpp klippet på vänster locator / Från-takten.
- **MIDI out:** skickar sekvensens toner live till DAW:ar som kan spela in MIDI från instrument
  (t.ex. Ableton, Bitwig, Cubase, FL Studio). Logic och Reason stöder det troligen inte – använd dra-och-släpp.
- Pattern sparas med projektet och med presets. Mode/Rate/Swing m.m. är automatiserbara.

**Format:** VST3 (Mac + Windows) och AU (Mac). Fungerar i Reason, Ableton, Cubase, FL Studio, Bitwig,
Studio One, Reaper, Logic och GarageBand. Pro Tools (AAX) stöds inte – kräver avtal och betald signering hos Avid.

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
| `Source/seq/Sequencer.*` | Sekvenser/arp, export till MIDI-fil (samma kod för realtid och export) |
| `Source/SeqPanel.*` | Sekvenser-GUI: stegrutnät, dragruta |
| `Source/PluginEditor.*` | GUI |

## Licens
Typsnittet PT Sans Narrow (ParaType) är inbäddat under SIL Open Font License – se `fonts/OFL.txt`.
JUCE används under JUCE 8-licensen (gratis för privat bruk/låg omsättning). VST är ett varumärke som tillhör Steinberg Media Technologies GmbH.
