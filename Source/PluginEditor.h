// ==============================================================================
// Source/PluginEditor.h
// ==============================================================================
#pragma once

#include <juce_gui_basics/juce_gui_basics.h>
#include <juce_audio_processors/juce_audio_processors.h>
#include "PluginProcessor.h"
#include <functional>

class MidiDragButton : public juce::TextButton {
public:
    MidiDragButton(const juce::String& name) : juce::TextButton(name) {}
    std::function<juce::File()> onDrag;

    void mouseDrag(const juce::MouseEvent& e) override {
        if (onDrag && !isDragging) {
            isDragging = true;
            juce::File f = onDrag();
            if (f.existsAsFile()) {
                juce::StringArray files;
                files.add(f.getFullPathName());
                if (auto* dnd = juce::DragAndDropContainer::findParentDragContainerFor(this)) {
                    dnd->performExternalDragDropOfFiles(files, false);
                }
            }
        }
    }
    void mouseUp(const juce::MouseEvent& e) override {
        juce::TextButton::mouseUp(e);
        isDragging = false;
    }
private:
    bool isDragging = false;
};

class SequencerGrid : public juce::Component {
public:
    SequencerGrid(BassLineMatrixAudioProcessor& p);

    void paint(juce::Graphics& g) override;
    void mouseDown(const juce::MouseEvent& e) override;
    void mouseDrag(const juce::MouseEvent& e) override;
    void mouseWheelMove(const juce::MouseEvent& e, const juce::MouseWheelDetails& wheel) override;

    // ★ 追加：グリッド内で矢印キーの入力を受け取るための関数
    bool keyPressed(const juce::KeyPress& key) override;

    void updateView(int barIndex) {
        if (currentViewBar != barIndex) { currentViewBar = barIndex; repaint(); }
    }

    // ★ ここで選択状態を管理します（すでに記述されていた素晴らしい構成です）
    int selectedTick = -1;
    int selectedRow = -1;

    juce::Rectangle<int> labelArea;
    juce::Rectangle<int> mainGridArea;

private:
    BassLineMatrixAudioProcessor& audioProcessor;
    int currentViewBar = 0;
    juce::Point<int> dragStartPos;
    int originalLength = 0;
};

