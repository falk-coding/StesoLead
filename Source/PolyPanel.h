#pragma once

#include "UiCommon.h"
#include "PluginProcessor.h"

// Polyrytm-läget: fyra spår som slår N gånger per cykel (eller delar Rate-klockan).
// Ringarna visar var slagen ligger och var rytmerna möts.
namespace steso::ui
{
juce::Colour polyLaneColour (int lane);

class PolyRings : public juce::Component
{
public:
    explicit PolyRings (StesoLeadProcessor& p) : processor (p) { setInterceptsMouseClicks (false, false); }
    void paint (juce::Graphics&) override;

private:
    StesoLeadProcessor& processor;
};

// Spårets tonhöjdssteg (1–8). Dra upp/ner = halvtoner, dubbelklick = 0.
class LaneSteps : public juce::Component, public juce::SettableTooltipClient
{
public:
    LaneSteps (StesoLeadProcessor& p, int laneIndex);

    void paint (juce::Graphics&) override;
    void mouseDown (const juce::MouseEvent&) override;
    void mouseDrag (const juce::MouseEvent&) override;
    void mouseDoubleClick (const juce::MouseEvent&) override;

private:
    int cellAt (float x) const;
    juce::Rectangle<float> cell (int i) const;

    StesoLeadProcessor& processor;
    const int lane;
    int dragCell = -1, startPitch = 0;
};

class PolyPanel : public juce::Component
{
public:
    explicit PolyPanel (StesoLeadProcessor& p);
    ~PolyPanel() override;

    void paint (juce::Graphics&) override;
    void resized() override;
    // Timer från editorn (30 Hz).
    void refresh();

    // Snabbval. Sätter spårens parametrar (tonhöjdsstegen lämnas orörda).
    struct LaneSetup { bool on; int type, pulses, hits, pitch; };
    void applySetup (std::initializer_list<LaneSetup> lanes);
    void resetAll();

private:
    struct Row
    {
        juce::TextButton on;
        juce::ComboBox type;
        juce::Slider pulses { juce::Slider::IncDecButtons, juce::Slider::TextBoxLeft };
        juce::Slider hits { juce::Slider::IncDecButtons, juce::Slider::TextBoxLeft };
        juce::Slider pitch { juce::Slider::IncDecButtons, juce::Slider::TextBoxLeft };
        juce::Slider steps { juce::Slider::IncDecButtons, juce::Slider::TextBoxLeft };
        std::unique_ptr<LaneSteps> notes;
        std::unique_ptr<juce::AudioProcessorValueTreeState::ButtonAttachment> onA;
        std::unique_ptr<juce::AudioProcessorValueTreeState::ComboBoxAttachment> typeA;
        std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> pulsesA, hitsA, pitchA, stepsA;
    };

    void setParam (const char* id, float value);

    StesoLeadProcessor& processor;
    PolyRings rings;
    std::array<Row, seq::polyLanes> rows;
    juce::Rectangle<int> headerArea;
    std::array<int, 7> columnX {}, columnW {};
    double lastPpq = -2.0;
};
} // namespace steso::ui
