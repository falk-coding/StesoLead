# Bakelit (Falk Instruments) – arbetsanteckningar för Claude

Polyfon analog-inspirerad synth som VST3 (Mac + Windows) och AU (Mac), byggd med JUCE 8.0.10.
Produkt: **Bakelit**, varumärke: **Falk Instruments** (ägs av Hans enskilda firma ijoli, liksom synkaoss).
Repo: **github.com/falk-coding/Bakelit** (hette StesoLead till 2026-11-01; gamla länkar omdirigeras).
Internt kodnamn **StesoLead** (klassnamn, CMake-target, mappnamn i molnmiljön) – syns aldrig för användaren.
GitHub-verktygen: använd `owner: falk-coding`, `repo: bakelit`.

## Användaren
Hans: tekniskt kunnig men inte utvecklare, svarar på svenska, vill ha korta skumbara svar (ADHD), komplett lösning direkt.
Kör Reason på Mac. Kostnadsmedveten. Säg emot om något är en dålig idé.

## Hårda regler
1. **Ändra aldrig** `PLUGIN_MANUFACTURER_CODE Falk` / `PLUGIN_CODE Stsl` eller ValueTree-typen `"StesoLead"` i
   `PluginProcessor.cpp` (apvts) – då hittar DAW:arna inte gamla projekt/sparat läge.
2. **Ändra aldrig ett parameter-ID** i `Parameters.h`. Nya parametrar: standardvärde som stänger av funktionen så
   att gamla projekt/presets låter som förut.
3. Fabrikspresets läggs **alltid sist** (programnumren sparas i projekt). Listan i GUI:t grupperas per kategori ändå.
4. Säg aldrig att något är klart förrän bygget + `StesoLeadTests` är grönt (lokalt) och CI (pluginval/auval) är grön.
5. Av/på-parametrar: använd `AudioParameterChoice {"Off","On"}`, inte `AudioParameterBool` (pluginval state restoration fallerar).
6. Synliga texter: **svenska med engelska inom parentes** (sektionsrubriker skrivs `"SVENSKA|English"`).
   Rattnamn/val på engelska. Använd `juce::String::fromUTF8` för åäö. Inga andras varumärken i UI/presets
   (Polivoks→Steppe, Juno→Chorus, ingen Moog).

## Bygga och testa (molnmiljön)
```bash
# JUCE lokalt (snabbare än FetchContent):
git clone --depth 1 --branch 8.0.10 https://github.com/juce-framework/JUCE.git /tmp/juce
sudo apt-get install -y libasound2-dev libxrandr-dev libxinerama-dev libxcursor-dev libxext-dev libxcomposite-dev xvfb
cmake -S . -B build -G Ninja -DCMAKE_BUILD_TYPE=Release -DJUCE_DIR=/tmp/juce
cmake --build build
cd /tmp/out && xvfb-run -a /home/user/StesoLead/build/StesoLeadTests_artefacts/Release/StesoLeadTests --wav-dir wav --screenshot shot.png
```
`BAKELIT_QUICK=1` hoppar över preset-renderingen (snabb körning). Testerna: alla presets (44.1/96 kHz: NaN, klipp, tystnad, hängande röster, CPU), röstlägen/pedal, spara/ladda läge
(även pluginval-liknande slumpvärden), sekvenser (export = uppspelning, polymetri, arp, bypass), stresstest 40 rundor,
egna förinställningar (inkl. gamla .stesolead), Polyrytm (motor, export = uppspelning, sparat läge, bypass), undertonernas frekvens,
skärmdumpar kompakt + utfällt + Polyrytm (`shot.png`, `shot-full.png`, `shot-poly.png`).
Att köra nedladdade binärer (t.ex. pluginval) lokalt nekas – pluginval/auval körs i CI.

