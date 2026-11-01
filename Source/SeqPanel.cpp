#include "SeqPanel.h"

namespace steso::ui
{
namespace
{
    constexpr float rowWeights[] { 1.0f, 0.8f, 0.8f, 1.5f, 1.0f, 1.0f };
    const char* rowNames[] { "Step", "Accent", "Tie", "Pitch", "Prob", "Gate" };

    void markDirty (StesoLeadProcessor& p)
    {
        p.updateHostDisplay (juce::AudioProcessor::ChangeDetails().withNonParameterStateChanged (true));
    }

    juce::String formatBar (double ppq, double barPpq)
    {
        const double b = ppq / barPpq + 1.0;
        return std::abs (b - std::round (b)) < 0.01 ? juce::String (juce::roundToInt (b)) : juce::String (b, 2);
    }
}

//==============================================================================
StepGrid::StepGrid (StesoLeadProcessor& p) : processor (p)
{
    setRepaintsOnMouseActivity (false);
}

int StepGrid::laneLength (int row) const
{
    const auto s = processor.currentSeqSettings();
    if (row == rowAccent) return s.accentLength;
    if (row == rowGate) return s.gateLength;
    return s.length;
}

juce::Rectangle<float> StepGrid::cell (int row, int step) const
{
    float total = 0.0f;
    for (auto w : rowWeights) total += w;

    const float h = (float) getHeight();
    float y = 0.0f;
    for (int r = 0; r < row; ++r)
        y += h * rowWeights[r] / total;

    const float colW = (float) (getWidth() - labelWidth) / (float) seq::maxSteps;
    return { (float) labelWidth + (float) step * colW, y, colW, h * rowWeights[row] / total };
}

int StepGrid::stepAt (float x) const
{
    const float colW = (float) (getWidth() - labelWidth) / (float) seq::maxSteps;
    if (x < (float) labelWidth)
        return -1;
    return juce::jlimit (0, seq::maxSteps - 1, (int) ((x - (float) labelWidth) / colW));
}

int StepGrid::rowAt (float y) const
{
    for (int r = 0; r < numRows; ++r)
        if (cell (r, 0).contains (cell (r, 0).getX() + 1.0f, y))
            return r;
    return numRows - 1;
}

void StepGrid::paint (juce::Graphics& g)
{
    auto& pat = processor.getPattern();
    const auto step = processor.getCurrentStep();

    g.setFont (panelFont (12.0f).withExtraKerningFactor (0.08f));

    for (int row = 0; row < numRows; ++row)
    {
        const auto first = cell (row, 0);
        g.setColour (colours::text.withAlpha (0.8f));
        g.drawText (juce::String (rowNames[row]).toUpperCase(),
                    juce::Rectangle<float> (0.0f, first.getY(), (float) labelWidth - 8.0f, first.getHeight()),
                    juce::Justification::centredRight);

        const int len = laneLength (row);
        const int playIdx = step >= 0 ? (int) (step % len) : -1;

        for (int i = 0; i < seq::maxSteps; ++i)
        {
            const auto idx = (size_t) i;
            const auto r = cell (row, i).reduced (1.5f);
            const bool inLane = i < len;
            const float alpha = inLane ? 1.0f : 0.28f;

            // Nedsänkt fält, var fjärde steg lite ljusare (taktmarkering).
            g.setColour (colours::bakelite.withAlpha ((i / 4) % 2 == 0 ? 0.75f : 0.55f));
            g.fillRect (r);
            drawEngravedBox (g, r, 0.6f);

            switch (row)
            {
                case rowOn:
                {
                    const float d = std::min (r.getWidth(), r.getHeight()) - 3.0f;
                    drawLamp (g, r.withSizeKeepingCentre (d, d), colours::lampRed, colours::lampRedOff, pat.on[idx].load(), alpha);
                    break;
                }
                case rowAccent:
                {
                    const float d = std::min (r.getWidth(), r.getHeight()) - 4.0f;
                    drawLamp (g, r.withSizeKeepingCentre (d, d), colours::lampAmber, colours::lampAmberOff, pat.accent[idx].load(), alpha);
                    break;
                }
                case rowTie:
                    if (pat.tie[idx].load())
                    {
                        g.setColour (colours::text.withAlpha (0.85f * alpha));
                        g.fillRect (r.withHeight (3.0f).withCentre (r.getCentre()).withTrimmedLeft (2.0f));
                    }
                    break;
                case rowPitch:
                {
                    const int p = pat.pitch[idx].load();
                    const float mid = r.getCentreY();
                    const float hh = r.getHeight() * 0.5f - 2.0f;
                    const float y2 = mid - hh * (float) p / 24.0f;
                    g.setColour (colours::dimText.withAlpha (0.35f));
                    g.drawHorizontalLine ((int) mid, r.getX() + 2.0f, r.getRight() - 2.0f);
                    if (p != 0)
                    {
                        g.setColour (colours::text.withAlpha (0.55f * alpha));
                        g.fillRect (juce::Rectangle<float>::leftTopRightBottom (r.getX() + 4.0f, std::min (mid, y2),
                                                                                r.getRight() - 4.0f, std::max (mid, y2)));
                        g.setColour (colours::lampAmber.withAlpha (alpha));
                        g.setFont (panelFont (11.0f));
                        g.drawText ((p > 0 ? "+" : "") + juce::String (p), r, juce::Justification::centred);
                        g.setFont (panelFont (12.0f).withExtraKerningFactor (0.08f));
                    }
                    break;
                }
                case rowProb:
                case rowGate:
                {
                    const float v = row == rowProb ? pat.prob[idx].load() : pat.gate[idx].load();
                    const auto col = row == rowProb ? colours::text : colours::lampAmber;
                    g.setColour (col.withAlpha (0.5f * alpha));
                    g.fillRect (r.reduced (3.0f, 2.0f).withTrimmedTop ((r.getHeight() - 4.0f) * (1.0f - v)));
                    break;
                }
                default:
                    break;
            }

            if (i == playIdx)
            {
                g.setColour (colours::text.withAlpha (0.9f));
                g.drawRect (r.expanded (0.5f), 1.4f);
            }
        }
    }
}

void StepGrid::applyAt (const juce::MouseEvent& e, bool isDown)
{
    auto& pat = processor.getPattern();
    const auto pos = e.position;
    const int step = stepAt (pos.x);
    if (step < 0)
        return;
    const auto idx = (size_t) step;

    if (isDown)
    {
        dragRow = rowAt (pos.y);
        dragStep = step;
        startPitch = pat.pitch[idx].load();
        if (dragRow == rowOn) paintValue = ! pat.on[idx].load();
        if (dragRow == rowAccent) paintValue = ! pat.accent[idx].load();
        if (dragRow == rowTie) paintValue = ! pat.tie[idx].load();
    }

    auto valueFromY = [&] (float min)
    {
        const auto r = cell (dragRow, 0);
        return juce::jlimit (min, 1.0f, 1.0f - (pos.y - r.getY()) / r.getHeight());
    };

    switch (dragRow)
    {
        case rowOn:     pat.on[idx] = paintValue; break;
        case rowAccent: pat.accent[idx] = paintValue; break;
        case rowTie:    pat.tie[idx] = paintValue; break;
        case rowPitch:
        {
            const int dy = juce::roundToInt ((float) e.getDistanceFromDragStartY() / 5.0f);
            pat.pitch[(size_t) dragStep] = juce::jlimit (-24, 24, startPitch - dy);
            break;
        }
        case rowProb:   pat.prob[idx] = valueFromY (0.0f); break;
        case rowGate:   pat.gate[idx] = valueFromY (0.05f); break;
        default: break;
    }

    markDirty (processor);
    repaint();
}

void StepGrid::mouseDown (const juce::MouseEvent& e) { applyAt (e, true); }
void StepGrid::mouseDrag (const juce::MouseEvent& e) { applyAt (e, false); }

void StepGrid::mouseDoubleClick (const juce::MouseEvent& e)
{
    const int step = stepAt (e.position.x);
    if (step < 0)
        return;
    auto& pat = processor.getPattern();
    const auto idx = (size_t) step;
    switch (rowAt (e.position.y))
    {
        case rowPitch: pat.pitch[idx] = 0; break;
        case rowProb:  pat.prob[idx] = 1.0f; break;
        case rowGate:  pat.gate[idx] = 0.5f; break;
        default: return;
    }
    markDirty (processor);
    repaint();
}

void StepGrid::updatePlayhead()
{
    const auto s = processor.getCurrentStep();
    if (s != lastStep)
    {
        lastStep = s;
        repaint();
    }
}

//==============================================================================
DragBox::DragBox (StesoLeadProcessor& p) : processor (p)
{
    setMouseCursor (juce::MouseCursor::DraggingHandCursor);
    setTooltip (juce::String::fromUTF8 ("Dra till ett MIDI-spår i din DAW. Stegsekvenseraren stängs sedan av så att det inte låter dubbelt. (Drag to a MIDI track in your DAW. The sequencer is then switched off to avoid doubled notes.)"));
}

void DragBox::paint (juce::Graphics& g)
{
    auto r = getLocalBounds().toFloat().reduced (2.0f);
    // Präglad plåtskylt.
    g.setColour (dragging ? colours::lampAmber.withAlpha (0.18f) : juce::Colours::black.withAlpha (0.22f));
    g.fillRect (r);
    drawEngravedBox (g, r);

    juce::Path border;
    border.addRectangle (r.reduced (5.0f));
    const float dashes[] { 7.0f, 4.0f };
    juce::Path dashed;
    juce::PathStrokeType (1.3f).createDashedStroke (dashed, border, dashes, 2);
    g.setColour (colours::text.withAlpha (0.45f));
    g.fillPath (dashed);

    const auto range = getRange ? getRange() : Range {};
    auto text = r.reduced (12.0f);
    g.setColour (colours::text);
    g.setFont (panelFont (18.0f).withExtraKerningFactor (0.1f));
    g.drawText (juce::String::fromUTF8 ("⇢  DRA MIDI TILL DAW"), text.removeFromTop (text.getHeight() * 0.42f),
                juce::Justification::centredBottom);
    g.setColour (colours::dimText);
    g.setFont (panelFont (12.0f).withExtraKerningFactor (0.1f));
    g.drawText ("(DRAG MIDI TO DAW)", text.removeFromTop (18.0f), juce::Justification::centred);
    g.setColour (colours::dimText);
    g.setFont (panelFont (13.0f, false));
    g.drawFittedText (range.label, text.toNearestInt().withTrimmedTop (4), juce::Justification::centredTop, 2);
}

void DragBox::mouseDrag (const juce::MouseEvent& e)
{
    if (dragging || e.getDistanceFromDragStart() < 6 || ! getRange)
        return;

    dragging = true;
    repaint();

    const auto range = getRange();
    const auto file = processor.writeMidiExport (range.from, range.to, range.fileName);
    if (! file.existsAsFile())
    {
        dragging = false;
        return;
    }

    auto* proc = &processor;
    juce::Component::SafePointer<DragBox> safe (this);
    juce::DragAndDropContainer::performExternalDragDropOfFiles ({ file.getFullPathName() }, false, this, [proc, safe]
    {
        proc->sequencerExported();
        if (safe != nullptr)
        {
            safe->dragging = false;
            safe->repaint();
        }
    });
}

void DragBox::mouseUp (const juce::MouseEvent&)
{
    dragging = false;
    repaint();
}

//==============================================================================
SeqPanel::SeqPanel (StesoLeadProcessor& p) : processor (p), grid (p), poly (p), dragBox (p)
{
    auto add = [this] (const char* id, const char* label, bool isChoice)
    {
        controls.push_back (std::make_unique<Control> (processor.apvts, id, label, colours::accent2, isChoice));
        addAndMakeVisible (*controls.back());
    };

    add (pid::seqMode, "Mode", true);
    add (pid::seqRate, "Rate", true);
    add (pid::seqSwing, "Swing", false);
    add (pid::seqGate, "Gate", false);
    add (pid::arpOrder, "Arp", true);
    add (pid::arpOctaves, "Octaves", false);
    add (pid::seqLength, "Steps", false);
    add (pid::seqAccLen, "Acc steps", false);
    add (pid::seqGateLen, "Gate steps", false);
    add (pid::seqLatch, "Latch", true);
    add (pid::seqMidiOut, "MIDI out", true);
    add (pid::polyOn, "Polyrhythm", true);
    add (pid::polyBars, "Cycle bars", false);

    addAndMakeVisible (grid);
    addChildComponent (poly);

    // Snabbval för Polyrytm (ersätter Euclid-raden när läget är på).
    struct Ratio { const char* text; juce::String tip; std::function<void()> apply; };
    const std::vector<Ratio> ratios {
        { "4:3", "4 mot 3 (4 against 3)", [this] { poly.applySetup ({ { true, 0, 4, 16, 0 }, { true, 0, 3, 16, 7 } }); } },
        { "3:2", "3 mot 2 (3 against 2)", [this] { poly.applySetup ({ { true, 0, 3, 16, 0 }, { true, 0, 2, 16, 12 } }); } },
        { "5:4", "5 mot 4 (5 against 4)", [this] { poly.applySetup ({ { true, 0, 4, 16, 0 }, { true, 0, 5, 16, 7 } }); } },
        { "7:4", "7 mot 4 (7 against 4)", [this] { poly.applySetup ({ { true, 0, 4, 16, -12 }, { true, 0, 7, 16, 0 } }); } },
        { "Div", juce::String::fromUTF8 ("Klockdelare ÷2 ÷3 ÷5 ÷7 av Rate (clock dividers)"),
          [this] { poly.applySetup ({ { true, 1, 2, 16, 0 }, { true, 1, 3, 16, 7 }, { true, 1, 5, 16, -12 }, { true, 1, 7, 16, 12 } }); } },
        { "Init", juce::String::fromUTF8 ("Återställ alla spår (reset lanes)"), [this] { poly.resetAll(); } },
    };
    for (const auto& r : ratios)
    {
        auto b = std::make_unique<juce::TextButton> (r.text);
        b->setTooltip (r.tip);
        b->onClick = r.apply;
        addChildComponent (*b);
        ratioButtons.push_back (std::move (b));
    }

    dragBox.getRange = [this] { return computeRange(); };
    addAndMakeVisible (dragBox);

    rangeMode.addItem (juce::String::fromUTF8 ("Längd: DAW:ns markörer (length: DAW locators)"), 1);
    rangeMode.addItem (juce::String::fromUTF8 ("Längd: från/till takt (length: from/to bar)"), 2);
    addAndMakeVisible (rangeMode);

    for (auto* s : { &fromBar, &toBar })
    {
        s->setRange (1.0, 999.0, 1.0);
        s->setTextBoxStyle (juce::Slider::TextBoxLeft, false, 44, 22);
        s->setIncDecButtonsMode (juce::Slider::incDecButtonsDraggable_Vertical);
        s->onValueChange = [this] { dragBox.repaint(); };
        addAndMakeVisible (*s);
    }
    rangeMode.onChange = [this] { dragBox.repaint(); };

    auto setupLabel = [this] (juce::Label& l, const juce::String& text)
    {
        l.setText (text, juce::dontSendNotification);
        l.setFont (panelFont (13.0f));
        l.setColour (juce::Label::textColourId, colours::text.withAlpha (0.85f));
        l.setJustificationType (juce::Justification::centredRight);
        addAndMakeVisible (l);
    };
    setupLabel (fromLabel, juce::String::fromUTF8 ("Från (From)"));
    setupLabel (toLabel, "Till (To)");
    setupLabel (hitsLabel, "Hits");
    setupLabel (rotateLabel, "Rot");

    bypassLabel.setText (juce::String::fromUTF8 ("Stegsekvenserare av – i DAW:en (sequencer off)"), juce::dontSendNotification);
    bypassLabel.setFont (panelFont (14.0f));
    bypassLabel.setColour (juce::Label::textColourId, colours::lampRed);
    addChildComponent (bypassLabel);
    reenableButton.onClick = [this] { processor.reenableSequencer(); };
    addChildComponent (reenableButton);

    euclidHits.setRange (0.0, 32.0, 1.0);
    euclidHits.setValue (5.0, juce::dontSendNotification);
    euclidRotate.setRange (0.0, 31.0, 1.0);
    for (auto* s : { &euclidHits, &euclidRotate })
    {
        s->setTextBoxStyle (juce::Slider::TextBoxLeft, false, 30, 22);
        s->setIncDecButtonsMode (juce::Slider::incDecButtonsDraggable_Vertical);
        addAndMakeVisible (*s);
    }

    euclidButton.setTooltip (juce::String::fromUTF8 ("Sprider Hits jämnt över Steps (spreads hits evenly – Euclidean rhythm)"));
    euclidButton.onClick = [this]
    {
        const auto s = processor.currentSeqSettings();
        processor.getPattern().applyEuclid ((int) euclidHits.getValue(), s.length, (int) euclidRotate.getValue());
        markDirty (processor);
        grid.repaint();
    };
    initButton.setTooltip (juce::String::fromUTF8 ("Återställ mönstret (reset pattern)"));
    initButton.onClick = [this]
    {
        processor.getPattern().resetSteps();
        markDirty (processor);
        grid.repaint();
    };
    addAndMakeVisible (euclidButton);
    addAndMakeVisible (initButton);

    bindState();
}

void SeqPanel::bindState()
{
    auto& st = processor.apvts.state;
    if (! st.hasProperty ("exportRange")) st.setProperty ("exportRange", 1, nullptr);
    if (! st.hasProperty ("exportFrom"))  st.setProperty ("exportFrom", 1, nullptr);
    if (! st.hasProperty ("exportTo"))    st.setProperty ("exportTo", 17, nullptr);

    updatePolyVisibility();
    poly.repaint();
    rangeMode.getSelectedIdAsValue().referTo (st.getPropertyAsValue ("exportRange", nullptr));
    fromBar.getValueObject().referTo (st.getPropertyAsValue ("exportFrom", nullptr));
    toBar.getValueObject().referTo (st.getPropertyAsValue ("exportTo", nullptr));
    grid.repaint();
    dragBox.repaint();
}

DragBox::Range SeqPanel::computeRange() const
{
    const auto host = processor.getHostInfo();
    const double barPpq = host.timeSigNum * 4.0 / host.timeSigDen;
    DragBox::Range r;
    juce::String prefix;

    if (rangeMode.getSelectedId() != 2 && host.hasLoop)
    {
        r.from = host.loopStart;
        r.to = host.loopEnd;
        r.fromLocators = true;
        prefix = juce::String::fromUTF8 ("Markörer (locators): ");
    }
    else
    {
        const double a = fromBar.getValue();
        const double b = std::max (a + 1.0, toBar.getValue());
        r.from = (a - 1.0) * barPpq;
        r.to = (b - 1.0) * barPpq;
        if (rangeMode.getSelectedId() != 2)
            prefix = juce::String::fromUTF8 ("Inga markörer från DAW:n (no DAW locators) – Från/Till används. ");
    }

    const auto a = formatBar (r.from, barPpq), b = formatBar (r.to, barPpq);
    const double bars = (r.to - r.from) / barPpq;
    const auto barsText = std::abs (bars - std::round (bars)) < 0.01 ? juce::String (juce::roundToInt (bars)) : juce::String (bars, 2);
    r.label = prefix + juce::String::fromUTF8 ("Takt ") + a + juce::String::fromUTF8 ("–") + b + "  ·  " + barsText + " takter (bars)";
    r.fileName = "Bakelit Takt (Bars) " + a + "-" + b + ".mid";
    return r;
}

void SeqPanel::paint (juce::Graphics& g)
{
    drawSectionFrame (g, getLocalBounds().toFloat().reduced (3.0f),
                      polyVisible ? juce::String::fromUTF8 ("POLYRYTM|Polyrhythm")
                                  : juce::String::fromUTF8 ("STEGSEKVENSERARE|Step sequencer / arp"));
}

void SeqPanel::resized()
{
    auto r = getLocalBounds().reduced (10, 6);
    r.removeFromTop (22);

    auto right = r.removeFromRight (340);
    r.removeFromRight (10);

    auto controlRow = r.removeFromTop (66);
    const int w = controlRow.getWidth() / (int) controls.size();
    for (auto& c : controls)
        c->setBounds (controlRow.removeFromLeft (w));
    r.removeFromTop (4);
    grid.setBounds (r);
    poly.setBounds (r);

    rangeMode.setBounds (right.removeFromTop (26));
    right.removeFromTop (4);
    auto barsRow = right.removeFromTop (26);
    fromLabel.setBounds (barsRow.removeFromLeft (70));
    fromBar.setBounds (barsRow.removeFromLeft (100).reduced (2, 0));
    toLabel.setBounds (barsRow.removeFromLeft (64));
    toBar.setBounds (barsRow.removeFromLeft (100).reduced (2, 0));

    auto euclidRow = right.removeFromBottom (26);
    {
        auto rr = euclidRow;
        const int bw = (rr.getWidth() - 4 * ((int) ratioButtons.size() - 1)) / (int) ratioButtons.size();
        for (auto& b : ratioButtons)
        {
            b->setBounds (rr.removeFromLeft (bw));
            rr.removeFromLeft (4);
        }
    }
    hitsLabel.setBounds (euclidRow.removeFromLeft (36));
    euclidHits.setBounds (euclidRow.removeFromLeft (80).reduced (2, 0));
    rotateLabel.setBounds (euclidRow.removeFromLeft (36));
    euclidRotate.setBounds (euclidRow.removeFromLeft (80).reduced (2, 0));
    euclidRow.removeFromLeft (4);
    euclidButton.setBounds (euclidRow.removeFromLeft (56));
    euclidRow.removeFromLeft (4);
    initButton.setBounds (euclidRow);
    right.removeFromBottom (4);

    auto bypassRow = right.removeFromBottom (26);
    reenableButton.setBounds (bypassRow.removeFromRight (130));
    bypassLabel.setBounds (bypassRow);
    right.removeFromBottom (4);
    right.removeFromTop (6);
    dragBox.setBounds (right);
}

void SeqPanel::updatePolyVisibility()
{
    const bool on = processor.apvts.getRawParameterValue (pid::polyOn)->load() > 0.5f;
    if (on == polyVisible && (poly.isVisible() == on))
        return;
    polyVisible = on;
    grid.setVisible (! on);
    poly.setVisible (on);
    for (juce::Component* c : { (juce::Component*) &euclidHits, (juce::Component*) &euclidRotate, (juce::Component*) &hitsLabel,
                                (juce::Component*) &rotateLabel, (juce::Component*) &euclidButton, (juce::Component*) &initButton })
        c->setVisible (! on);
    for (auto& b : ratioButtons)
        b->setVisible (on);
    repaint();
}

void SeqPanel::refresh()
{
    updatePolyVisibility();
    if (polyVisible)
        poly.refresh();
    else
        grid.updatePlayhead();

    const auto s = processor.currentSeqSettings();
    const int laneHash = s.length * 10000 + s.accentLength * 100 + s.gateLength;
    if (laneHash != lastLaneHash)
    {
        lastLaneHash = laneHash;
        grid.repaint();
    }

    const auto range = computeRange();
    if (range.label != lastInfo)
    {
        lastInfo = range.label;
        dragBox.repaint();
    }

    const bool showBypass = processor.isBypassedByExport()
                            && processor.currentSeqSettings().mode == seq::Mode::off;
    if (showBypass != lastBypassVisible)
    {
        lastBypassVisible = showBypass;
        bypassLabel.setVisible (showBypass);
        reenableButton.setVisible (showBypass);
    }
}
} // namespace steso::ui
