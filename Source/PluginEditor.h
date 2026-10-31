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

    steso::ui::Section osc1 { "TONGENERATOR 1|Osc 1", steso::ui::colours::accent };
    steso::ui::Section osc2 { "TONGENERATOR 2|Osc 2", steso::ui::colours::accent };
    steso::ui::Section osc3 { "TONGENERATOR 3|Osc 3", steso::ui::colours::accent };
    steso::ui::Section mixer { "BLANDARE|Mixer", steso::ui::colours::accent };
    steso::ui::Section filter { "FILTER", steso::ui::colours::accent2 };
    steso::ui::Section filterEnv { "FILTERKURVA|Filter env", steso::ui::colours::accent2 };
    steso::ui::Section ampEnv { "VOLYMKURVA|Amp env", steso::ui::colours::accent };
    steso::ui::Section lfo { "MODULATION|LFO", steso::ui::colours::accent2 };
    steso::ui::Section voice { "RÖST|Voice", steso::ui::colours::accent };
    steso::ui::Section fx { "EFFEKTER|Effects", steso::ui::colours::accent2 };
    steso::ui::Section output { "UTGÅNG|Output", steso::ui::colours::accent };

    juce::MidiKeyboardComponent keyboard;
    steso::ui::SeqPanel seqPanel;

    juce::Image panelTexture;
    std::vector<int> presetOrder;   // programnummer i den ordning de visas
    int lastProgramCounter = -1;
    int lastVoices = -1;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (StesoLeadEditor)
};