## Release
Push till main → `.github/workflows/build.yml` kör Linux (tester + pluginval), macOS (`macos-26`, universal, ad hoc-signerat,
pluginval + auval) och Windows (pluginval). Byter man macOS-runner: byt även cache-nyckeln (Xcode-sökvägar ligger i cachen). Release: GitHub-verktyget `actions_run_trigger` → `run_workflow`,
`build.yml`, ref `main`, inputs `{"release_tag": "vX.Y.Z"}`. Kolla sedan `get_release_by_tag` att
`Bakelit-mac.zip` och `Bakelit-windows.zip` finns. Höj `project(... VERSION x.y.z)` i CMakeLists.txt.
Commits: författare `Hans Falk <hans.falk@gmail.com>`. Pusha direkt till main (enmansprojekt).

## Var saker finns
| Del | Fil |
|---|---|
| Parametrar | `Source/Parameters.*` |
| Fabrikspresets 1–35, Polyrytm 136–145, Subharmonic 146–149 | `Source/Presets.cpp` |
| Presets 36–135 | `tools/gen_presets.py` → `Source/PresetBank.inc` (nivåer i `tools/levels.json`) |
| Oscillator, envelope, Ladder-/Steppe-filter (`PolivoksFilter`), halvbandsdecimator | `Source/dsp/DspUtils.h` |
| Röster (3 osc, unison, 2x oversampling, feedback, warmth), MIDI, LFO | `Source/dsp/SynthEngine.*` |
| Chorus, delay, reverb | `Source/dsp/Effects.*` |
| Stegsekvenserare/arp/Polyrytm, MIDI-export | `Source/seq/Sequencer.*`, GUI `Source/SeqPanel.*`, `Source/PolyPanel.*` |
| Egna förinställningar, sparat läge, värdinfo | `Source/PluginProcessor.*` |
| GUI: rack-layout, skin, rubriker | `Source/PluginEditor.*`, `Source/UiCommon.h` |
| Installation | `packaging/mac`, `packaging/windows` |
| Manual sv/en | `docs/Manual-*.md` → `tools/make_manual.sh` → `docs/Manual-*.pdf` (följer med i release-zipparna). Uppdatera vid nya funktioner! |

## Status (2026-11-01, v1.8.0)
Klart: motor, Fat-funktioner, sekvenser med MIDI-export/markörer, mörkt slitet skin (v1.7: kall nattsvart plåt, punktljus över allt, rost/ärg, korn, slitna stålrattar, präglad logga) med svenska texter,
rack-layout med fällbar stegsekvenserare/klaviatur, 149 presets, egna förinställningar, namnbyte till Bakelit,
Polyrytm-läge (v1.7: 4 spår Bar/Div, euklid, tonhöjdssteg, ringvisning; parametern `polyOn` går före `seqMode`).
Hans har provlyssnat några presets – "låter VÄLDIGT bra". Undertoner (v1.7/1.8): `osc1SubDiv`/`osc2SubDiv`/`osc3SubDiv` ÷1–16 + nivåer `mixSubh1`/`mixSubh2`/`mixSubh3` (0 = av),
samma vågform och PW som oscillatorn (OSC 3 har PW sedan v1.8, `osc3PW`); rattarna ligger inramade i respektive OSC-sektion (Hans krav: tydligt vilken sub hör till vilken osc).
Skin: mörkret ligger i bakgrunden (paint), bara svagt ljuslager över kontrollerna – Hans krav: mörkt men allt läsbart.
Idé till senare: låta Polyrytm-spåren styra undertonernas delare (som originalets sekvenser gör).

**Kvar innan försäljning** (i ordning):
1. Hans lyssnar och testar i Reason (ljud, presets, dra MIDI, fönsterstorlek vid utfällning) – inget är provlyssnat än.
2. Byggflödet för privat repo: Mac/Windows bara vid release (Linux-tester vid push) – gör INNAN repot blir privat.
3. Signering: Apple Developer (notarisering) + Windows-kodsignering; riktiga installationsprogram (.pkg/.exe).
4. JUCE-licens för sluten kod (kolla juce.com), varumärkessök Falk Instruments/Bakelit (TMview, klass 9), domän falkinstruments.com.
5. Betatest, ljuddemos, produktsida, butik via merchant of record (Paddle/Lemon Squeezy).
Rack Extension (Reason) avvaktas – stort jobb, kräver lokal testning hos Hans.
