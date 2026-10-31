#include "PluginEditor.h"

namespace steso::ui
{
//==============================================================================
LookAndFeel::LookAndFeel()
{
    setColour (juce::ComboBox::backgroundColourId, colours::knobBody);
    setColour (juce::ComboBox::outlineColourId, colours::panelEdge);
    setColour (juce::ComboBox::textColourId, colours::text);
    setColour (juce::ComboBox::arrowColourId, colours::dimText);
    setColour (juce::PopupMenu::backgroundColourId, colours::panel);
    setColour (juce::PopupMenu::textColourId, colours::text);
    setColour (juce::PopupMenu::highlightedBackgroundColourId, colours::accent.withAlpha (0.85f));
    setColour (juce::PopupMenu::highlightedTextColourId, colours::background);
    setColour (juce::PopupMenu::headerTextColourId, colours::accent2);
    setColour (juce::Label::textColourId, colours::text);
    setColour (juce::TextButton::buttonColourId, colours::knobBody);
    setColour (juce::TextButton::textColourOffId, colours::text);
    setColour (juce::BubbleComponent::backgroundColourId, colours::background.withAlpha (0.95f));
    setColour (juce::BubbleComponent::outlineColourId, colours::accent);
    setColour (juce::TooltipWindow::textColourId, colours::text);
}

void LookAndFeel::drawRotarySlider (juce::Graphics& g, int x, int y, int width, int height, float pos,
                                    float startAngle, float endAngle, juce::Slider& slider)
{
    const auto bounds = juce::Rectangle<float> ((float) x, (float) y, (float) width, (float) height).reduced (4.0f);
    const float radius = std::min (bounds.getWidth(), bounds.getHeight()) * 0.5f;
    const auto centre = bounds.getCentre();
    const float trackWidth = std::max (2.5f, radius * 0.13f);
    const float arcRadius = radius - trackWidth * 0.5f;
    const float angle = startAngle + pos * (endAngle - startAngle);
    const auto colour = slider.findColour (juce::Slider::rotarySliderFillColourId);

    // Spår
    juce::Path track;
    track.addCentredArc (centre.x, centre.y, arcRadius, arcRadius, 0.0f, startAngle, endAngle, true);
    g.setColour (colours::background);
    g.strokePath (track, juce::PathStrokeType (trackWidth, juce::PathStrokeType::curved, juce::PathStrokeType::rounded));

    // Värdebåge (bipolära parametrar ritas från mitten)
    const bool bipolar = slider.getMinimum() < 0.0 && juce::approximatelyEqual (slider.getMinimum(), -slider.getMaximum());
    const float from = bipolar ? (startAngle + endAngle) * 0.5f : startAngle;
    juce::Path value;
    value.addCentredArc (centre.x, centre.y, arcRadius, arcRadius, 0.0f, std::min (from, angle), std::max (from, angle), true);
    g.setColour (colour);
    g.strokePath (value, juce::PathStrokeType (trackWidth, juce::PathStrokeType::curved, juce::PathStrokeType::rounded));

    // Rattkropp
    const float bodyRadius = arcRadius - trackWidth * 1.3f;
    g.setGradientFill (juce::ColourGradient (colours::knobBody.brighter (0.25f), centre.x, centre.y - bodyRadius,
                                             colours::knobBody.darker (0.4f), centre.x, centre.y + bodyRadius, false));
    g.fillEllipse (centre.x - bodyRadius, centre.y - bodyRadius, bodyRadius * 2.0f, bodyRadius * 2.0f);
    g.setColour (colours::panelEdge.brighter (0.2f));
    g.drawEllipse (centre.x - bodyRadius, centre.y - bodyRadius, bodyRadius * 2.0f, bodyRadius * 2.0f, 1.0f);

    // Visare
    juce::Path pointer;
    const float pointerLength = bodyRadius * 0.75f;
    pointer.addRoundedRectangle (-1.5f, -bodyRadius + 2.0f, 3.0f, pointerLength, 1.5f);
    g.setColour (colours::text);
    g.fillPath (pointer, juce::AffineTransform::rotation (angle).translated (centre.x, centre.y));
}

void LookAndFeel::drawComboBox (juce::Graphics& g, int width, int height, bool, int, int, int, int, juce::ComboBox& box)
{
    const auto r = juce::Rectangle<float> (0.0f, 0.0f, (float) width, (float) height).reduced (0.5f);
    g.setColour (box.findColour (juce::ComboBox::backgroundColourId));
    g.fillRoundedRectangle (r, 5.0f);
    g.setColour (box.hasKeyboardFocus (false) ? colours::accent : box.findColour (juce::ComboBox::outlineColourId));
    g.drawRoundedRectangle (r, 5.0f, 1.0f);

    juce::Path arrow;
    const float ax = (float) width - 14.0f, ay = (float) height * 0.5f;
    arrow.addTriangle (ax - 4.0f, ay - 2.0f, ax + 4.0f, ay - 2.0f, ax, ay + 3.0f);
    g.setColour (colours::dimText);
    g.fillPath (arrow);
}

juce::Font LookAndFeel::getComboBoxFont (juce::ComboBox&)
{
    return juce::Font (juce::FontOptions (14.0f));
}

juce::Font LookAndFeel::getPopupMenuFont()
{
    return juce::Font (juce::FontOptions (15.0f));
}

void LookAndFeel::drawButtonBackground (juce::Graphics& g, juce::Button& b, const juce::Colour&, bool highlighted, bool down)
{
    auto r = b.getLocalBounds().toFloat().reduced (0.5f);
    g.setColour (down ? colours::accent : (highlighted ? colours::knobBody.brighter (0.2f) : colours::knobBody));
    g.fillRoundedRectangle (r, 5.0f);
    g.setColour (colours::panelEdge);
    g.drawRoundedRectangle (r, 5.0f, 1.0f);
}

//==============================================================================
Control::Control (juce::AudioProcessorValueTreeState& apvts, const juce::String& paramId, const juce::String& text,
                  juce::Colour colour, bool isChoice)
{
    label.setText (text, juce::dontSendNotification);
    label.setJustificationType (juce::Justification::centred);
    label.setFont (juce::Font (juce::FontOptions (12.5f)));
    label.setColour (juce::Label::textColourId, colours::dimText);
    label.setInterceptsMouseClicks (false, false);
    addAndMakeVisible (label);

    auto* param = apvts.getParameter (paramId);
    jassert (param != nullptr);

    if (isChoice)
    {
        combo = std::make_unique<juce::ComboBox>();
        if (auto* choice = dynamic_cast<juce::AudioParameterChoice*> (param))
            combo->addItemList (choice->choices, 1);
        else if (dynamic_cast<juce::AudioParameterBool*> (param) != nullptr)
            combo->addItemList ({ "Off", "On" }, 1);
        comboAttachment = std::make_unique<juce::AudioProcessorValueTreeState::ComboBoxAttachment> (apvts, paramId, *combo);
        combo->setTooltip (param->getName (64));
        addAndMakeVisible (*combo);
    }
    else
    {
        slider = std::make_unique<juce::Slider> (juce::Slider::RotaryHorizontalVerticalDrag, juce::Slider::NoTextBox);
        slider->setColour (juce::Slider::rotarySliderFillColourId, colour);
        slider->setPopupDisplayEnabled (true, true, nullptr, 1500);
        slider->setMouseDragSensitivity (220);
        sliderAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment> (apvts, paramId, *slider);
        if (auto* ranged = dynamic_cast<juce::RangedAudioParameter*> (param))
            slider->setDoubleClickReturnValue (true, ranged->convertFrom0to1 (ranged->getDefaultValue()));
        slider->setTooltip (param->getName (64) + " (dubbelklick = standard)");
        addAndMakeVisible (*slider);
    }
}

void Control::resized()
{
    auto r = getLocalBounds();
    auto labelArea = r.removeFromBottom (18);
    label.setBounds (labelArea);

    if (slider != nullptr)
    {
        const int size = std::min (r.getWidth(), r.getHeight());
        slider->setBounds (r.withSizeKeepingCentre (size, size));
    }
    else if (combo != nullptr)
    {
        combo->setBounds (r.withSizeKeepingCentre (std::min (r.getWidth() - 4, 96), 26));
    }
}

//==============================================================================
Section::Section (juce::String t, juce::Colour c) : title (std::move (t)), colour (c) {}

void Section::add (juce::AudioProcessorValueTreeState& apvts, const juce::String& paramId, const juce::String& label,
                   bool isChoice)
{
    controls.push_back (std::make_unique<Control> (apvts, paramId, label, colour, isChoice));
    addAndMakeVisible (*controls.back());
}

void Section::paint (juce::Graphics& g)
{
    auto r = getLocalBounds().toFloat().reduced (3.0f);
    g.setColour (colours::panel);
    g.fillRoundedRectangle (r, 8.0f);
    g.setColour (colours::panelEdge);
    g.drawRoundedRectangle (r, 8.0f, 1.0f);

    auto header = r.removeFromTop (24.0f).reduced (10.0f, 0.0f);
    g.setColour (colour);
    g.fillRoundedRectangle (header.getX(), header.getCentreY() - 1.0f, 4.0f, 4.0f, 2.0f);
    g.setColour (colours::text.withAlpha (0.9f));
    g.setFont (juce::Font (juce::FontOptions (13.0f, juce::Font::bold)).withExtraKerningFactor (0.12f));
    g.drawText (title, header.withTrimmedLeft (10.0f), juce::Justification::centredLeft);
}

void Section::resized()
{
    auto r = getLocalBounds().reduced (8, 6);
    r.removeFromTop (22);
    if (controls.empty())
        return;

    const int w = r.getWidth() / (int) controls.size();
    for (auto& c : controls)
        c->setBounds (r.removeFromLeft (w));
}
} // namespace steso::ui

