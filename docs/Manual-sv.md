# Bakelit – manual

**Falk Instruments · version 1.8**

Bakelit är en polyfon analog-inspirerad synt med tre oscillatorer, undertoner, två filtertyper, stegsekvenserare, arpeggiator och polyrytm. Den här manualen tar dig från installation till eget ljud på tio minuter.

---

## 1. Installera

**Mac (VST3 + AU)**

1. Packa upp `Bakelit-mac.zip`.
2. Högerklicka på **Installera.command** → **Öppna** (första gången, eftersom programmet inte är notariserat än).
3. Starta om din DAW. Bakelit finns under *Falk Instruments*.

**Windows (VST3)**

1. Packa upp `Bakelit-windows.zip`.
2. Dubbelklicka på **Installera.bat** och godkänn administratörsfrågan.
3. Starta om din DAW.

**Reason:** Bakelit finns i webbläsaren under *Instruments → Plugins → Falk Instruments*. Fungerar även i Ableton, Logic (AU), Cubase, FL Studio, Bitwig, Studio One och Reaper.

---

## 2. Kom igång på en minut

1. Lägg Bakelit på ett instrumentspår och spela.
2. Bläddra bland ljuden med **<** och **>** uppe till höger, eller klicka på namnet för att se alla ljud i kategorier.
3. Prova kategorierna **Sequence**, **Polyrhythm** och **Subharmonic** – håll en tangent och tryck Play i DAW:n.
4. Gillar du något du skruvat fram: **Spara (Save)**.

**Bra att veta**

- **Dubbelklick** på en ratt = standardvärde.
- Dra uppåt/nedåt på en ratt för att vrida. Värdet visas medan du drar.
- Hovra över en kontroll för en kort förklaring.
- Dra i fönstrets nedre högra hörn för att göra det större eller mindre.
- **Sekv. (Seq)** och **Klav. (Keys)** fäller ut sekvenseraren och klaviaturen. Lampan på Sekv. lyser när sekvenseraren spelar fast den är infälld.

---

## 3. Så hänger ljudet ihop

```
OSC 1 ─┐
OSC 2 ─┼─► NIVÅER ─► FILTER ─► VOLYMKURVA ─► EFFEKTER ─► UTGÅNG
OSC 3 ─┤  (mixer)     ▲              ▲
Under- ┘          FILTERKURVA     velocity
toner                 ▲
                MODULATION (LFO): tonhöjd, filter, PW
```

---

## 4. Sektionerna

### Oscillatorer (OSC 1–3)
**OSC 1 är referensen.** Den spelar exakt den ton du trycker och väljer register med **Octave** (−2 till +2). **OSC 2 och 3 stäms mot OSC 1**: **Semi** ger intervall (+7 = kvint, ±12 = oktav), **Fine** stämmer isär lite för en fet, svävande klang.

| Ratt | Gör |
|---|---|
| Wave | Saw, Square, Triangle, Sine |
| PW | Pulsbredd för Square (50 % = fyrkant, smalare = tunnare, nasalare) |
| **Sub N Div** | Undertonens delare: oscillatorns frekvens ÷ 1–16 |
| **Sub N** | Undertonens nivå (0 = av) |

**Undertoner (Sub 1–3)** ligger inramade i varje oscillator. De följer sin oscillators tonhöjd, vågform och PW exakt. ÷2 = en oktav ner, ÷3 = en kvint under det, ÷4 = två oktaver ner, ÷5, ÷6 … ger allt mörkare, mer sällsamma klanger. Prova OSC 1 med Sub 1 Div = 3 och Sub 1 = 50 %.

### Nivåer (Mixer)
Volym för **OSC 1–3**, **Sub Oct** (en klassisk fyrkant en oktav under OSC 1) och **Noise** (brus).

### Filter (VCF)
| Ratt | Gör |
|---|---|
| Type | **Ladder** (rund, fet, 24 dB) · **Steppe LP** (aggressiv, rå) · **Steppe BP** (bandpass, nasal) |
| Cutoff | Brytfrekvens – mörkt/ljust |
| Reso | Resonans – tjut vid brytfrekvensen; självsvänger högst upp |
| Drive | Överstyrning in i filtret |
| Env | Hur mycket Filterkurvan öppnar filtret |
| Key | Filtret följer tangenterna (ljusare uppåt) |
| Velocity | Hårdare anslag = ljusare |
| Feedback | Filtrets utgång tillbaka in – smutsigare, tjockare |
| Warmth | Mjuk mättnad efter filtret |

### Filterkurva och Volymkurva (ADSR)
**Attack** (insvängning), **Decay** (avklingning), **Sustain** (nivå medan tangenten hålls), **Release** (efterklang när du släpper). Volymkurvans **Velocity** styr hur mycket anslaget påverkar volymen.

### Modulation (LFO)
En långsam oscillator som rör ljudet: **Wave** (form), **Rate** (hastighet), **Fade In** (vibratot smyger in), och mängd till **Pitch** (vibrato), **Filter** (wah) och **PW** (pulsbreddsmodulering, alla tre oscillatorer). Modhjulet ger extra vibrato.

