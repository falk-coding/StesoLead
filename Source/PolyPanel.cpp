#include "PolyPanel.h"

namespace steso::ui
{
namespace
{
    void markDirty (StesoLeadProcessor& p)
    {
        p.updateHostDisplay (juce::AudioProcessor::ChangeDetails().withNonParameterStateChanged (true));
    }

    // Senaste spelade slaget (med euklid) vid ppq, eller INT64_MIN.
    int64_t lastHitAt (const seq::Settings& s, int lane, double ppq)
    {
        auto n = seq::polyPulseAt (s, lane, ppq);
        for (int k = 0; k <= seq::polyMaxPulses; ++k, --n)
            if (seq::polyPulseIsHit (s, lane, n))
                return n;
        return std::numeric_limits<int64_t>::min();
    }

    int posMod (int64_t n, int m) { const auto r = (int) (n % m); return r < 0 ? r + m : r; }

    const char* columnNames[] { "Lane", "Type", "Pulses", "Hits", "Pitch", "Steps", "Notes" };
}

juce::Colour polyLaneColour (int lane)
{
    static const juce::Colour c[] { colours::lampRed, colours::lampAmber, juce::Colour (0xff9cc28e), juce::Colour (0xff86b4d4) };
    return c[lane & 3];
}

//==============================================================================
void PolyRings::paint (juce::Graphics& g)
{
    const auto s = processor.currentSeqSettings();
    const double ppq = processor.getCurrentPpq();
    const double cycle = s.polyCyclePpq();
    const double c0 = ppq >= 0.0 ? std::floor (ppq / cycle) * cycle : 0.0;

    auto area = getLocalBounds().toFloat().reduced (4.0f);
    const float size = std::min (area.getWidth(), area.getHeight());
    area = area.withSizeKeepingCentre (size, size);
    g.setColour (juce::Colours::black.withAlpha (0.25f));
    g.fillEllipse (area);
    drawEngravedBox (g, area, 0.0f);

    const auto centre = area.getCentre();
    const float outer = size * 0.5f - 7.0f;
    constexpr float twoPi = juce::MathConstants<float>::twoPi;
    auto angleOf = [&] (double t) { return (float) ((t - c0) / cycle) * twoPi; };
    auto pointAt = [&] (float radius, float angle)
    {
        return centre + juce::Point<float> (radius * std::sin (angle), -radius * std::cos (angle));
    };

    // Taktmarkeringar (en per takt i cykeln) i yttre kanten.
    for (int b = 0; b < s.polyBars; ++b)
    {
        const float a = twoPi * (float) b / (float) s.polyBars;
        g.setColour (colours::text.withAlpha (0.5f));
        g.drawLine (juce::Line<float> (pointAt (outer + 2.0f, a), pointAt (outer + 6.0f, a)), 1.5f);
    }

    juce::String ratio;
    for (int lane = 0; lane < seq::polyLanes; ++lane)
    {
        const auto& L = s.lanes[(size_t) lane];
        const float radius = outer * (1.0f - 0.22f * (float) lane);
        const auto col = polyLaneColour (lane);

        g.setColour (col.withAlpha (L.on ? 0.35f : 0.08f));
        g.drawEllipse (juce::Rectangle<float> (radius * 2.0f, radius * 2.0f).withCentre (centre), 1.0f);
        if (! L.on)
            continue;

        if (L.type == 0)
            ratio << (ratio.isEmpty() ? "" : ":") << juce::jlimit (1, seq::polyMaxPulses, L.pulses);
        else
            ratio << (ratio.isEmpty() ? "" : ":") << juce::String::fromUTF8 ("÷") << L.pulses;

        const double interval = seq::polyIntervalPpq (s, lane);
        auto n = seq::polyPulseAt (s, lane, c0);
        if (seq::polyPulsePpq (s, lane, n) < c0 - 1.0e-9)
            ++n;

        for (int k = 0; k < 64; ++k, ++n)
        {
            const double t = seq::polyPulsePpq (s, lane, n);
            if (t >= c0 + cycle - 1.0e-9)
                break;
            const auto p = pointAt (radius, angleOf (t));
            if (seq::polyPulseIsHit (s, lane, n))
            {
                const bool lit = ppq >= t && ppq < t + std::min (0.25, interval * 0.6);
                const float d = lit ? 11.0f : 8.0f;
                drawLamp (g, juce::Rectangle<float> (d, d).withCentre (p), col, col.darker (2.2f), lit);
            }
            else
            {
                g.setColour (col.withAlpha (0.3f));
                g.fillEllipse (juce::Rectangle<float> (3.0f, 3.0f).withCentre (p));
            }
        }
    }

    if (ppq >= 0.0)
    {
        g.setColour (colours::text.withAlpha (0.7f));
        g.drawLine (juce::Line<float> (centre, pointAt (outer + 4.0f, angleOf (ppq))), 1.2f);
    }

    g.setColour (colours::text);
    g.setFont (panelFont (ratio.length() > 7 ? 11.0f : 14.0f));
    g.drawText (ratio, juce::Rectangle<float> (outer * 0.5f, 18.0f).withCentre (centre), juce::Justification::centred);
}

//==============================================================================
LaneSteps::LaneSteps (StesoLeadProcessor& p, int laneIndex) : processor (p), lane (laneIndex)
{
    setTooltip (juce::String::fromUTF8 ("Tonhöjd per slag: dra upp/ner, dubbelklick = 0. Steps väljer hur många som används. "
                                        "(Pitch per hit: drag up/down, double-click = 0.)"));
}

juce::Rectangle<float> LaneSteps::cell (int i) const
{
    const float w = (float) getWidth() / (float) seq::polyMaxSteps;
    return { (float) i * w, 0.0f, w, (float) getHeight() };
}

int LaneSteps::cellAt (float x) const
{
    return juce::jlimit (0, seq::polyMaxSteps - 1, (int) (x / ((float) getWidth() / (float) seq::polyMaxSteps)));
}

void LaneSteps::paint (juce::Graphics& g)
{
    const auto s = processor.currentSeqSettings();
    const auto& L = s.lanes[(size_t) lane];
    const int steps = juce::jlimit (1, seq::polyMaxSteps, L.steps);
    const double ppq = processor.getCurrentPpq();

    int playIdx = -1;
    if (ppq >= 0.0 && L.on && s.mode == seq::Mode::poly)
    {
        const auto n = lastHitAt (s, lane, ppq);
        if (n != std::numeric_limits<int64_t>::min())
            playIdx = posMod (seq::polyHitIndex (s, lane, n), steps);
    }

    const auto col = polyLaneColour (lane);
    auto& pat = processor.getPattern();
    for (int i = 0; i < seq::polyMaxSteps; ++i)
    {
        const auto r = cell (i).reduced (1.5f);
        const bool inLane = i < steps;
        const float alpha = (inLane ? 1.0f : 0.28f) * (L.on ? 1.0f : 0.5f);

        g.setColour (colours::bakelite.withAlpha (0.7f));
        g.fillRect (r);
        drawEngravedBox (g, r, 0.6f);

        const int p = pat.lanePitch[(size_t) lane][(size_t) i].load();
        const float mid = r.getCentreY();
        const float hh = r.getHeight() * 0.5f - 2.0f;
        const float y2 = mid - hh * (float) p / 24.0f;
        if (p != 0)
        {
            g.setColour (col.withAlpha (0.35f * alpha));
            g.fillRect (juce::Rectangle<float>::leftTopRightBottom (r.getX() + 3.0f, std::min (mid, y2),
                                                                    r.getRight() - 3.0f, std::max (mid, y2)));
        }
        g.setColour (colours::text.withAlpha (alpha));
        g.setFont (panelFont (11.0f));
        g.drawText (p == 0 ? juce::String ("0") : (p > 0 ? "+" : "") + juce::String (p), r, juce::Justification::centred);

        if (i == playIdx)
        {
            g.setColour (col);
            g.drawRect (r.expanded (0.5f), 1.4f);
        }
    }
}

void LaneSteps::mouseDown (const juce::MouseEvent& e)
{
    dragCell = cellAt (e.position.x);
    startPitch = processor.getPattern().lanePitch[(size_t) lane][(size_t) dragCell].load();
}

void LaneSteps::mouseDrag (const juce::MouseEvent& e)
{
    if (dragCell < 0)
        return;
    const int dy = juce::roundToInt ((float) e.getDistanceFromDragStartY() / 5.0f);
    processor.getPattern().lanePitch[(size_t) lane][(size_t) dragCell] = juce::jlimit (-24, 24, startPitch - dy);
    markDirty (processor);
    repaint();
}

void LaneSteps::mouseDoubleClick (const juce::MouseEvent& e)
{
    processor.getPattern().lanePitch[(size_t) lane][(size_t) cellAt (e.position.x)] = 0;
    markDirty (processor);
    repaint();
}

//==============================================================================
PolyPanel::PolyPanel (StesoLeadProcessor& p) : processor (p), rings (p)
{
    addAndMakeVisible (rings);
    auto& apvts = processor.apvts;
    using SA = juce::AudioProcessorValueTreeState::SliderAttachment;

    auto setupInc = [this] (juce::Slider& s, const juce::String& tip)
    {
        s.setTextBoxStyle (juce::Slider::TextBoxLeft, false, 28, 20);
        s.setIncDecButtonsMode (juce::Slider::incDecButtonsDraggable_Vertical);
        s.setTooltip (tip);
        addAndMakeVisible (s);
    };

    for (int l = 0; l < seq::polyLanes; ++l)
    {
        auto& r = rows[(size_t) l];
        r.on.setButtonText (juce::String (l + 1));
        r.on.setClickingTogglesState (true);
        r.on.setColour (juce::TextButton::buttonOnColourId, polyLaneColour (l));
        r.on.setTooltip (juce::String::fromUTF8 ("Spår ") + juce::String (l + 1) + juce::String::fromUTF8 (" av/på (lane on/off)"));
        r.on.onClick = [this] { rings.repaint(); };
        addAndMakeVisible (r.on);
        r.onA = std::make_unique<juce::AudioProcessorValueTreeState::ButtonAttachment> (apvts, pid::polyLaneOn (l), r.on);

        r.type.addItemList (seq::polyTypeNames, 1);
        r.type.setTooltip (juce::String::fromUTF8 ("Bar = Pulses slag jämnt fördelade över cykeln. "
                                                   "Div = ett slag var Pulses:e steg av Rate (klockdelare). "
                                                   "(Bar = pulses per cycle, Div = clock divider)"));
        addAndMakeVisible (r.type);
        r.typeA = std::make_unique<juce::AudioProcessorValueTreeState::ComboBoxAttachment> (apvts, pid::polyLaneType (l), r.type);

        setupInc (r.pulses, juce::String::fromUTF8 ("Slag per cykel, eller delare i Div-läge (pulses per cycle / divider)"));
        setupInc (r.hits, juce::String::fromUTF8 ("Hur många av slagen som spelas, jämnt utspridda – euklidiskt (hits, Bar only)"));
        setupInc (r.pitch, juce::String::fromUTF8 ("Spårets tonhöjd i halvtoner mot tangenten (lane pitch)"));
        setupInc (r.steps, juce::String::fromUTF8 ("Antal tonhöjdssteg spåret går igenom, ett per slag (pitch steps)"));
        r.pulsesA = std::make_unique<SA> (apvts, pid::polyLanePulses (l), r.pulses);
        r.hitsA = std::make_unique<SA> (apvts, pid::polyLaneHits (l), r.hits);
        r.pitchA = std::make_unique<SA> (apvts, pid::polyLanePitch (l), r.pitch);
        r.stepsA = std::make_unique<SA> (apvts, pid::polyLaneSteps (l), r.steps);
        for (auto* s : { &r.pulses, &r.hits, &r.pitch, &r.steps })
            s->onValueChange = [this] { repaint(); rings.repaint(); };
        r.type.onChange = [this] { rings.repaint(); };

        r.notes = std::make_unique<LaneSteps> (processor, l);
        addAndMakeVisible (*r.notes);
    }

}

PolyPanel::~PolyPanel() = default;

void PolyPanel::setParam (const char* id, float value)
{
    if (auto* p = processor.apvts.getParameter (id))
        p->setValueNotifyingHost (p->convertTo0to1 (value));
}

void PolyPanel::applySetup (std::initializer_list<LaneSetup> lanes)
{
    int l = 0;
    for (const auto& d : lanes)
    {
        setParam (pid::polyLaneOn (l), d.on ? 1.0f : 0.0f);
        setParam (pid::polyLaneType (l), (float) d.type);
        setParam (pid::polyLanePulses (l), (float) d.pulses);
        setParam (pid::polyLaneHits (l), (float) d.hits);
        setParam (pid::polyLanePitch (l), (float) d.pitch);
        ++l;
    }
    for (; l < seq::polyLanes; ++l)
        setParam (pid::polyLaneOn (l), 0.0f);
    repaint();
    rings.repaint();
}

void PolyPanel::resetAll()
{
    if (auto* p = processor.apvts.getParameter (pid::polyBars))
        p->setValueNotifyingHost (p->getDefaultValue());
    for (int l = 0; l < seq::polyLanes; ++l)
        for (const auto* id : { pid::polyLaneOn (l), pid::polyLaneType (l), pid::polyLanePulses (l),
                                pid::polyLaneHits (l), pid::polyLanePitch (l), pid::polyLaneSteps (l) })
            if (auto* p = processor.apvts.getParameter (id))
                p->setValueNotifyingHost (p->getDefaultValue());
    processor.getPattern().resetLanes();
    markDirty (processor);
    repaint();
    for (auto& r : rows)
        r.notes->repaint();
    rings.repaint();
}

void PolyPanel::paint (juce::Graphics& g)
{
    g.setFont (panelFont (11.0f).withExtraKerningFactor (0.08f));
    g.setColour (colours::text.withAlpha (0.75f));
    for (size_t c = 0; c < std::size (columnNames); ++c)
        g.drawText (juce::String (columnNames[c]).toUpperCase(),
                    juce::Rectangle<int> (columnX[c], headerArea.getY(), columnW[c], headerArea.getHeight()),
                    juce::Justification::centredLeft);
}

void PolyPanel::resized()
{
    auto r = getLocalBounds();
    rings.setBounds (r.removeFromLeft (std::min (r.getHeight(), 170)));
    r.removeFromLeft (10);

    headerArea = r.removeFromTop (20);

    const int widths[] { 34, 56, 68, 68, 68, 62 };
    const int gap = 5;
    const int rowH = r.getHeight() / seq::polyLanes;

    {
        int x = r.getX();
        for (size_t c = 0; c < 6; ++c)
        {
            columnX[c] = x;
            columnW[c] = widths[c];
            x += widths[c] + gap;
        }
        columnX[6] = x;
        columnW[6] = r.getRight() - x;
    }

    for (auto& row : rows)
    {
        auto line = r.removeFromTop (rowH).reduced (0, 3);
        juce::Component* comps[] { &row.on, &row.type, &row.pulses, &row.hits, &row.pitch, &row.steps };
        for (size_t c = 0; c < 6; ++c)
        {
            comps[c]->setBounds (line.removeFromLeft (widths[c]));
            line.removeFromLeft (gap);
        }
        row.notes->setBounds (line);
    }
}

void PolyPanel::refresh()
{
    const double ppq = processor.getCurrentPpq();
    if (std::abs (ppq - lastPpq) > 1.0e-9)
    {
        // Rita bara om när något rör sig (eller precis har stannat).
        if (ppq >= 0.0 || lastPpq >= 0.0)
        {
            rings.repaint();
            for (auto& r : rows)
                r.notes->repaint();
        }
        lastPpq = ppq;
    }
}
} // namespace steso::ui