class BassLineMatrixAudioProcessorEditor : public juce::AudioProcessorEditor,
    public juce::Timer,
    public juce::DragAndDropContainer
{
public:
    BassLineMatrixAudioProcessorEditor(BassLineMatrixAudioProcessor&);
    ~BassLineMatrixAudioProcessorEditor() override;

    void paint(juce::Graphics&) override;
    void resized() override;
    void timerCallback() override;

    // ※Editor全体でのショートカットキーが必要な場合のために残しておきます
    bool keyPressed(const juce::KeyPress& key) override;
    void mouseDown(const juce::MouseEvent& e) override;

private:
    BassLineMatrixAudioProcessor& audioProcessor;
    SequencerGrid seqGrid;

    // --- ビュー切り替えフラグ ---
    bool isSettingsView = false;
    bool isSettings2View = false;

    // --- GUIコンポーネント ---
    juce::TextButton syncButton{ "SYNC" };
    juce::TextButton playButton{ "PLAY" }, stopButton{ "STOP" };
    juce::TextButton followBtn{ "FOLLOW" };
    juce::Label tempoLabel;
    juce::TextButton tempoLockBtn{ "L" };
    juce::Label timeSigLabel{ "", "Time Sig:" }, timeSigSlash{ "", "/" };
    juce::ComboBox timeSigNumMenu, timeSigDenMenu;
    juce::Label barCountLabel{ "", "Bars:" };
    juce::ComboBox barCountMenu;

    juce::Label shiftLabel{ "", "Shift:" };
    juce::Slider shiftSlider;
    juce::TextButton btnClearAll{ "CLEAR ALL" };

    MidiDragButton chordMidiBtn{ "CHORD MIDI" };
    MidiDragButton bassMidiBtn{ "BASS MIDI" };

    juce::TextButton barTabs[4];
    int currentViewBar = 0;
    juce::TextButton slotBtns[4];
    juce::TextButton settingsBtn{ "SETTING" };
    juce::TextButton settings2Btn{ "SETTING 2" };
    // ★ 新規追加: 全マスのロック/解除を行うマスターボタン
    juce::TextButton globalLockBtn{ "LOCK ALL" };
    juce::ComboBox genreMenu;
    juce::TextButton generateButton{ "GENERATE BASSLINE" };

    juce::TextButton modeToggle{ "MODE: SCALE" };
    juce::TextButton chordLockBtn{ "LOCK" };
    juce::ComboBox chordMenus[16];

    juce::GroupComponent inspectorGroup{ "inspector", "STEP INSPECTOR" };
    juce::Slider velSlider, lenSlider, octSlider;
    juce::Label velLabel{ "", "Velocity" }, lenLabel{ "", "Length" }, octLabel{ "", "Octave" };
    juce::TextButton glideToggle{ "GLIDE" }, staccatoToggle{ "STACCATO" }, lockToggle{ "LOCK" };
    juce::Label noteNameLabel;

    juce::GroupComponent barSettingsGroup{ "barSettings", "BAR SETTINGS" };
    juce::Label barKeyLabel{ "", "Key" }, barScaleLabel{ "", "Scale" }, barDivLabel{ "", "Div" };
    juce::ComboBox barKeyMenu, barScaleMenu, divSelector;
    juce::Slider cmplxSlider, entrpSlider, autoGlideSlider;
    juce::Label cmplxLabel{ "", "Cmplx" }, entrpLabel{ "", "Entrp" }, autoGlideLabel{ "", "Glide%" };
    juce::TextButton lockCmplx{ "L" }, lockEntrp{ "L" }, lockGlide{ "L" };
    juce::TextButton anchorToggle{ "ROOT ANCHOR" };

    juce::GroupComponent settingsPanel{ "settingsPanel", "RANDOMIZE MIN / MAX SETTINGS" };
    juce::Slider velRange, lenRange, octRange, cmplxRange, entrpRange, glideRange, humRange;
    juce::Label velRLabel{ "", "Velocity" }, lenRLabel{ "", "Length" }, octRLabel{ "", "Octave" };
    juce::Label cmplxRLabel{ "", "Cmplx" }, entrpRLabel{ "", "Entrp" }, glideRLabel{ "", "Glide%" };
    juce::Label humRLabel{ "", "Humanize" };
    juce::TextButton closeSettingsBtn{ "CLOSE SETTINGS" };

    juce::GroupComponent settings2Panel{ "settings2Panel", "SOUND ENGINE & CHORD INVERSIONS" };

    juce::TextButton chordOnBtn{ "CHORD ON" };
    juce::ComboBox chordSoundMenu;
    juce::Slider chordVolSlider;
    juce::Label chordVolLabel{ "", "Vol:" };
    juce::ComboBox chordTriggerMenu;
    juce::Slider chordOctSlider;
    juce::Label chordOctLabel{ "", "Octave:" };

    juce::TextButton bassOnBtn{ "BASS ON" };
    juce::ComboBox bassSoundMenu;
    juce::Slider bassVolSlider;
    juce::Label bassVolLabel{ "", "Vol:" };

    juce::Slider staccatoRatioSlider;
    juce::Label staccatoRatioLabel{ "", "Staccato Length (%)" };

    juce::TextButton lockStaccatoBtn{ "L" };

    juce::Label inversionTitleLabel{ "", "Chord Inversions for Current Bar:" };
    juce::ComboBox inversionMenus[16];

    juce::TextButton closeSettings2Btn{ "CLOSE" };
    juce::Label recommendedScaleLabel;

    void updateInspectorValues();
    void updateBarSettingsUI();
    void updateTabColors();
    void updateRangeSettingsUI();

    juce::File exportBassMidi();
    juce::File exportChordMidi();

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(BassLineMatrixAudioProcessorEditor)
};