### Stämma (Voice)
| Ratt | Gör |
|---|---|
| Mode | **Poly** (ackord) · **Mono** · **Legato** (glid bara när tonerna binds) |
| Glide | Portamento mellan toner |
| Bend | Pitch bend-omfång |
| Drift | Analog ostadighet i stämning och filter |
| Spread | Sprider stämmorna i stereo |
| Unison | 1–4 lager per stämma |
| Detune | Hur mycket unison-lagren stämmer isär |

### Effekter och Utgång
**Chorus** (Off/I/II), **Delay** (Time, Feedback, mängd), **Reverb** (mängd). **Volume** är huvudvolymen. *Stämmor* uppe till höger visar hur många röster som spelar.

---

## 5. Stegsekvenserare och arpeggiator

Fäll ut med **Sekv. (Seq)**. Följer DAW:ns tempo; utan transport kör den på egen klocka så länge du håller en tangent.

**Mode**

- **Arp** – spelar ackordet du håller. **Arp** väljer ordning (Up, Down, Up/Down, Random, As Played), **Octaves** 1–4.
- **Seq** – spelar mönstret, transponerat av tangenten du håller.

**Rutnätet** (klicka/dra): **Step** på/av · **Accent** · **Tie** (bind till nästa steg = glid) · **Pitch** (dra upp/ner, ±24) · **Prob** (sannolikhet) · **Gate** (tonlängd). Dubbelklick nollställer Pitch/Prob/Gate.

| Kontroll | Gör |
|---|---|
| Rate | Stegens längd (1/4 … 1/32, trioler, punkterade) |
| Swing | Fördröjer vartannat steg |
| Gate | Längd på alla toner |
| Steps / Acc steps / Gate steps | Egna längder (1–32) → polymetri, t.ex. 7 steg mot 4/4 |
| Latch | Fortsätter spela när du släpper tangenterna |
| MIDI out | Skickar tonerna till DAW:n (inte Reason/Logic – dra MIDI i stället) |
| Euclid | Sprider *Hits* jämnt över stegen. **Init** nollställer mönstret |

**Dra MIDI till DAW** – dra rutan till ett MIDI-spår så får du exakt det som spelas som MIDI-klipp. Längden tas från DAW:ns loop-markörer eller från **Från/Till** takt. Sekvenseraren stängs sedan av så att det inte låter dubbelt; **Slå på (Re-enable)** startar den igen.

---

## 6. Polyrytm

Slå på **Polyrhythm** i sekvenserarens rad. Den går före Arp/Seq och spelar från tangenten du håller. Ringarna visar var slagen ligger – där spåren möts blir det **accent**.

Fyra spår, var och en med:

| Kolumn | Gör |
|---|---|
| Lane (1–4) | Spåret av/på |
| Type | **Bar** = *Pulses* slag jämnt fördelade över cykeln · **Div** = ett slag var *Pulses*:e steg av Rate (klockdelare) |
| Pulses | Antal slag per cykel (eller delare i Div) |
| Hits | Hur många av slagen som spelas, jämnt utspridda (euklidiskt) |
| Pitch | Spårets tonhöjd mot tangenten, ±24 halvtoner |
| Steps + Notes | 1–8 tonhöjder som spåret går igenom, ett per slag. Dra i rutorna |

**Cycle bars** (1–4) = cykelns längd i takter för Bar-spår.

**Snabbval:** **4:3**, **3:2**, **5:4**, **7:4**, **Div** (÷2 ÷3 ÷5 ÷7), **Init**.

**Kom igång:** klicka **4:3**, håll en ton och tryck Play. Ändra sedan *Pulses* på spår 2 till 5, eller slå på spår 3.

---

## 7. Egna ljud

- **Spara (Save)** sparar alla rattar och mönster. Samma namn = skriv över (frågar först).
- **Ta bort (Delete)** tar bort valt eget ljud.
- Egna ljud ligger överst i listan under **Egna (User)**.
- Filerna (`.bakelit`) finns i **Dokument/Falk Instruments/Bakelit/Presets** – kopiera mappen för säkerhetskopia eller för att flytta mellan Mac och Windows.
- DAW-projektet minns allt, även mönster och utfällda paneler.

---

## 8. Felsökning

| Problem | Lösning |
|---|---|
| Syns inte i DAW:n | Starta om DAW:n. Kör om installationen. Låt DAW:n söka efter nya plugins |
| Mac: "kan inte öppnas" | Högerklicka → Öppna, eller Systeminställningar → Integritet och säkerhet → Öppna ändå |
| Sekvenseraren tyst | Mode eller Polyrhythm måste vara på, och en tangent hållas (eller Latch) |
| Dubbla toner efter MIDI-dragning | Sekvenseraren ska vara av – det sker automatiskt; kolla att *Slå på* inte är tryckt |
| Hackar | Höj buffertstorleken i DAW:n, eller sänk Unison |

---

*Bakelit © Falk Instruments. VST är ett varumärke som tillhör Steinberg Media Technologies GmbH.*