//==============================================================================
StesoLeadEditor::StesoLeadEditor (StesoLeadProcessor& p)
    : AudioProcessorEditor (&p),
      processor (p),
      keyboard (p.keyboardState, juce::MidiKeyboardComponent::horizontalKeyboard),
      seqPanel (p)
{
    using namespace steso;
    setLookAndFeel (&lnf);
    auto& s = p.apvts;

    osc1.add (s, pid::osc1Wave, "Wave", true);
    osc1.add (s, pid::osc1Octave, "Octave");
    osc1.add (s, pid::osc1PW, "PW");

    osc2.add (s, pid::osc2Wave, "Wave", true);
    osc2.add (s, pid::osc2Semi, "Semi");
    osc2.add (s, pid::osc2Fine, "Fine");
    osc2.add (s, pid::osc2PW, "PW");

    osc3.add (s, pid::osc3Wave, "Wave", true);
    osc3.add (s, pid::osc3Semi, "Semi");
    osc3.add (s, pid::osc3Fine, "Fine");

    mixer.add (s, pid::mixOsc1, "Osc 1");
    mixer.add (s, pid::mixOsc2, "Osc 2");
    mixer.add (s, pid::mixOsc3, "Osc 3");
    mixer.add (s, pid::mixSub, "Sub");
    mixer.add (s, pid::mixNoise, "Noise");

    filter.add (s, pid::filterType, "Type", true);
    filter.add (s, pid::cutoff, "Cutoff");
    filter.add (s, pid::resonance, "Reso");
    filter.add (s, pid::drive, "Drive");
    filter.add (s, pid::filterEnv, "Env");
    filter.add (s, pid::keyTrack, "Key");
    filter.add (s, pid::filterVel, "Velocity");
    filter.add (s, pid::feedback, "Feedback");
    filter.add (s, pid::warmth, "Warmth");

    filterEnv.add (s, pid::fAttack, "Attack");
    filterEnv.add (s, pid::fDecay, "Decay");
    filterEnv.add (s, pid::fSustain, "Sustain");
    filterEnv.add (s, pid::fRelease, "Release");

    ampEnv.add (s, pid::aAttack, "Attack");
    ampEnv.add (s, pid::aDecay, "Decay");
    ampEnv.add (s, pid::aSustain, "Sustain");
    ampEnv.add (s, pid::aRelease, "Release");
    ampEnv.add (s, pid::ampVel, "Velocity");

    lfo.add (s, pid::lfoWave, "Wave", true);
    lfo.add (s, pid::lfoRate, "Rate");
    lfo.add (s, pid::lfoFade, "Fade In");
    lfo.add (s, pid::lfoPitch, "Pitch");
    lfo.add (s, pid::lfoFilter, "Filter");
    lfo.add (s, pid::lfoPW, "PW");

    voice.add (s, pid::voiceMode, "Mode", true);
    voice.add (s, pid::glide, "Glide");
    voice.add (s, pid::bendRange, "Bend");
    voice.add (s, pid::drift, "Drift");
    voice.add (s, pid::spread, "Spread");
    voice.add (s, pid::unison, "Unison");
    voice.add (s, pid::uniDetune, "Detune");

    fx.add (s, pid::chorus, "Chorus", true);
    fx.add (s, pid::delayTime, "Time");
    fx.add (s, pid::delayFb, "Feedback");
    fx.add (s, pid::delayMix, "Delay");
    fx.add (s, pid::reverbMix, "Reverb");

    output.add (s, pid::master, "Volume");

    for (auto* c : std::initializer_list<juce::Component*> { &osc1, &osc2, &osc3, &mixer, &filter, &filterEnv, &ampEnv,
                                                             &lfo, &voice, &fx, &output })
        content.addAndMakeVisible (c);

    // Presetväljare grupperad per kategori.
    const auto& presets = getFactoryPresets();
    juce::String lastCategory;
    for (size_t i = 0; i < presets.size(); ++i)
    {
        if (presets[i].category != lastCategory)
        {
            presetBox.addSectionHeading (presets[i].category);
            lastCategory = presets[i].category;
        }
        presetBox.addItem (presets[i].name, (int) i + 1);
    }
    presetBox.setTextWhenNothingSelected ("Preset");
    presetBox.onChange = [this]
    {
        const int id = presetBox.getSelectedId();
        if (id > 0 && id - 1 != processor.getCurrentProgram())
            processor.setCurrentProgram (id - 1);
    };
    content.addAndMakeVisible (presetBox);

    prevButton.onClick = [this] { stepPreset (-1); };
    nextButton.onClick = [this] { stepPreset (1); };
    content.addAndMakeVisible (prevButton);
    content.addAndMakeVisible (nextButton);

    voicesLabel.setJustificationType (juce::Justification::centredRight);
    voicesLabel.setColour (juce::Label::textColourId, steso::ui::colours::dimText);
    voicesLabel.setFont (juce::Font (juce::FontOptions (13.0f)));
    content.addAndMakeVisible (voicesLabel);

    keyboard.setAvailableRange (24, 108);
    keyboard.setLowestVisibleKey (36);
    keyboard.setKeyWidth (22.0f);
    keyboard.setColour (juce::MidiKeyboardComponent::keyDownOverlayColourId, steso::ui::colours::accent.withAlpha (0.7f));
    keyboard.setColour (juce::MidiKeyboardComponent::mouseOverKeyOverlayColourId, steso::ui::colours::accent.withAlpha (0.3f));
    keyboard.setColour (juce::MidiKeyboardComponent::shadowColourId, juce::Colours::transparentBlack);
    keyboard.setColour (juce::MidiKeyboardComponent::keySeparatorLineColourId, steso::ui::colours::background);
    keyboard.setWantsKeyboardFocus (false);
    content.addAndMakeVisible (keyboard);
    content.addAndMakeVisible (seqPanel);

    addAndMakeVisible (content);
    content.setSize (baseWidth, baseHeight);

    setResizable (true, true);
    setResizeLimits (baseWidth * 6 / 10, baseHeight * 6 / 10, baseWidth * 2, baseHeight * 2);
    getConstrainer()->setFixedAspectRatio ((double) baseWidth / (double) baseHeight);
    setSize (baseWidth, baseHeight);

    refreshPresetBox();
    startTimerHz (30);
}

