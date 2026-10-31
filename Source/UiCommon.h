#pragma once

#include <juce_audio_processors/juce_audio_processors.h>
#include <juce_gui_basics/juce_gui_basics.h>
#include <juce_audio_utils/juce_audio_utils.h>

namespace steso::ui
{
namespace colours
{
    inline const juce::Colour background { 0xff15171b };
    inline const juce::Colour panel      { 0xff22262d };
    inline const juce::Colour panelEdge  { 0xff30353e };
    inline const juce::Colour text       { 0xffe7e2d8 };
    inline const juce::Colour dimText    { 0xff8d929b };
    inline const juce::Colour accent     { 0xffff8a3d };
    inline const juce::Colour accent2    { 0xff4fc3c8 };
    inline const juce::Colour knobBody   { 0xff2f343c };
}

class LookAndFeel : public juce::LookAndFeel_V4
{
public:
    LookAndFeel();

    void drawRotarySlider (juce::Graphics&, int x, int y, int width, int height, float sliderPos,
                           float startAngle, float endAngle, juce::Slider&) override;
    void drawComboBox (juce::Graphics&, int width, int height, bool isButtonDown,
                       int buttonX, int buttonY, int buttonW, int buttonH, juce::ComboBox&) override;
    juce::Font getComboBoxFont (juce::ComboBox&) override;
    juce::Font getPopupMenuFont() override;
    void drawButtonBackground (juce::Graphics&, juce::Button&, const juce::Colour&, bool highlighted, bool down) override;
};

// En ratt eller en väljare med namn under.
class Control : public juce::Component
{
public:
    Control (juce::AudioProcessorValueTreeState& apvts, const juce::String& paramId, const juce::String& label,
             juce::Colour colour, bool isChoice);

    void resized() override;

private:
    juce::Label label;
    std::unique_ptr<juce::Slider> slider;
    std::unique_ptr<juce::ComboBox> combo;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> sliderAttachment;
    std::unique_ptr<juce::AudioProcessorValueTreeState::ComboBoxAttachment> comboAttachment;
};

class Section : public juce::Component
{
public:
    Section (juce::String title, juce::Colour colour);

    void add (juce::AudioProcessorValueTreeState& apvts, const juce::String& paramId, const juce::String& label,
              bool isChoice = false);
    void paint (juce::Graphics&) override;
    void resized() override;

private:
    juce::String title;
    juce::Colour colour;
    std::vector<std::unique_ptr<Control>> controls;
};
} // namespace steso::ui
