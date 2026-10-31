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

    steso::ui::Section osc1 { "ГЕНЕРАТОР 1|Osc 1", steso::ui::colours::accent };
    steso::ui::Section osc2 { "ГЕНЕРАТОР 2|Osc 2", steso::ui::colours::accent };
    steso::ui::Section osc3 { "ГЕНЕРАТОР 3|Osc 3", steso::ui::colours::accent };
    steso::ui::Section mixer { "СМЕСИТЕЛЬ|Mixer", steso::ui::colours::accent };
    steso::ui::Section filter { "ФИЛЬТР|Filter", steso::ui::colours::accent2 };
    steso::ui::Section filterEnv { "ОГИБАЮЩАЯ ФИЛЬТРА|Filter env", steso::ui::colours::accent2 };
    steso::ui::Section ampEnv { "УСИЛИТЕЛЬ|Amp env", steso::ui::colours::accent };
    steso::ui::Section lfo { "ГНЧ|LFO", steso::ui::colours::accent2 };
    steso::ui::Section voice { "ГОЛОС|Voice", steso::ui::colours::accent };
    steso::ui::Section fx { "ЭФФЕКТЫ|Effects", steso::ui::colours::accent2 };
    steso::ui::Section output { "ВЫХОД|Output", steso::ui::colours::accent };

    juce::MidiKeyboardComponent keyboard;
    steso::ui::SeqPanel seqPanel;

    juce::Image panelTexture;
    int lastProgramCounter = -1;
    int lastVoices = -1;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (StesoLeadEditor)
};
