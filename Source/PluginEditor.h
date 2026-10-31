#pragma once

#include <juce_audio_utils/juce_audio_utils.h>
#include "PluginProcessor.h"

#include "UiCommon.h"
#include "SeqPanel.h"

class StesoLeadEditor : public juce::AudioProcessorEditor, private juce::Timer
{
public:
    explicit StesoLeadEditor (StesoLeadProcessor&);
    ~StesoLeadEditor() override;

    void paint (juce::Graphics&) override;
    void resized() override;

    static constexpr int baseWidth = 1120;
    static constexpr int baseHeight = 940;

private:
    void timerCallback() override;
    void refreshPresetBox();
    void stepPreset (int delta);

    StesoLeadProcessor& processor;
    steso::ui::LookAndFeel lnf;

    juce::Component content;
    juce::ComboBox presetBox;
    juce::TextButton prevButton { "<" }, nextButton { ">" };
    juce::Label voicesLabel;

    steso::ui::Section osc1 { "OSC 1", steso::ui::colours::accent };
    steso::ui::Section osc2 { "OSC 2", steso::ui::colours::accent };
    steso::ui::Section osc3 { "OSC 3", steso::ui::colours::accent };
    steso::ui::Section mixer { "MIXER", steso::ui::colours::accent };
    steso::ui::Section filter { "FILTER", steso::ui::colours::accent2 };
    steso::ui::Section filterEnv { "FILTER ENV", steso::ui::colours::accent2 };
    steso::ui::Section ampEnv { "AMP ENV", steso::ui::colours::accent };
    steso::ui::Section lfo { "LFO", steso::ui::colours::accent2 };
    steso::ui::Section voice { "VOICE", steso::ui::colours::accent };
    steso::ui::Section fx { "EFFECTS", steso::ui::colours::accent2 };
    steso::ui::Section output { "OUTPUT", steso::ui::colours::accent };

    juce::MidiKeyboardComponent keyboard;
    steso::ui::SeqPanel seqPanel;

    int lastProgramCounter = -1;
    int lastVoices = -1;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (StesoLeadEditor)
};
