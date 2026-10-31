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
    const auto accentCol = colours::accent;
    const auto mainCol = colours::accent2;

    g.setFont (juce::Font (juce::FontOptions (11.5f)));

    for (int row = 0; row < numRows; ++row)
    {
        const auto first = cell (row, 0);
        g.setColour (colours::dimText);
        g.drawText (rowNames[row], juce::Rectangle<float> (0.0f, first.getY(), (float) labelWidth - 6.0f, first.getHeight()),
                    juce::Justification::centredRight);

        const int len = laneLength (row);
        const int playIdx = step >= 0 ? (int) (step % len) : -1;

        for (int i = 0; i < seq::maxSteps; ++i)
        {
            const auto idx = (size_t) i;
            const auto r = cell (row, i).reduced (1.5f);
            const bool inLane = i < len;
            const float alpha = inLane ? 1.0f : 0.25f;

            g.setColour (colours::knobBody.withAlpha (inLane ? 1.0f : 0.45f).brighter ((i / 4) % 2 == 0 ? 0.0f : 0.08f));
            g.fillRoundedRectangle (r, 3.0f);

            switch (row)
            {
                case rowOn:
                    if (pat.on[idx].load())
                    {
                        g.setColour (mainCol.withAlpha (alpha));
                        g.fillRoundedRectangle (r.reduced (2.0f), 2.5f);
                    }
                    break;
                case rowAccent:
                    if (pat.accent[idx].load())
                    {
                        g.setColour (accentCol.withAlpha (alpha));
                        g.fillRoundedRectangle (r.reduced (3.0f), 2.5f);
                    }
                    break;
                case rowTie:
                    if (pat.tie[idx].load())
                    {
                        g.setColour (mainCol.withAlpha (alpha));
                        g.fillRoundedRectangle (r.withHeight (r.getHeight() * 0.35f).withCentre (r.getCentre()), 2.0f);
                    }
                    break;
                case rowPitch:
                {
                    const int p = pat.pitch[idx].load();
                    const float mid = r.getCentreY();
                    const float hh = r.getHeight() * 0.5f - 1.0f;
                    const float y2 = mid - hh * (float) p / 24.0f;
                    g.setColour (colours::panelEdge);
                    g.drawHorizontalLine ((int) mid, r.getX() + 2.0f, r.getRight() - 2.0f);
                    if (p != 0)
                    {
                        g.setColour (mainCol.withAlpha (0.8f * alpha));
                        g.fillRect (juce::Rectangle<float>::leftTopRightBottom (r.getX() + 3.0f, std::min (mid, y2),
                                                                                r.getRight() - 3.0f, std::max (mid, y2)));
                        g.setColour (colours::text.withAlpha (alpha));
                        g.setFont (juce::Font (juce::FontOptions (10.0f)));
                        g.drawText ((p > 0 ? "+" : "") + juce::String (p), r, juce::Justification::centred);
                        g.setFont (juce::Font (juce::FontOptions (11.5f)));
                    }
                    break;
                }
                case rowProb:
                case rowGate:
                {
                    const float v = row == rowProb ? pat.prob[idx].load() : pat.gate[idx].load();
                    g.setColour ((row == rowProb ? mainCol : accentCol).withAlpha (0.75f * alpha));
                    g.fillRoundedRectangle (r.withTrimmedTop (r.getHeight() * (1.0f - v)).reduced (2.0f, 0.0f), 2.0f);
                    break;
                }
                default:
                    break;
            }

            if (i == playIdx)
            {
                g.setColour (colours::text.withAlpha (0.9f));
                g.drawRoundedRectangle (r, 3.0f, 1.5f);
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
    setTooltip (juce::String::fromUTF8 ("Dra till ett MIDI-spår i din DAW. Sekvensern stängs sedan av så att det inte låter dubbelt."));
}

void DragBox::paint (juce::Graphics& g)
{
    auto r = getLocalBounds().toFloat().reduced (2.0f);
    g.setColour (dragging ? colours::accent.withAlpha (0.25f) : colours::knobBody);
    g.fillRoundedRectangle (r, 8.0f);

    juce::Path border;
    border.addRoundedRectangle (r, 8.0f);
    const float dashes[] { 6.0f, 4.0f };
    juce::Path dashed;
    juce::PathStrokeType (1.5f).createDashedStroke (dashed, border, dashes, 2);
    g.setColour (colours::accent);
    g.fillPath (dashed);

    const auto range = getRange ? getRange() : Range {};
    auto text = r.reduced (8.0f);
    g.setColour (colours::text);
    g.setFont (juce::Font (juce::FontOptions (16.0f, juce::Font::bold)));
    g.drawText (juce::String::fromUTF8 ("⇢  Dra MIDI till DAW"), text.removeFromTop (text.getHeight() * 0.55f),
                juce::Justification::centredBottom);
    g.setColour (colours::dimText);
    g.setFont (juce::Font (juce::FontOptions (12.5f)));
    g.drawFittedText (range.label, text.toNearestInt(), juce::Justification::centredTop, 2);
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
SeqPanel::SeqPanel (StesoLeadProcessor& p) : processor (p), grid (p), dragBox (p)
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

    addAndMakeVisible (grid);

    dragBox.getRange = [this] { return computeRange(); };
    addAndMakeVisible (dragBox);

    rangeMode.addItem (juce::String::fromUTF8 ("Längd: DAW:ns locators"), 1);
    rangeMode.addItem (juce::String::fromUTF8 ("Längd: från/till takt"), 2);
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
        l.setFont (juce::Font (juce::FontOptions (12.5f)));
        l.setColour (juce::Label::textColourId, colours::dimText);
        l.setJustificationType (juce::Justification::centredRight);
        addAndMakeVisible (l);
    };
    setupLabel (fromLabel, juce::String::fromUTF8 ("Från"));
    setupLabel (toLabel, "Till");
    setupLabel (hitsLabel, "Hits");
    setupLabel (rotateLabel, "Rot");

    bypassLabel.setText (juce::String::fromUTF8 ("Sekvensern är av – mönstret ligger i DAW:en"), juce::dontSendNotification);
    bypassLabel.setFont (juce::Font (juce::FontOptions (12.5f, juce::Font::bold)));
    bypassLabel.setColour (juce::Label::textColourId, colours::accent);
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

    euclidButton.setTooltip (juce::String::fromUTF8 ("Sprider Hits jämnt över Steps (euklidisk rytm)"));
    euclidButton.onClick = [this]
    {
        const auto s = processor.currentSeqSettings();
        processor.getPattern().applyEuclid ((int) euclidHits.getValue(), s.length, (int) euclidRotate.getValue());
        markDirty (processor);
        grid.repaint();
    };
    initButton.setTooltip (juce::String::fromUTF8 ("Återställ mönstret"));
    initButton.onClick = [this]
    {
        processor.getPattern().setDefault();
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
        prefix = "Locators: ";
    }
    else
    {
        const double a = fromBar.getValue();
        const double b = std::max (a + 1.0, toBar.getValue());
        r.from = (a - 1.0) * barPpq;
        r.to = (b - 1.0) * barPpq;
        if (rangeMode.getSelectedId() != 2)
            prefix = juce::String::fromUTF8 ("DAW:n skickar inga locators – använder Från/Till. ");
    }

    const auto a = formatBar (r.from, barPpq), b = formatBar (r.to, barPpq);
    const double bars = (r.to - r.from) / barPpq;
    const auto barsText = std::abs (bars - std::round (bars)) < 0.01 ? juce::String (juce::roundToInt (bars)) : juce::String (bars, 2);
    r.label = prefix + juce::String::fromUTF8 ("Takt ") + a + juce::String::fromUTF8 ("–") + b + "  ·  " + barsText + " takter";
    r.fileName = "StesoLead Takt " + a + "-" + b + ".mid";
    return r;
}

void SeqPanel::paint (juce::Graphics& g)
{
    auto r = getLocalBounds().toFloat().reduced (3.0f);
    g.setColour (colours::panel);
    g.fillRoundedRectangle (r, 8.0f);
    g.setColour (colours::panelEdge);
    g.drawRoundedRectangle (r, 8.0f, 1.0f);

    auto header = r.removeFromTop (24.0f).reduced (10.0f, 0.0f);
    g.setColour (colours::accent2);
    g.fillRoundedRectangle (header.getX(), header.getCentreY() - 1.0f, 4.0f, 4.0f, 2.0f);
    g.setColour (colours::text.withAlpha (0.9f));
    g.setFont (juce::Font (juce::FontOptions (13.0f, juce::Font::bold)).withExtraKerningFactor (0.12f));
    g.drawText ("SEQUENCER / ARP", header.withTrimmedLeft (10.0f), juce::Justification::centredLeft);
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

    rangeMode.setBounds (right.removeFromTop (26));
    right.removeFromTop (4);
    auto barsRow = right.removeFromTop (26);
    fromLabel.setBounds (barsRow.removeFromLeft (40));
    fromBar.setBounds (barsRow.removeFromLeft (120).reduced (2, 0));
    toLabel.setBounds (barsRow.removeFromLeft (40));
    toBar.setBounds (barsRow.removeFromLeft (120).reduced (2, 0));

    auto euclidRow = right.removeFromBottom (26);
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
    reenableButton.setBounds (bypassRow.removeFromRight (96));
    bypassLabel.setBounds (bypassRow);
    right.removeFromBottom (4);
    right.removeFromTop (6);
    dragBox.setBounds (right);
}

void SeqPanel::refresh()
{
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
