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

    // Rack-layout: synthenheten + (utfällbart) sekvenser- och klaviaturenhet, alla med 19"-rackfästen.
    static constexpr int earW = 36;                         // rackfäste per sida
    static constexpr int innerW = 1120;                     // panelens bredd mellan fästena
    static constexpr int baseWidth = innerW + 2 * earW;
    static constexpr int synthH = 542, seqH = 272, keysH = 74, unitGap = 6;
    static constexpr int baseHeight = synthH + unitGap + seqH + unitGap + keysH;   // allt utfällt

    // Fäll ut/in sekvenser och klaviatur (sparas i projektet).
    void setPanelsVisible (bool sequencer, bool keyboard);
    int currentBaseHeight() const;

private:
    void timerCallback() override;
    void refreshPresetBox();
    void stepPreset (int delta);
    void rebuildPresetList();
    void savePresetDialog();
    void deletePresetDialog();
    int currentUserIndex() const;   // index i userFiles för laddat eget preset, -1 = inget
    void selectEntry (int entry);    // >= 0 fabriksprogram, < 0 eget preset -(index+1)
    void applyPanelSize (float scale);

    static constexpr int userIdBase = 100000;

    StesoLeadProcessor& processor;
    steso::ui::LookAndFeel lnf;

    juce::Component content;
    juce::ComboBox presetBox;
    juce::TextButton prevButton { "<" }, nextButton { ">" };
    juce::TextButton saveButton { "Spara (Save)" };
    juce::TextButton deleteButton { juce::String::fromUTF8 ("Ta bort (Delete)") };
    juce::TextButton seqToggle { "Sekv. (Seq)" }, keysToggle { "Klav. (Keys)" };
    bool showSeq = false, showKeys = false;
    juce::Array<juce::File> userFiles;

    steso::ui::Section osc1 { "TONGENERATOR 1|Osc 1", steso::ui::colours::accent };
    steso::ui::Section osc2 { "TONGENERATOR 2|Osc 2", steso::ui::colours::accent };
    steso::ui::Section osc3 { "TONGENERATOR 3|Osc 3", steso::ui::colours::accent };
    steso::ui::Section mixer { "NIVÅER|Mixer", steso::ui::colours::accent };
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
    std::vector<int> presetOrder;   // poster i visningsordning: >= 0 fabriksprogram, < 0 eget preset -(i+1)
    int lastProgramCounter = -1;
    int lastVoices = -1;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (StesoLeadEditor)
};
