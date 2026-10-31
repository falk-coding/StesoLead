#pragma once

#include "UiCommon.h"
#include "PluginProcessor.h"

namespace steso::ui
{
// Stegrutnät: On, Accent, Tie, Pitch, Prob, Gate × 32 steg.
class StepGrid : public juce::Component
{
public:
    explicit StepGrid (StesoLeadProcessor& p);

    void paint (juce::Graphics&) override;
    void mouseDown (const juce::MouseEvent&) override;
    void mouseDrag (const juce::MouseEvent&) override;
    void mouseDoubleClick (const juce::MouseEvent&) override;

    // Anropas av timern; ritar om bara när spelhuvudet flyttat sig.
    void updatePlayhead();

    static constexpr int labelWidth = 58;

private:
    enum Row { rowOn = 0, rowAccent, rowTie, rowPitch, rowProb, rowGate, numRows };

    int stepAt (float x) const;
    int rowAt (float y) const;
    juce::Rectangle<float> cell (int row, int step) const;
    int laneLength (int row) const;
    void applyAt (const juce::MouseEvent& e, bool isDown);

    StesoLeadProcessor& processor;
    int64_t lastStep = -2;
    int dragRow = -1, dragStep = -1, startPitch = 0;
    bool paintValue = true;
};

// Ytan man drar MIDI-klippet från.
class DragBox : public juce::Component, public juce::SettableTooltipClient
{
public:
    explicit DragBox (StesoLeadProcessor& p);

    void paint (juce::Graphics&) override;
    void mouseDrag (const juce::MouseEvent&) override;
    void mouseUp (const juce::MouseEvent&) override;

    // Räknar ut exportområdet och texten som visas.
    struct Range { double from = 0, to = 16; juce::String label, fileName; bool fromLocators = false; };
    std::function<Range()> getRange;

private:
    StesoLeadProcessor& processor;
    bool dragging = false;
};

class SeqPanel : public juce::Component
{
public:
    explicit SeqPanel (StesoLeadProcessor& p);

    void paint (juce::Graphics&) override;
    void resized() override;

    // Timer från editorn (30 Hz).
    void refresh();
    // Kopplar om export-inställningarna efter att ett sparat läge laddats.
    void bindState();

private:
    DragBox::Range computeRange() const;

    StesoLeadProcessor& processor;
    std::vector<std::unique_ptr<Control>> controls;
    StepGrid grid;
    DragBox dragBox;

    juce::ComboBox rangeMode;
    juce::Slider fromBar { juce::Slider::IncDecButtons, juce::Slider::TextBoxLeft };
    juce::Slider toBar { juce::Slider::IncDecButtons, juce::Slider::TextBoxLeft };
    juce::Label fromLabel, toLabel, rangeInfo, bypassLabel;
    juce::TextButton reenableButton { juce::String::fromUTF8 ("Slå på igen") };

    juce::Slider euclidHits { juce::Slider::IncDecButtons, juce::Slider::TextBoxLeft };
    juce::Slider euclidRotate { juce::Slider::IncDecButtons, juce::Slider::TextBoxLeft };
    juce::Label hitsLabel, rotateLabel;
    juce::TextButton euclidButton { "Euclid" }, initButton { "Init" };

    juce::String lastInfo;
    int lastLaneHash = -1;
    bool lastBypassVisible = false;
};
} // namespace steso::ui