StesoLeadEditor::~StesoLeadEditor()
{
    setLookAndFeel (nullptr);
}

void StesoLeadEditor::paint (juce::Graphics& g)
{
    g.fillAll (steso::ui::colours::background);

    // Titel ritas i skalad bas-koordinat.
    g.addTransform (content.getTransform());
    auto header = juce::Rectangle<float> (18.0f, 8.0f, 400.0f, 44.0f);
    g.setColour (steso::ui::colours::text);
    g.setFont (juce::Font (juce::FontOptions (28.0f, juce::Font::bold)).withExtraKerningFactor (0.04f));
    g.drawText ("Steso", header, juce::Justification::centredLeft);
    const float w = juce::GlyphArrangement::getStringWidth (g.getCurrentFont(), "Steso");
    g.setColour (steso::ui::colours::accent);
    g.drawText ("Lead", header.withTrimmedLeft (w + 1.0f), juce::Justification::centredLeft);
    g.setColour (steso::ui::colours::dimText);
    g.setFont (juce::Font (juce::FontOptions (12.0f)));
    g.drawText ("POLY ANALOG SYNTH  v" STESO_VERSION, header.withTrimmedLeft (150.0f).translated (0.0f, 3.0f),
                juce::Justification::centredLeft);
}

void StesoLeadEditor::resized()
{
    const float scale = (float) getWidth() / (float) baseWidth;
    content.setTransform (juce::AffineTransform::scale (scale));

    auto r = juce::Rectangle<int> (0, 0, baseWidth, baseHeight).reduced (12, 8);

    auto header = r.removeFromTop (46);
    header.removeFromLeft (420);
    voicesLabel.setBounds (header.removeFromRight (110).reduced (0, 10));
    header.removeFromRight (12);
    nextButton.setBounds (header.removeFromRight (34).reduced (0, 8));
    header.removeFromRight (4);
    presetBox.setBounds (header.removeFromRight (240).reduced (0, 8));
    header.removeFromRight (4);
    prevButton.setBounds (header.removeFromRight (34).reduced (0, 8));

    keyboard.setBounds (r.removeFromBottom (62).reduced (3, 4));
    r.removeFromBottom (2);
    seqPanel.setBounds (r.removeFromBottom (268));

    const int rowH = r.getHeight() / 4;
    const int unit = r.getWidth() / 16;   // 16 kolumner totalt per rad

    auto row1 = r.removeFromTop (rowH);
    osc1.setBounds (row1.removeFromLeft (unit * 3));
    osc2.setBounds (row1.removeFromLeft (unit * 4));
    osc3.setBounds (row1.removeFromLeft (unit * 3));
    mixer.setBounds (row1);

    auto row2 = r.removeFromTop (rowH);
    filter.setBounds (row2.removeFromLeft (unit * 11));
    filterEnv.setBounds (row2);

    auto row3 = r.removeFromTop (rowH);
    ampEnv.setBounds (row3.removeFromLeft (unit * 7));
    lfo.setBounds (row3);

    auto row4 = r;
    voice.setBounds (row4.removeFromLeft (unit * 8));
    fx.setBounds (row4.removeFromLeft (unit * 6));
    output.setBounds (row4);
}

void StesoLeadEditor::timerCallback()
{
    const int counter = processor.programChangeCounter.load();
    if (counter != lastProgramCounter)
    {
        lastProgramCounter = counter;
        refreshPresetBox();
        seqPanel.bindState();
    }

    seqPanel.refresh();

    const int v = processor.getActiveVoiceCount();
    if (v != lastVoices)
    {
        lastVoices = v;
        voicesLabel.setText ("Voices " + juce::String (v) + "/" + juce::String (steso::dsp::SynthEngine::maxVoices),
                             juce::dontSendNotification);
    }
}

void StesoLeadEditor::refreshPresetBox()
{
    presetBox.setSelectedId (processor.getCurrentProgram() + 1, juce::dontSendNotification);
}

void StesoLeadEditor::stepPreset (int delta)
{
    const int n = processor.getNumPrograms();
    const int next = (processor.getCurrentProgram() + delta + n) % n;
    processor.setCurrentProgram (next);
    refreshPresetBox();
}
