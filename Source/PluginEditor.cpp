// ==============================================================================
// Source/PluginEditor.cpp
// ==============================================================================
#include "PluginProcessor.h"
#include "PluginEditor.h"

SequencerGrid::SequencerGrid(BassLineMatrixAudioProcessor& p) : audioProcessor(p) {
    setWantsKeyboardFocus(true); // ★ 追加：グリッド自身がキーボード入力を受け取る
}
void SequencerGrid::paint(juce::Graphics& g) {
    int slot = audioProcessor.currentSlot.load();
    auto& bs = audioProcessor.barSettingsUI[slot][currentViewBar];
    int numBeats = audioProcessor.timeSigNumerator.load();
    if (numBeats < 1) numBeats = 4;
    int ticksPerBar = numBeats * 24;

    float gridW = mainGridArea.getWidth() - 2.0f;
    float pxPerTick = gridW / (float)juce::jmax(1, ticksPerBar);
    float cellH = mainGridArea.getHeight() / 12.0f;

    for (int row = 0; row < 12; ++row) {
        int pitchIdx = 11 - row;
        juce::Rectangle<float> rArea(labelArea.getX(), mainGridArea.getY() + row * cellH, labelArea.getWidth(), cellH);

        bool isDiatonic = (pitchIdx < 8);
        g.setColour(isDiatonic ? juce::Colours::white.withAlpha(0.15f) : juce::Colours::black.withAlpha(0.2f));
        g.fillRect(rArea);

        g.setColour(juce::Colours::white.withAlpha(0.8f));
        g.setFont(13.0f);

        juce::String degreeLabel = (pitchIdx < 8) ? juce::StringArray{ "R", "2", "3", "4", "5", "6", "7", "8" }[pitchIdx] : "XXXX";
        g.drawText(degreeLabel, rArea.reduced(10, 0), juce::Justification::centredRight);

        g.setColour(juce::Colours::black.withAlpha(0.4f));
        g.fillRect((float)mainGridArea.getX(), (float)(mainGridArea.getY() + row * cellH + cellH - 1.0f), gridW, 1.0f);
    }

    // --- 1. グリッド線の描画 (ここは t に offset を足してはいけません) ---
    for (int t = 0; t <= ticksPerBar; ++t) {
        // グリッド線は「本来のジャストな位置」に引くので t のまま
        float x = mainGridArea.getX() + t * pxPerTick;

        if (t % 24 == 0) {
            g.setColour(juce::Colours::white.withAlpha(0.4f));
            g.fillRect(x, (float)mainGridArea.getY(), 2.0f, (float)mainGridArea.getHeight());
        }
        else if (t % juce::jmax(1, bs.div) == 0) {
            g.setColour(juce::Colours::black.withAlpha(0.6f));
            g.fillRect(x, (float)mainGridArea.getY(), 1.0f, (float)mainGridArea.getHeight());
        }
    }

    // --- 2. ノートの描画 ---
    for (int row = 0; row < 12; ++row) {
        int pitchIdx = 11 - row;
        for (int t = 0; t < ticksPerBar; ++t) {
            int globalTick = (currentViewBar * ticksPerBar) + t;
            const auto& data = audioProcessor.patternUI[slot][pitchIdx][globalTick];

            if (data.velocity > 0) {
                // 色と透明度の計算
                float safeVel = juce::jlimit(1.0f, 127.0f, (float)data.velocity);
                float alpha = juce::jmap(safeVel, 1.0f, 127.0f, 0.4f, 1.0f);
                g.setColour(data.glide ? juce::Colours::cyan.withAlpha(alpha) : juce::Colours::orange.withAlpha(alpha));

                // ★ Humanize offset を加算して X 座標を決定
                float x = mainGridArea.getX() + (float)(t + data.offset) * pxPerTick;

                float rawW = data.length * pxPerTick;
                float w = juce::jmax(rawW, 5.0f);

                juce::Rectangle<float> noteRect(x + 1, mainGridArea.getY() + row * cellH + 2, w - 2, cellH - 4);

                // ノート本体を描画
                g.fillRoundedRectangle(noteRect, 3.0f);

                // --- ここが修正ポイント：if と else if のペアを正しく記述する ---
                if (data.locked) {
                    // ロックされているノートは赤枠
                    g.setColour(juce::Colours::red.withAlpha(0.9f));
                    g.drawRoundedRectangle(noteRect, 3.0f, 2.5f);
                }
                else if (selectedTick == globalTick && selectedRow == pitchIdx) {
                    // 選択されているノートは白枠
                    g.setColour(juce::Colours::white);
                    g.drawRoundedRectangle(noteRect, 3.0f, 2.0f);
                }

                // テキスト（オクターブやスタッカート）を描画
                g.setColour(juce::Colours::black.withAlpha(0.7f));
                g.setFont(11.0f);
                juce::String attrText;
                if (data.octave != 0) attrText = (data.octave > 0 ? "+" : "") + juce::String(data.octave);
                if (data.staccato) attrText += " .";
                g.drawText(attrText, noteRect, juce::Justification::centred);
            }
        }
    }

    int curTick = audioProcessor.getCurrentStep();
    if (curTick >= currentViewBar * ticksPerBar && curTick < (currentViewBar + 1) * ticksPerBar) {
        int tickInBar = curTick % ticksPerBar;
        int snapTick = (tickInBar / juce::jmax(1, bs.div)) * bs.div;

        float headX = mainGridArea.getX() + snapTick * pxPerTick;
        g.setColour(juce::Colours::white.withAlpha(0.3f));
        g.fillRect(headX, (float)mainGridArea.getY(), pxPerTick * bs.div, (float)mainGridArea.getHeight());
    }
    // =========================================================
    // ★ 追加：選択中セルのハイライト描画 (矢印キー用)
    // =========================================================
    if (selectedRow != -1 && selectedTick != -1) {
        int snapDiv = juce::jmax(1, bs.div);
        float x = mainGridArea.getX() + (selectedTick % ticksPerBar) * pxPerTick;
        float y = mainGridArea.getY() + (11 - selectedRow) * cellH;

        g.setColour(juce::Colours::yellow.withAlpha(0.6f));
        g.drawRect(x, y, pxPerTick * snapDiv, cellH, 2.0f); // Divの幅に合わせて枠を描画
    }
} // <--- paint関数の終わり


void SequencerGrid::mouseDown(const juce::MouseEvent& e) {
    if (!mainGridArea.contains(e.getPosition())) return;
    int slot = audioProcessor.currentSlot.load();
    int ticksPerBar = juce::jmax(1, audioProcessor.timeSigNumerator.load()) * 24;
    float pxPerTick = (mainGridArea.getWidth() - 2.0f) / (float)ticksPerBar;

    // 1. スナップ前の生のTick座標を取得
    int rawTickInView = (int)((e.x - mainGridArea.getX()) / pxPerTick);
    int pitchIdx = 11 - (int)((e.y - mainGridArea.getY()) / (mainGridArea.getHeight() / 12.0f));
    int globalRawTick = (currentViewBar * ticksPerBar) + rawTickInView;

    if (pitchIdx < 0 || pitchIdx >= 12 || globalRawTick >= 1024) return;

    // =========================================================
        // ★ 磁力（Hit-Test）判定：クリックした座標にノートがあるか探す (Offset完全対応版)
        // =========================================================
    int targetTick = -1;
    bool hitExisting = false;

    // オフセットのズレ（マイナス・プラス両方）を考慮し、少し未来〜十分な過去まで探す
    int searchStart = std::min(1023, globalRawTick + 24);
    int searchEnd = std::max(0, globalRawTick - 120);

    for (int t = searchStart; t >= searchEnd; --t) {
        auto& d = audioProcessor.patternUI[slot][pitchIdx][t];
        if (d.velocity > 0) {
            int visualStart = t + d.offset;         // 実際の見た目のスタート位置
            int visualEnd = visualStart + d.length; // 実際の見た目のエンド位置

            // クリックした座標が、見た目の枠内に収まっているか
            if (globalRawTick >= visualStart && globalRawTick < visualEnd) {
                targetTick = t;
                hitExisting = true;
                break;
            }
        }
    }    // =========================================================
    // ★ 既存の音がなければ、現在のDiv設定に従ってスナップ（新規配置）
    // =========================================================
    int snapDiv = juce::jmax(1, audioProcessor.barSettingsUI[slot][currentViewBar].div);
    if (!hitExisting) {
        targetTick = (currentViewBar * ticksPerBar) + ((rawTickInView / snapDiv) * snapDiv);
    }

    int globalTick = targetTick;
    auto& data = audioProcessor.patternUI[slot][pitchIdx][globalTick];

    // --- 以降は既存の操作ロジック ---
    if (e.mods.isShiftDown()) {
        if (data.velocity > 0) data.glide = !data.glide;
    }
    else if (e.mods.isRightButtonDown()) {
        if (data.velocity > 0) data.staccato = !data.staccato;
    }
    else {
        if (data.velocity > 0) {
            // すでに選択中なら削除、そうでなければ選択
// すでに選択中なら削除、そうでなければ選択
            if (selectedTick == globalTick && selectedRow == pitchIdx) {
                data.velocity = 0;
                data.locked = false; // ★追加：消去したマスのロックも確実に解除する
                selectedTick = -1; selectedRow = -1;
            }
            else {
                selectedTick = globalTick; selectedRow = pitchIdx;
            }
        }
        else {
            // 新規配置（スナップされた位置）
            bool canPlace = true;
            for (int r = 0; r < 12; ++r) {
                if (audioProcessor.patternUI[slot][r][globalTick].locked) { canPlace = false; break; }
            }
            if (canPlace) {
                for (int r = 0; r < 12; ++r) {
                    audioProcessor.patternUI[slot][r][globalTick].velocity = 0;
                    audioProcessor.patternUI[slot][r][globalTick].staccato = false;
                    audioProcessor.patternUI[slot][r][globalTick].glide = false;
                    audioProcessor.patternUI[slot][r][globalTick].octave = 0;
                    audioProcessor.patternUI[slot][r][globalTick].locked = false;
                }
                data.velocity = 100;
                data.length = snapDiv;
                data.octave = 0;
                data.staccato = false;
                data.glide = false;
                data.locked = false;
                selectedTick = globalTick; selectedRow = pitchIdx;
            }
        }
    }

    // ==============================================================================
    // ★ 修正ブロック 4: PluginEditor.cpp / SequencerGrid::mouseDown 内 (プレビュー発音)
    // ==============================================================================
        // プレビュー発音
    if (selectedTick != -1 && selectedRow != -1) {
        int octOffset = audioProcessor.patternUI[slot][selectedRow][selectedTick].octave;
        // ★ 課題1の修正: globalPitchShiftの重複加算を削除
        int noteNum = audioProcessor.getMidiNoteFromRow(selectedRow, selectedTick, octOffset, slot, false);
        audioProcessor.previewNoteMidi.store(juce::jlimit(0, 127, noteNum));
    }
    dragStartPos = e.getPosition();
    originalLength = data.length;
    audioProcessor.patternUpdated = true;
    repaint();
    grabKeyboardFocus(); // ★ 変更：親ではなく SequencerGrid 自身がフォーカスを保持する
}
void SequencerGrid::mouseDrag(const juce::MouseEvent& e) {
    int slot = audioProcessor.currentSlot.load();
    if (selectedTick != -1 && selectedRow != -1 && e.mods.isLeftButtonDown() && !e.mods.isShiftDown()) {
        auto& data = audioProcessor.patternUI[slot][selectedRow][selectedTick];

        int ticksPerBar = juce::jmax(1, audioProcessor.timeSigNumerator.load()) * 24;
        float pxPerTick = (mainGridArea.getWidth() - 2.0f) / (float)ticksPerBar;

        int deltaTicks = (int)((e.x - dragStartPos.x) / pxPerTick);
        data.length = juce::jlimit(1, 96, originalLength + deltaTicks);
        audioProcessor.patternUpdated = true;
        repaint();
    }
}

void SequencerGrid::mouseWheelMove(const juce::MouseEvent& e, const juce::MouseWheelDetails& wheel) {
    if (!mainGridArea.contains(e.getPosition())) return;

    int slot = audioProcessor.currentSlot.load();
    int ticksPerBar = juce::jmax(1, audioProcessor.timeSigNumerator.load()) * 24;
    float pxPerTick = (mainGridArea.getWidth() - 2.0f) / (float)ticksPerBar;

    int tickInView = (int)((e.x - mainGridArea.getX()) / pxPerTick);
    int pitchIdx = 11 - (int)((e.y - mainGridArea.getY()) / (mainGridArea.getHeight() / 12.0f));

    for (int i = 0; i < ticksPerBar; ++i) {
        int globalTick = (currentViewBar * ticksPerBar) + i;
        auto& data = audioProcessor.patternUI[slot][pitchIdx][globalTick];

        if (data.velocity > 0) {
            int visualStart = i + data.offset;
            int visualEnd = visualStart + data.length;

            // オフセットを反映した表示上の範囲でホイール判定を行う
            if (tickInView >= visualStart && tickInView < visualEnd) {

                if (e.mods.isCtrlDown() || e.mods.isCommandDown()) {
                    // Ctrl/Cmd + ホイールでベロシティ変更
                    data.velocity = juce::jlimit(1, 127, data.velocity + (wheel.deltaY > 0 ? 5 : -5));
                }
                // ==============================================================================
                // ★ 修正ブロック 5: PluginEditor.cpp / SequencerGrid::mouseWheelMove 内
                // ==============================================================================
                else {
                    // 通常ホイールでオクターブ変更
                    data.octave = juce::jlimit(-2, 2, data.octave + (wheel.deltaY > 0 ? 1 : -1));
                    // ★ 課題1の修正: globalPitchShiftの重複加算を削除
                    int noteNum = audioProcessor.getMidiNoteFromRow(pitchIdx, globalTick, data.octave, slot, false);
                    audioProcessor.previewNoteMidi.store(juce::jlimit(0, 127, noteNum));
                }
                audioProcessor.patternUpdated = true;
                repaint();
                break; // 見つかって処理したらループを抜ける
            } // <- 追加：表示範囲の if の閉じ括弧
        } // <- 追加：velocity > 0 の if の閉じ括弧
    } // <- 既存：for ループの閉じ括弧
} // <- 既存：関数の閉じ括弧
// ==============================================================================
// ★ 新規追加：矢印キーによるノート移動ロジック
// ==============================================================================
bool SequencerGrid::keyPressed(const juce::KeyPress& key) {
    if (selectedRow == -1 || selectedTick == -1) return false;

    int slot = audioProcessor.currentSlot.load();
    int newRow = selectedRow;
    int newTick = selectedTick;

    // 現在のTickがどの小節に属しているか確認し、その小節のDivを取得
    int barIndex = selectedTick / 96;
    auto& bs = audioProcessor.barSettingsUI[slot][juce::jlimit(0, 7, barIndex)];

    // 移動ステップ量：UI側のDiv設定(1/16=6, 1/8=12等)がそのままTick数になる
    int moveStep = juce::jmax(1, (int)bs.div);

    // --- 移動先の計算 ---
    if (key == juce::KeyPress::upKey)          newRow = juce::jlimit(0, 11, selectedRow + 1);
    else if (key == juce::KeyPress::downKey)   newRow = juce::jlimit(0, 11, selectedRow - 1);
    else if (key == juce::KeyPress::rightKey)  newTick = (selectedTick + moveStep) % 1024;
    else if (key == juce::KeyPress::leftKey)   newTick = (selectedTick - moveStep + 1024) % 1024;
    else return false;

    // --- データの引っ越し ---
    if (newRow != selectedRow || newTick != selectedTick) {
        auto& oldData = audioProcessor.patternUI[slot][selectedRow][selectedTick];
        auto& newData = audioProcessor.patternUI[slot][newRow][newTick];

        // 元の場所に音がある場合のみ「移動」を実行
        if (oldData.velocity > 0)
        {
            newData = oldData;      // データをコピー
            oldData.velocity = 0;   // 元の場所を消去
            oldData.locked = false; // ★追加：移動元のマスのロックを解除する

            selectedRow = newRow;   // 選択枠を更新
            selectedTick = newTick;

            audioProcessor.patternUpdated = true;
            repaint();
            return true;
        }
        else {
            // 元のセルに音が無い場合は枠だけ動かす
            selectedRow = newRow;
            selectedTick = newTick;
            repaint();
            return true;
        }
    }
    return false;
}

// ==============================================================================
// ★ Main Editor 実装
// ==============================================================================
BassLineMatrixAudioProcessorEditor::BassLineMatrixAudioProcessorEditor(BassLineMatrixAudioProcessor& p)
    : AudioProcessorEditor(&p), audioProcessor(p), seqGrid(p)
{
    setSize(1200, 850);
    setWantsKeyboardFocus(true);

    addAndMakeVisible(seqGrid);
    addChildComponent(settingsPanel);
    addChildComponent(settings2Panel);

    addAndMakeVisible(syncButton);
    syncButton.setClickingTogglesState(true);
    syncButton.setColour(juce::TextButton::buttonOnColourId, juce::Colours::orange);

    addAndMakeVisible(playButton); addAndMakeVisible(stopButton);
    addAndMakeVisible(followBtn);
    followBtn.setClickingTogglesState(true);
    followBtn.setColour(juce::TextButton::buttonOnColourId, juce::Colours::orange);
    followBtn.setToggleState(true, juce::dontSendNotification);

    addAndMakeVisible(tempoLabel);

    addAndMakeVisible(tempoLockBtn);
    tempoLockBtn.setClickingTogglesState(true);
    tempoLockBtn.setColour(juce::TextButton::buttonOnColourId, juce::Colours::red);
    tempoLockBtn.onClick = [this] { audioProcessor.isTempoLocked = tempoLockBtn.getToggleState(); };

    addAndMakeVisible(timeSigLabel);
    addAndMakeVisible(timeSigNumMenu); addAndMakeVisible(timeSigSlash); addAndMakeVisible(timeSigDenMenu);
    addAndMakeVisible(barCountLabel); addAndMakeVisible(barCountMenu);

    addAndMakeVisible(shiftLabel);
    shiftLabel.setJustificationType(juce::Justification::centredRight);
    addAndMakeVisible(shiftSlider);
    shiftSlider.setSliderStyle(juce::Slider::LinearHorizontal);
    shiftSlider.setTextBoxStyle(juce::Slider::TextBoxLeft, false, 35, 20);
    shiftSlider.setRange(-24, 24, 1);
    shiftSlider.setValue(audioProcessor.globalPitchShift.load());
    shiftSlider.onValueChange = [this] {
        audioProcessor.globalPitchShift.store((int)shiftSlider.getValue());
        audioProcessor.uiNeedsUpdate = true;
        audioProcessor.patternUpdated = true;
        };

    addAndMakeVisible(bassMidiBtn);
    bassMidiBtn.setColour(juce::TextButton::buttonColourId, juce::Colours::purple.withAlpha(0.6f));
    bassMidiBtn.onDrag = [this] { return exportBassMidi(); };

    addAndMakeVisible(chordMidiBtn);
    chordMidiBtn.setColour(juce::TextButton::buttonColourId, juce::Colours::hotpink.withAlpha(0.6f));
    chordMidiBtn.onDrag = [this] { return exportChordMidi(); };

    addAndMakeVisible(btnClearAll);

    for (int i = 0; i < 4; ++i) {
        addAndMakeVisible(slotBtns[i]);
        slotBtns[i].setButtonText("P " + juce::String(i + 1));
        slotBtns[i].onClick = [this, i] {
            audioProcessor.currentSlot.store(i);
            audioProcessor.patternUpdated = true;
            seqGrid.selectedTick = -1; seqGrid.selectedRow = -1;

            // ★ 追加: スロットを切り替えたら一旦LOCK ALLの見た目をOFFにする（誤操作防止）
            globalLockBtn.setToggleState(false, juce::dontSendNotification);

            updateTabColors(); updateBarSettingsUI(); seqGrid.repaint();
            grabKeyboardFocus();
            };
    }
    addAndMakeVisible(settingsBtn);
    settingsBtn.setClickingTogglesState(true);
    settingsBtn.setColour(juce::TextButton::buttonOnColourId, juce::Colours::orange);

    addAndMakeVisible(settings2Btn);
    settings2Btn.setClickingTogglesState(true);
    settings2Btn.setColour(juce::TextButton::buttonOnColourId, juce::Colours::cyan.darker());

    // ==============================================================================
    // ★ 新規追加: LOCK ALL ボタンの初期設定とロジック
    // ==============================================================================
    addAndMakeVisible(globalLockBtn);
    globalLockBtn.setClickingTogglesState(true);
    globalLockBtn.setColour(juce::TextButton::buttonOnColourId, juce::Colours::red); // ロック時は赤く点灯
    globalLockBtn.onClick = [this] {
        bool isLocked = globalLockBtn.getToggleState();
        int slot = audioProcessor.currentSlot.load();

        // 現在のスロットの全1024 Ticks、全12 Rowsのロック状態を一括書き換え
        for (int r = 0; r < 12; ++r) {
            for (int t = 0; t < 1024; ++t) {
                if (audioProcessor.patternUI[slot][r][t].velocity > 0) {
                    // 音があるマスだけロック状態を適用
                    audioProcessor.patternUI[slot][r][t].locked = isLocked;
                }
                else {
                    // ★重要：空マスは必ずロック解除（ゴーストロック防止）
                    audioProcessor.patternUI[slot][r][t].locked = false;
                }
            }
        }
        audioProcessor.patternUpdated = true;
        seqGrid.repaint();
        updateInspectorValues();
        };    // ==============================================================================
    for (int i = 0; i < 4; ++i) addAndMakeVisible(barTabs[i]);
    addAndMakeVisible(genreMenu); addAndMakeVisible(generateButton);

    addAndMakeVisible(modeToggle);
    modeToggle.setColour(juce::TextButton::buttonColourId, juce::Colours::darkgrey);
    modeToggle.setColour(juce::TextButton::buttonOnColourId, juce::Colours::hotpink.darker());
    modeToggle.setClickingTogglesState(true);
    modeToggle.onClick = [this] {
        int slot = audioProcessor.currentSlot.load();
        auto& bs = audioProcessor.barSettingsUI[slot][currentViewBar];
        bs.useCodeMode = modeToggle.getToggleState();
        audioProcessor.patternUpdated = true;
        updateBarSettingsUI(); seqGrid.repaint();
        };

    addAndMakeVisible(chordLockBtn);
    chordLockBtn.setClickingTogglesState(true);
    chordLockBtn.setColour(juce::TextButton::buttonOnColourId, juce::Colours::red);
    chordLockBtn.onClick = [this] {
        int slot = audioProcessor.currentSlot.load();
        audioProcessor.barSettingsUI[slot][currentViewBar].lockChords = chordLockBtn.getToggleState();
        };

    // --- コードメニューの初期化（15種類対応版） ---
    const char* romanNames[] = { "I", "II", "III", "IV", "V", "VI", "VII" };

    // UI上の表示名（Enumの順番と一致させてください）
    const char* qualNames[] = {
        "", "m", "7", "m7", "M7", "dim", "m7b5",
        "dim7", "5", "m9", "M9", "7b9", "alt", "13", "aug"
    };

    const int numQuals = 15; // 種類を 7 から 15 に拡張

    for (int i = 0; i < 16; ++i) {
        addAndMakeVisible(chordMenus[i]);
        chordMenus[i].setJustificationType(juce::Justification::centred);

        for (int d = 0; d < 7; ++d) {
            for (int q = 0; q < numQuals; ++q) {
                // ID計算の係数を 15 (numQuals) に変更
                chordMenus[i].addItem(juce::String(romanNames[d]) + qualNames[q], d * numQuals + q + 1);
            }
        }

        // --- PluginEditor.cpp 265行目付近 ---
        chordMenus[i].onChange = [this, i] { // numQualsを直接数値で書くかキャプチャ
            int slot = audioProcessor.currentSlot.load();
            auto& bs = audioProcessor.barSettingsUI[slot][currentViewBar];

            int id = chordMenus[i].getSelectedId() - 1;
            if (id >= 0) {
                // ★ 7 ではなく 15 で割るのが正解
                bs.chords[i].degree = id / 15;
                bs.chords[i].quality = static_cast<ChordQuality>(id % 15);

                audioProcessor.uiNeedsUpdate = true;
                audioProcessor.patternUpdated = true;
            }
            };
    }

    for (const auto& genre : genreRegistry) { genreMenu.addItem(juce::String(genre.name), genre.id); }
    // --- PluginEditor.cpp コンストラクタ内 ---

// 保存されている現在のジャンルIDをProcessorから読み取ってセットする
    genreMenu.setSelectedId(audioProcessor.currentGenre.load(), juce::dontSendNotification);

    // さらに、画面を開いた瞬間にそのジャンルに応じた「推奨スケール」なども表示させる
    audioProcessor.uiNeedsUpdate.store(true);

    // ★ バグ修正: ジャンルが変更された瞬間に、Setting 1の範囲やテンポなどの最適値をDNAからロードして同期

// --- PluginEditor.cpp 内の genreMenu.onChange ---
    genreMenu.onChange = [this] {
        int id = genreMenu.getSelectedId();
        audioProcessor.currentGenre.store(id);
        const auto& dna = genreRegistry[std::clamp(id - 1, 0, (int)genreRegistry.size() - 1)];

        audioProcessor.genSettings.vel = dna.defVel;
        audioProcessor.genSettings.len = dna.defLen;
        audioProcessor.genSettings.cmplx = dna.defCmplx;
        audioProcessor.genSettings.entrp = dna.defEntrp;
        audioProcessor.genSettings.glide = dna.defGlide;
        audioProcessor.genSettings.hum.max = dna.humanizeMax;
        audioProcessor.staccatoRatio.store(dna.defaultStaccato);

        // ジャンル変更時のみ、全スロット・全小節のDivをジャンル推奨値に合わせる
        for (int s = 0; s < 4; ++s) {
            for (int b = 0; b < 8; ++b) {
                audioProcessor.barSettingsUI[s][b].div = dna.defaultDiv;
            }
        }

        // テンポを計算して内部変数に保存
        if (!audioProcessor.isTempoLocked.load()) {
            double newTempo = (double)(dna.minTempo + dna.maxTempo) / 2.0;
            audioProcessor.internalTempo.store(newTempo);

            // 強制的に表示を更新
            if (!audioProcessor.isSyncEnabled.load())
                tempoLabel.setText(juce::String(newTempo, 1) + " BPM", juce::dontSendNotification);
        }

        updateRangeSettingsUI();

        // UIのDiv表記とグリッド線を即座に最新状態へ更新！
        updateBarSettingsUI();
        seqGrid.repaint();

        // ★ 追加：ジャンルに応じた推奨スケールテキストを生成して表示
        juce::String recText = "Recommended Scales\n(for Scale Mode):\n\n";
        switch (id) {
        case 1:  recText += "2. Natural Min\n9. Phrygian\n15. Locrian\n6. Harmonic Min"; break; // 01. Techno
        case 2:  recText += "5. Dorian\n11. Minor Blues\n8. Mixolydian\n2. Natural Min"; break; // 02. House
        case 3:  recText += "5. Dorian (Essential)\n11. Minor Blues\n10. Melodic Min\n2. Natural Min"; break; // 03. UK Garage
        case 4:  recText += "2. Natural Min\n9. Phrygian (Dark)\n6. Harmonic Min\n15. Locrian"; break; // 04. Drum & Bass
        case 5:  // 05. Trap
            recText += "9. Phrygian (Dark/Essential)\n"
                "22. Phrygian Dom (Ethnic/Aggressive)\n"
                "2. Natural Min (Standard)\n"
                "15. Locrian (Industrial)";
        break;
        case 6:  recText += "2. Natural Min\n5. Dorian\n19. Half-Whole Dim (Panic)"; break; // 06. Footwork
        case 7:  recText += "7. Lydian (#4: Dreamy/Future)\n18. Whole Tone (Calculated Chaos)\n40. Enigmatic (Alien/Exotic)\n19. Half-Whole Dim (Mathematical)"; break; // 07. IDM
        case 8:  recText += "9. Phrygian (Dark)\n15. Locrian\n24. Hungarian Min\n22. Phrygian Dom"; break; // 08. Dubstep
            // --- PluginEditor.cpp 内 genreMenu.onChange 内の追加 ---
        case 9:  // 09. Afrobeat
            recText += "5. Dorian (Essential/Funk)\n"
                "3. Pentatonic Maj (Highlife)\n"
                "11. Minor Blues (Jazzy Afro)\n"
                "2. Natural Min (Modern Pop)";
        break;
        case 10: recText += "9. Phrygian (Dark)\n15. Locrian\n22. Phrygian Dom"; break; // Gqom
        case 11: recText += "5. Dorian (Jazz vibe)\n10. Melodic Min\n11. Minor Blues"; break; // Amapiano
        case 12: recText += "23. Dbl Harmonic (Bhairav)\n22. Phrygian Dom\n24. Hungarian Min"; break; // Indian
        case 13: recText += "2. Natural Min\n6. Harmonic Min\n9. Phrygian"; break; // Latin / Reggaeton
        case 14: recText += "2. Natural Min\n6. Harmonic Min\n9. Phrygian"; break; // 14. Trance
        case 15: // 15. Synthwave
            recText += "2. Natural Minor (Essential Nostalgia)\n"
                "5. Dorian (Sophisticated 80s)\n"
                "10. Melodic Minor (Cinematic Dramaturgy)";
        break;        case 16: recText += "8. Mixolydian (Dominant Funk)\n5. Dorian (Minor Funk)\n11. Minor Blues (Classic)\n12. Major Blues (Gospel/Soul)"; break; // 16. Funk / Disco
        case 17: recText += "5. Dorian (Urban standard)\n1. Major (Ionian / Pop-Jack)\n11. Minor Blues (Classic R&B)\n8. Mixolydian (Gospel/Funk)"; break; // 17. New Jack Swing
        case 18: recText += "5. Dorian (Soulful/Cool)\n10. Melodic Minor (Jazzy)\n3. Pentatonic Maj (Gospel)\n11. Minor Blues (Classic)"; break; // 18. Neo Soul
        case 19: recText += "2. Natural Min (Classic)\n11. Minor Blues (Gritty)\n5. Dorian (Jazzy)\n4. Pentatonic Min (Basic)"; break; // 19. Boom Bap
        case 20: recText += "5. Dorian (Essential Soul)\n7. Lydian (Modern Sophistication)\n10. Melodic Minor (Urban/Tension)\n8. Mixolydian (Bluesy/Jazz)"; break; // 20. Urban Jazz
        case 21: recText += "2. Natural Min\n6. Harmonic Min\n5. Dorian"; break; // 21. Melodic Techno
        case 22: recText += "8. Mixolydian (Jazz Standard)\n5. Dorian (Cool Jazz)\n1. Major (Traditional)\n2. Natural Minor (Walking Minor)"; break; // 22. Walking Bass
        case 23: // 23. Electronic Generic
            recText += "2. Natural Minor (Standard EDM)\n"
                "5. Dorian (House / Techno)\n"
                "1. Major (Pop / Uplifting)\n"
                "11. Minor Blues (Funky Electronic)";
        break;
        default: recText += "1. Major (Ionian)\n2. Natural Min"; break;
        }
        recommendedScaleLabel.setText(recText, juce::dontSendNotification);
        };

    generateButton.setColour(juce::TextButton::buttonColourId, juce::Colours::orange.darker());
    generateButton.onClick = [this] {
        audioProcessor.generateBassline();
        seqGrid.repaint(); updateBarSettingsUI(); grabKeyboardFocus();
        };

    addAndMakeVisible(inspectorGroup);
    addAndMakeVisible(velSlider); addAndMakeVisible(lenSlider); addAndMakeVisible(octSlider);
    addAndMakeVisible(velLabel); addAndMakeVisible(lenLabel); addAndMakeVisible(octLabel);
    addAndMakeVisible(glideToggle); addAndMakeVisible(staccatoToggle); addAndMakeVisible(lockToggle);

    addAndMakeVisible(noteNameLabel);
    noteNameLabel.setFont(juce::Font(28.0f, juce::Font::bold));
    noteNameLabel.setColour(juce::Label::textColourId, juce::Colours::white);
    noteNameLabel.setJustificationType(juce::Justification::centred);
    noteNameLabel.setText("--", juce::dontSendNotification);

    addAndMakeVisible(barSettingsGroup);
    addAndMakeVisible(barKeyLabel); addAndMakeVisible(barScaleLabel); addAndMakeVisible(barDivLabel);
    barKeyLabel.setJustificationType(juce::Justification::centredLeft);
    barScaleLabel.setJustificationType(juce::Justification::centredLeft);
    barDivLabel.setJustificationType(juce::Justification::centredLeft);

    addAndMakeVisible(barKeyMenu); addAndMakeVisible(barScaleMenu); addAndMakeVisible(divSelector);
    addAndMakeVisible(cmplxSlider); addAndMakeVisible(entrpSlider); addAndMakeVisible(autoGlideSlider);
    addAndMakeVisible(cmplxLabel); addAndMakeVisible(entrpLabel); addAndMakeVisible(autoGlideLabel);
    addAndMakeVisible(lockCmplx); addAndMakeVisible(lockEntrp); addAndMakeVisible(lockGlide);
    addAndMakeVisible(anchorToggle);

    auto setupRangeSlider = [this](juce::Slider& s, juce::Label& l, double min, double max) {
        settingsPanel.addAndMakeVisible(s); settingsPanel.addAndMakeVisible(l);
        s.setSliderStyle(juce::Slider::TwoValueHorizontal);
        s.setTextBoxStyle(juce::Slider::NoTextBox, false, 0, 0);
        s.setRange(min, max, 1);
        l.setJustificationType(juce::Justification::centredLeft);
        };
    setupRangeSlider(velRange, velRLabel, 1, 127); setupRangeSlider(lenRange, lenRLabel, 1, 96); setupRangeSlider(octRange, octRLabel, -2, 2);
    setupRangeSlider(cmplxRange, cmplxRLabel, 0, 100); setupRangeSlider(entrpRange, entrpRLabel, 0, 100); setupRangeSlider(glideRange, glideRLabel, 0, 100); setupRangeSlider(humRange, humRLabel, 0, 10);
    settingsPanel.addAndMakeVisible(closeSettingsBtn);

    settings2Panel.addAndMakeVisible(chordOnBtn);
    chordOnBtn.setClickingTogglesState(true); chordOnBtn.setToggleState(true, juce::dontSendNotification);
    chordOnBtn.setColour(juce::TextButton::buttonOnColourId, juce::Colours::green);
    chordOnBtn.onClick = [this] { audioProcessor.isChordOn = chordOnBtn.getToggleState(); };

    settings2Panel.addAndMakeVisible(chordSoundMenu);
    chordSoundMenu.addItemList({ "Stab 1 (Sine)", "Stab 2 (Saw)", "Stab 3 (Square)", "Pad 1 (Sine)", "Pad 2 (Saw)", "Pad 3 (Square)" }, 1);
    chordSoundMenu.setSelectedId(1, juce::dontSendNotification);
    chordSoundMenu.onChange = [this] { audioProcessor.chordSoundType = chordSoundMenu.getSelectedId() - 1; audioProcessor.patternUpdated = true; };

    settings2Panel.addAndMakeVisible(chordVolLabel); chordVolLabel.setJustificationType(juce::Justification::centredRight);
    settings2Panel.addAndMakeVisible(chordVolSlider);
    chordVolSlider.setSliderStyle(juce::Slider::LinearHorizontal); chordVolSlider.setTextBoxStyle(juce::Slider::NoTextBox, false, 0, 0);
    chordVolSlider.setRange(0.0, 2.0, 0.01);
    chordVolSlider.setSkewFactorFromMidPoint(0.5);
    chordVolSlider.setValue(0.1, juce::dontSendNotification);
    chordVolSlider.onValueChange = [this] { audioProcessor.chordVolume = (float)chordVolSlider.getValue(); };

    settings2Panel.addAndMakeVisible(chordTriggerMenu);
    chordTriggerMenu.addItemList({ "Trigger: On Change", "Trigger: Every Beat" }, 1);
    chordTriggerMenu.setSelectedId(2, juce::dontSendNotification);
    chordTriggerMenu.onChange = [this] { audioProcessor.chordTriggerMode = chordTriggerMenu.getSelectedId() - 1; };

    settings2Panel.addAndMakeVisible(chordOctLabel); chordOctLabel.setJustificationType(juce::Justification::centredRight);
    settings2Panel.addAndMakeVisible(chordOctSlider);
    chordOctSlider.setSliderStyle(juce::Slider::LinearHorizontal); chordOctSlider.setTextBoxStyle(juce::Slider::TextBoxRight, false, 30, 20);
    chordOctSlider.setRange(-2.0, 2.0, 1.0); chordOctSlider.setValue(0.0, juce::dontSendNotification);
    chordOctSlider.onValueChange = [this] { audioProcessor.chordOctave = (int)chordOctSlider.getValue(); audioProcessor.patternUpdated = true; };

    settings2Panel.addAndMakeVisible(bassOnBtn);
    bassOnBtn.setClickingTogglesState(true); bassOnBtn.setToggleState(true, juce::dontSendNotification);
    bassOnBtn.setColour(juce::TextButton::buttonOnColourId, juce::Colours::orange);
    bassOnBtn.onClick = [this] { audioProcessor.isBassOn = bassOnBtn.getToggleState(); };

    settings2Panel.addAndMakeVisible(bassSoundMenu);
    bassSoundMenu.addItemList({ "Sine/Pluck Bass", "Saw Bass", "Square Sub", "Acoustic Pluck" }, 1);
    bassSoundMenu.setSelectedId(1, juce::dontSendNotification);
    bassSoundMenu.onChange = [this] { audioProcessor.bassSoundType = bassSoundMenu.getSelectedId() - 1; audioProcessor.patternUpdated = true; };

    settings2Panel.addAndMakeVisible(bassVolLabel); bassVolLabel.setJustificationType(juce::Justification::centredRight);
    settings2Panel.addAndMakeVisible(bassVolSlider);
    bassVolSlider.setSliderStyle(juce::Slider::LinearHorizontal); bassVolSlider.setTextBoxStyle(juce::Slider::NoTextBox, false, 0, 0);
    bassVolSlider.setRange(0.0, 2.0, 0.01);
    bassVolSlider.setSkewFactorFromMidPoint(0.5);
    bassVolSlider.setValue(0.8, juce::dontSendNotification);
    bassVolSlider.onValueChange = [this] { audioProcessor.bassVolume = (float)bassVolSlider.getValue(); };

    settings2Panel.addAndMakeVisible(staccatoRatioLabel); staccatoRatioLabel.setJustificationType(juce::Justification::centredRight);
    settings2Panel.addAndMakeVisible(staccatoRatioSlider);
    staccatoRatioSlider.setSliderStyle(juce::Slider::LinearHorizontal); staccatoRatioSlider.setTextBoxStyle(juce::Slider::TextBoxRight, false, 50, 20);
    staccatoRatioSlider.setRange(10.0, 95.0, 1.0); staccatoRatioSlider.setValue(30.0, juce::dontSendNotification);
    staccatoRatioSlider.onValueChange = [this] { audioProcessor.staccatoRatio = (float)(staccatoRatioSlider.getValue() / 100.0); audioProcessor.patternUpdated = true; };
    // ★ ここから挿入 ★
    settings2Panel.addAndMakeVisible(lockStaccatoBtn);
    lockStaccatoBtn.setButtonText("L");
    lockStaccatoBtn.setClickingTogglesState(true);
    lockStaccatoBtn.setColour(juce::TextButton::buttonOnColourId, juce::Colours::red);
    lockStaccatoBtn.onClick = [this] {
        audioProcessor.isStaccatoLocked.store(lockStaccatoBtn.getToggleState());
        };
    // ★ ここまで ★

    settings2Panel.addAndMakeVisible(inversionTitleLabel);
    inversionTitleLabel.setJustificationType(juce::Justification::centredLeft);

    for (int i = 0; i < 16; ++i) {
        settings2Panel.addAndMakeVisible(inversionMenus[i]);
        inversionMenus[i].addItemList({ "Root", "1st", "2nd", "3rd" }, 1);
        inversionMenus[i].onChange = [this, i] {
            int slot = audioProcessor.currentSlot.load();
            audioProcessor.barSettingsUI[slot][currentViewBar].chords[i].inversion = inversionMenus[i].getSelectedId() - 1;
            audioProcessor.uiNeedsUpdate = true;
            audioProcessor.patternUpdated = true;
            };
    }

    settings2Panel.addAndMakeVisible(closeSettings2Btn);
    // ★ 追加：推奨スケールラベルの初期設定
    settings2Panel.addAndMakeVisible(recommendedScaleLabel);
    recommendedScaleLabel.setFont(juce::Font(14.0f));
    recommendedScaleLabel.setColour(juce::Label::textColourId, juce::Colours::cyan.withAlpha(0.8f)); // 少し色をつけて目立たせる
    recommendedScaleLabel.setJustificationType(juce::Justification::topLeft); // 左上揃え

    auto& gs = audioProcessor.genSettings;
    velRange.setMinAndMaxValues(gs.vel.min, gs.vel.max, juce::dontSendNotification);
    lenRange.setMinAndMaxValues(gs.len.min, gs.len.max, juce::dontSendNotification);
    octRange.setMinAndMaxValues(gs.oct.min, gs.oct.max, juce::dontSendNotification);
    cmplxRange.setMinAndMaxValues(gs.cmplx.min, gs.cmplx.max, juce::dontSendNotification);
    entrpRange.setMinAndMaxValues(gs.entrp.min, gs.entrp.max, juce::dontSendNotification);
    glideRange.setMinAndMaxValues(gs.glide.min, gs.glide.max, juce::dontSendNotification);
    humRange.setMinAndMaxValues(gs.hum.min, gs.hum.max, juce::dontSendNotification);

    syncButton.onClick = [this] { audioProcessor.isSyncEnabled = syncButton.getToggleState(); };
    playButton.onClick = [this] { audioProcessor.isPlayingInternal = true; };
    stopButton.onClick = [this] {
        if (audioProcessor.isPlayingInternal.load()) {
            audioProcessor.isPlayingInternal = false;
        }
        else {
            audioProcessor.resetPosition();
            if (followBtn.getToggleState()) {
                currentViewBar = 0; seqGrid.updateView(0); updateBarSettingsUI(); updateTabColors(); seqGrid.repaint();
            }
        }
        };

    settingsBtn.onClick = [this] {
        if (settingsBtn.getToggleState()) { settings2Btn.setToggleState(false, juce::sendNotification); }
        isSettingsView = settingsBtn.getToggleState();
        seqGrid.setVisible(!isSettingsView && !isSettings2View);
        settingsPanel.setVisible(isSettingsView);
        if (isSettingsView) updateRangeSettingsUI();
        resized();
        };
    closeSettingsBtn.onClick = [this] { settingsBtn.setToggleState(false, juce::sendNotification); };

    settings2Btn.onClick = [this] {
        if (settings2Btn.getToggleState()) { settingsBtn.setToggleState(false, juce::sendNotification); }
        isSettings2View = settings2Btn.getToggleState();
        seqGrid.setVisible(!isSettingsView && !isSettings2View);
        settings2Panel.setVisible(isSettings2View);
        if (isSettings2View) updateBarSettingsUI();
        resized();
        };
    closeSettings2Btn.onClick = [this] { settings2Btn.setToggleState(false, juce::sendNotification); };

    tempoLabel.setEditable(true);
    tempoLabel.setText(juce::String(audioProcessor.internalTempo.load(), 1) + " BPM", juce::dontSendNotification);
    tempoLabel.onTextChange = [this] {
        double t = tempoLabel.getText().getDoubleValue();
        if (t > 0) audioProcessor.internalTempo.store(juce::jlimit(20.0, 999.0, t));
        };

    for (int i = 1; i <= 16; ++i) timeSigNumMenu.addItem(juce::String(i), i);
    timeSigDenMenu.addItem("4", 4); timeSigDenMenu.addItem("8", 8); timeSigDenMenu.addItem("16", 16);
    for (int i = 1; i <= 4; ++i) barCountMenu.addItem(juce::String(i) + " Bars", i);

    auto updateSig = [this] {
        audioProcessor.timeSigNumerator = timeSigNumMenu.getSelectedId();
        audioProcessor.timeSigDenominator = timeSigDenMenu.getSelectedId();
        audioProcessor.globalBarCount = barCountMenu.getSelectedId();
        audioProcessor.patternUpdated = true;
        updateBarSettingsUI(); resized(); seqGrid.repaint();
        };
    timeSigNumMenu.onChange = updateSig; timeSigDenMenu.onChange = updateSig; barCountMenu.onChange = updateSig;
    btnClearAll.onClick = [this] { audioProcessor.clearPattern(); seqGrid.repaint(); };

    for (int i = 0; i < 4; ++i) {
        barTabs[i].setButtonText("BAR " + juce::String(i + 1));
        barTabs[i].onClick = [this, i] { currentViewBar = i; seqGrid.updateView(i); updateBarSettingsUI(); updateTabColors(); grabKeyboardFocus(); };
    }

    auto setupLabel = [](juce::Label& l) {
        l.setJustificationType(juce::Justification::centredLeft); l.setFont(14.0f); l.setColour(juce::Label::textColourId, juce::Colours::lightgrey);
        };
    setupLabel(velLabel); setupLabel(lenLabel); setupLabel(octLabel);
    setupLabel(cmplxLabel); setupLabel(entrpLabel); setupLabel(autoGlideLabel);

    auto setupLinear = [](juce::Slider& s, double min, double max) {
        s.setSliderStyle(juce::Slider::LinearHorizontal); s.setTextBoxStyle(juce::Slider::TextBoxRight, false, 40, 20); s.setRange(min, max, 1);
        };
    setupLinear(velSlider, 1, 127); setupLinear(lenSlider, 1, 96); setupLinear(octSlider, -2, 2);
    setupLinear(cmplxSlider, 0, 100); setupLinear(entrpSlider, 0, 100); setupLinear(autoGlideSlider, 0, 100);

    auto applyInspector = [this] {
        if (seqGrid.selectedTick != -1 && seqGrid.selectedRow != -1) {
            int slot = audioProcessor.currentSlot.load();
            auto& d = audioProcessor.patternUI[slot][seqGrid.selectedRow][seqGrid.selectedTick];
            d.velocity = (int)velSlider.getValue();
            d.length = (int)lenSlider.getValue();
            d.octave = (int)octSlider.getValue();
            audioProcessor.patternUpdated = true;
            seqGrid.repaint();
        }
        };
    velSlider.onValueChange = applyInspector; lenSlider.onValueChange = applyInspector; octSlider.onValueChange = applyInspector;

    glideToggle.setClickingTogglesState(true); staccatoToggle.setClickingTogglesState(true); lockToggle.setClickingTogglesState(true);
    glideToggle.setColour(juce::TextButton::buttonOnColourId, juce::Colours::cyan);
    staccatoToggle.setColour(juce::TextButton::buttonOnColourId, juce::Colours::orange);
    lockToggle.setColour(juce::TextButton::buttonOnColourId, juce::Colours::red);

    glideToggle.onClick = [this] {
        int slot = audioProcessor.currentSlot.load();
        if (seqGrid.selectedTick != -1 && seqGrid.selectedRow != -1) {
            audioProcessor.patternUI[slot][seqGrid.selectedRow][seqGrid.selectedTick].glide = glideToggle.getToggleState();
            audioProcessor.patternUpdated = true; seqGrid.repaint();
        }
        };
    staccatoToggle.onClick = [this] {
        int slot = audioProcessor.currentSlot.load();
        if (seqGrid.selectedTick != -1 && seqGrid.selectedRow != -1) {
            audioProcessor.patternUI[slot][seqGrid.selectedRow][seqGrid.selectedTick].staccato = staccatoToggle.getToggleState();
            audioProcessor.patternUpdated = true; seqGrid.repaint();
        }
        };
    lockToggle.onClick = [this] {
        int slot = audioProcessor.currentSlot.load();
        if (seqGrid.selectedTick != -1 && seqGrid.selectedRow != -1) {
            audioProcessor.patternUI[slot][seqGrid.selectedRow][seqGrid.selectedTick].locked = lockToggle.getToggleState();
            audioProcessor.patternUpdated = true; seqGrid.repaint();
        }
        };

    const char* keys[] = { "C", "C#", "D", "D#", "E", "F", "F#", "G", "G#", "A", "A#", "B" };
    for (int i = 0; i < 12; ++i) barKeyMenu.addItem(keys[i], i + 1);

    const char* scales[40] = {
        "1. Major (Ionian)", "2. Natural Min", "3. Pentatonic Maj", "4. Pentatonic Min", "5. Dorian", "6. Harmonic Min", "7. Lydian", "8. Mixolydian", "9. Phrygian",
        "10. Melodic Min", "11. Minor Blues", "12. Major Blues", "13. Lydian b7", "14. Altered", "15. Locrian", "16. Locrian #2", "17. Bebop Dom",
        "18. Whole Tone", "19. Half-Whole Dim", "20. Whole-Half Dim", "21. Augmented",
        "22. Phrygian Dom", "23. Dbl Harmonic", "24. Hungarian Min", "25. Hungarian Gypsy", "26. Persian", "27. Oriental", "28. Hirajoshi", "29. Insen", "30. Iwato", "31. Kumoi", "32. Pelog",
        "33. Harmonic Maj", "34. Neapolitan Maj", "35. Neapolitan Min", "36. Prometheus", "37. Prom Neapolitan", "38. Lydian Aug", "39. Lydian Min", "40. Enigmatic"
    };
    for (int i = 0; i < 40; ++i) barScaleMenu.addItem(scales[i], i + 1);

    divSelector.addItem("1/8 (12)", 12);
    divSelector.addItem("1/16 (6)", 6);
    divSelector.addItem("1/32 (3)", 3);
    divSelector.addItem("1/16T (4)", 4);

    // ★ ここに追記：さらに細かい解像度を追加
    divSelector.addItem("1/48 (2)", 2); // 16分音符の1/3 (2 Ticks)
    divSelector.addItem("1/96 (1)", 1); // 限界解像度 (1 Tick)

    auto applyBar = [this] {
        int slot = audioProcessor.currentSlot.load();
        auto& bs = audioProcessor.barSettingsUI[slot][currentViewBar];
        bs.key = barKeyMenu.getSelectedId() - 1; bs.scale = barScaleMenu.getSelectedId() - 1; bs.div = divSelector.getSelectedId();

        bs.cmplx = (int)cmplxSlider.getValue();
        bs.entrp = (int)entrpSlider.getValue();
        bs.autoGlide = (int)autoGlideSlider.getValue();

        bs.anchor = anchorToggle.getToggleState();
        audioProcessor.patternUpdated = true;
        seqGrid.repaint(); grabKeyboardFocus();
        };
    barKeyMenu.onChange = applyBar; barScaleMenu.onChange = applyBar; divSelector.onChange = applyBar;
    cmplxSlider.onValueChange = applyBar; entrpSlider.onValueChange = applyBar; autoGlideSlider.onValueChange = applyBar;

    lockCmplx.setClickingTogglesState(true); lockEntrp.setClickingTogglesState(true); lockGlide.setClickingTogglesState(true);
    lockCmplx.setColour(juce::TextButton::buttonOnColourId, juce::Colours::red);
    lockEntrp.setColour(juce::TextButton::buttonOnColourId, juce::Colours::red);
    lockGlide.setColour(juce::TextButton::buttonOnColourId, juce::Colours::red);

    lockCmplx.onClick = [this] { audioProcessor.barSettingsUI[audioProcessor.currentSlot.load()][currentViewBar].lockCmplx = lockCmplx.getToggleState(); };
    lockEntrp.onClick = [this] { audioProcessor.barSettingsUI[audioProcessor.currentSlot.load()][currentViewBar].lockEntrp = lockEntrp.getToggleState(); };
    lockGlide.onClick = [this] { audioProcessor.barSettingsUI[audioProcessor.currentSlot.load()][currentViewBar].lockGlide = lockGlide.getToggleState(); };

    anchorToggle.setClickingTogglesState(true); anchorToggle.setColour(juce::TextButton::buttonOnColourId, juce::Colours::goldenrod); anchorToggle.onClick = applyBar;

    auto applyRange = [this] {
        auto& gs = audioProcessor.genSettings;
        gs.vel.min = velRange.getMinValue(); gs.vel.max = velRange.getMaxValue();
        velRLabel.setText("Velocity [" + juce::String(gs.vel.min) + " - " + juce::String(gs.vel.max) + "]", juce::dontSendNotification);
        gs.len.min = lenRange.getMinValue(); gs.len.max = lenRange.getMaxValue();
        lenRLabel.setText("Length [" + juce::String(gs.len.min) + " - " + juce::String(gs.len.max) + "]", juce::dontSendNotification);
        gs.oct.min = octRange.getMinValue(); gs.oct.max = octRange.getMaxValue();
        octRLabel.setText("Octave [" + juce::String(gs.oct.min) + " - " + juce::String(gs.oct.max) + "]", juce::dontSendNotification);
        gs.cmplx.min = cmplxRange.getMinValue(); gs.cmplx.max = cmplxRange.getMaxValue();
        cmplxRLabel.setText("Cmplx [" + juce::String(gs.cmplx.min) + " - " + juce::String(gs.cmplx.max) + "]", juce::dontSendNotification);
        gs.entrp.min = entrpRange.getMinValue(); gs.entrp.max = entrpRange.getMaxValue();
        entrpRLabel.setText("Entrp [" + juce::String(gs.entrp.min) + " - " + juce::String(gs.entrp.max) + "]", juce::dontSendNotification);
        gs.glide.min = glideRange.getMinValue(); gs.glide.max = glideRange.getMaxValue();
        glideRLabel.setText("Glide% [" + juce::String(gs.glide.min) + " - " + juce::String(gs.glide.max) + "]", juce::dontSendNotification);
        gs.hum.min = humRange.getMinValue(); gs.hum.max = humRange.getMaxValue();
        humRLabel.setText("Humanize [Max +/- " + juce::String(gs.hum.max) + " Ticks]", juce::dontSendNotification);
        };
    velRange.onValueChange = applyRange; lenRange.onValueChange = applyRange; octRange.onValueChange = applyRange;
    cmplxRange.onValueChange = applyRange; entrpRange.onValueChange = applyRange; glideRange.onValueChange = applyRange; humRange.onValueChange = applyRange;

    // 初期ロード時にもUIを同期させる
    int initialGenre = audioProcessor.currentGenre.load();
    if (initialGenre > 0 && initialGenre <= genreRegistry.size()) {
        const auto& dna = genreRegistry[initialGenre - 1];
        audioProcessor.genSettings.vel = dna.defVel;
        audioProcessor.genSettings.len = dna.defLen;
        audioProcessor.genSettings.cmplx = dna.defCmplx;
        audioProcessor.genSettings.entrp = dna.defEntrp;
        audioProcessor.genSettings.glide = dna.defGlide;
        audioProcessor.genSettings.hum.max = dna.humanizeMax;
    }
    applyRange();

    audioProcessor.uiNeedsUpdate = true;
    startTimerHz(30);
}

BassLineMatrixAudioProcessorEditor::~BassLineMatrixAudioProcessorEditor() { stopTimer(); }

void BassLineMatrixAudioProcessorEditor::mouseDown(const juce::MouseEvent& e) {
    for (int i = 0; i < 4; ++i) {
        if (e.originalComponent == &slotBtns[i] && e.mods.isRightButtonDown()) {
            juce::AlertWindow::showAsync(
                juce::MessageBoxOptions().withIconType(juce::MessageBoxIconType::QuestionIcon)
                .withTitle("Clear Pattern").withMessage("Do you want to clear Pattern " + juce::String(i + 1) + "?")
                .withButton("Yes").withButton("No"),
                [this, i](int result) {
                    if (result == 1) { audioProcessor.clearSpecificPattern(i); if (audioProcessor.currentSlot.load() == i) seqGrid.repaint(); }
                }
            );
            return;
        }
    }
}

juce::File BassLineMatrixAudioProcessorEditor::exportBassMidi() {
    juce::MidiMessageSequence seq;
    int slot = audioProcessor.currentSlot.load(); int shift = audioProcessor.globalPitchShift.load();
    int bars = audioProcessor.globalBarCount.load(); int numBeats = audioProcessor.timeSigNumerator.load();
    int totalTicks = bars * numBeats * 24;

    // ==============================================================================
    // ★ 修正ブロック 6: PluginEditor.cpp / exportBassMidi() 内
    // ==============================================================================
    for (int t = 0; t < totalTicks; ++t) {
        for (int r = 0; r < 12; ++r) {
            auto& d = audioProcessor.patternUI[slot][r][t];
            if (d.velocity > 0) {
                // ★ 課題1の修正: globalPitchShift(shift)の重複加算を削除
                int noteNum = juce::jlimit(0, 127, audioProcessor.getMidiNoteFromRow(r, t, d.octave, slot, false));

                // ★ ここで t に offset を加算する
                double startTick = ((double)t + (double)d.offset) * 40.0;
                double gateRatio = d.glide ? 1.0 : (d.staccato ? audioProcessor.staccatoRatio.load() : 0.85);
                seq.addEvent(juce::MidiMessage::noteOn(1, noteNum, (juce::uint8)d.velocity), startTick);
                seq.addEvent(juce::MidiMessage::noteOff(1, noteNum), startTick + (d.length * 40.0 * gateRatio));
            }
        }
    }
    seq.updateMatchedPairs(); juce::MidiFile mf; mf.setTicksPerQuarterNote(960); mf.addTrack(seq);
    juce::File f = juce::File::getSpecialLocation(juce::File::tempDirectory).getChildFile("BassLineMatrix_Bass.mid");
    f.deleteFile(); juce::FileOutputStream out(f); mf.writeTo(out); return f;
}

// ==============================================================================
// 修正ブロック: PluginEditor.cpp / exportChordMidi() 内の和音生成ロジック
// ==============================================================================

juce::File BassLineMatrixAudioProcessorEditor::exportChordMidi() {
    juce::MidiMessageSequence seq;
    int slot = audioProcessor.currentSlot.load(); int shift = audioProcessor.globalPitchShift.load();
    int bars = audioProcessor.globalBarCount.load(); int numBeats = audioProcessor.timeSigNumerator.load();
    int totalTicks = bars * numBeats * 24;

    for (int t = 0; t < totalTicks; t += 24) {
        int barIndex = t / (numBeats * 24); int beatInBar = (t % (numBeats * 24)) / 24;
        const auto& bs = audioProcessor.barSettingsUI[slot][barIndex];

        if (bs.useCodeMode) {
            ChordDef chord = bs.chords[beatInBar];

            // ルート音の算出 (現状はダイアトニック度数 0〜6 に依存)
            int rootMidi = 60 + bs.key + (std::array<int, 7>{0, 2, 4, 5, 7, 9, 11})[chord.degree % 7] + shift + (audioProcessor.chordOctave.load() * 12);

            // ★ 修正：最大5和音(9th, 13th等)に対応できるよう配列を拡張
            int notes[5] = { rootMidi, 0, 0, 0, 0 };
            int activeVoices = 3;

            // ★ 修正：15種類すべてのChordQualityに対応した正確なインターバル定義
            switch (chord.quality) {
            case ChordQuality::Major:
                notes[1] = rootMidi + 4; notes[2] = rootMidi + 7;
                activeVoices = 3; break;
            case ChordQuality::Minor:
                notes[1] = rootMidi + 3; notes[2] = rootMidi + 7;
                activeVoices = 3; break;
            case ChordQuality::Dom7:
                notes[1] = rootMidi + 4; notes[2] = rootMidi + 7; notes[3] = rootMidi + 10;
                activeVoices = 4; break;
            case ChordQuality::Min7:
                notes[1] = rootMidi + 3; notes[2] = rootMidi + 7; notes[3] = rootMidi + 10;
                activeVoices = 4; break;
            case ChordQuality::Maj7:
                notes[1] = rootMidi + 4; notes[2] = rootMidi + 7; notes[3] = rootMidi + 11;
                activeVoices = 4; break;
            case ChordQuality::Dim:
                notes[1] = rootMidi + 3; notes[2] = rootMidi + 6;
                activeVoices = 3; break;
            case ChordQuality::HalfDim:
                notes[1] = rootMidi + 3; notes[2] = rootMidi + 6; notes[3] = rootMidi + 10;
                activeVoices = 4; break;
            case ChordQuality::Dim7:
                notes[1] = rootMidi + 3; notes[2] = rootMidi + 6; notes[3] = rootMidi + 9;
                activeVoices = 4; break;
            case ChordQuality::Power:
                notes[1] = rootMidi + 7; notes[2] = rootMidi + 12; // Root, 5th, Octave
                activeVoices = 3; break;
            case ChordQuality::Min9:
                notes[1] = rootMidi + 3; notes[2] = rootMidi + 7; notes[3] = rootMidi + 10; notes[4] = rootMidi + 14; // m3, 5, m7, 9
                activeVoices = 5; break;
            case ChordQuality::Maj9:
                notes[1] = rootMidi + 4; notes[2] = rootMidi + 7; notes[3] = rootMidi + 11; notes[4] = rootMidi + 14; // M3, 5, M7, 9
                activeVoices = 5; break;
            case ChordQuality::Dom7b9:
                notes[1] = rootMidi + 4; notes[2] = rootMidi + 7; notes[3] = rootMidi + 10; notes[4] = rootMidi + 13; // M3, 5, m7, b9
                activeVoices = 5; break;
            case ChordQuality::Dom7alt:
                notes[1] = rootMidi + 4; notes[2] = rootMidi + 6; notes[3] = rootMidi + 10; notes[4] = rootMidi + 15; // M3, b5, m7, #9
                activeVoices = 5; break;
            case ChordQuality::Dom13:
                notes[1] = rootMidi + 4; notes[2] = rootMidi + 10; notes[3] = rootMidi + 14; notes[4] = rootMidi + 21; // M3, m7, 9, 13 (5thは濁り回避で省略)
                activeVoices = 5; break;
            case ChordQuality::Aug:
                notes[1] = rootMidi + 4; notes[2] = rootMidi + 8; // M3, #5
                activeVoices = 3; break;
            default:
                notes[1] = rootMidi + 4; notes[2] = rootMidi + 7; // フォールバック (Major)
                activeVoices = 3; break;
            }

            // インバージョンの適用 (展開形)
            for (int inv = 0; inv < chord.inversion; ++inv) {
                notes[inv % activeVoices] += 12;
            }

            // MIDIイベントの書き込み
            double startTick = (double)t * 40.0;
            double endTick = startTick + (24.0 * 40.0 * 0.9);

            for (int v = 0; v < activeVoices; ++v) {
                int note = juce::jlimit(0, 127, notes[v]);
                seq.addEvent(juce::MidiMessage::noteOn(1, note, (juce::uint8)80), startTick);
                seq.addEvent(juce::MidiMessage::noteOff(1, note), endTick);
            }
        }
    }

    seq.updateMatchedPairs(); juce::MidiFile mf; mf.setTicksPerQuarterNote(960); mf.addTrack(seq);
    juce::File f = juce::File::getSpecialLocation(juce::File::tempDirectory).getChildFile("BassLineMatrix_Chord.mid");
    f.deleteFile(); juce::FileOutputStream out(f); mf.writeTo(out); return f;
}
bool BassLineMatrixAudioProcessorEditor::keyPressed(const juce::KeyPress& key) {
    if (key.getTextCharacter() == 'l' || key.getTextCharacter() == 'L') {
        if (seqGrid.selectedTick != -1 && seqGrid.selectedRow != -1) {
            int slot = audioProcessor.currentSlot.load();
            auto& d = audioProcessor.patternUI[slot][seqGrid.selectedRow][seqGrid.selectedTick];
            if (d.velocity > 0) { d.locked = !d.locked; audioProcessor.patternUpdated = true; seqGrid.repaint(); updateInspectorValues(); }
        }
        return true;
    }
    return false;
}

void BassLineMatrixAudioProcessorEditor::updateTabColors() {
    for (int i = 0; i < 4; ++i) barTabs[i].setColour(juce::TextButton::buttonColourId, (i == currentViewBar) ? juce::Colours::orange : juce::Colours::darkgrey);
    int slot = audioProcessor.currentSlot.load();
    for (int i = 0; i < 4; ++i) slotBtns[i].setColour(juce::TextButton::buttonColourId, (i == slot) ? juce::Colours::cyan.darker() : juce::Colours::darkgrey);
}

void BassLineMatrixAudioProcessorEditor::updateBarSettingsUI() {
    int slot = audioProcessor.currentSlot.load();
    auto& bs = audioProcessor.barSettingsUI[slot][currentViewBar];

    // 基本設定の同期
    barKeyMenu.setSelectedId(bs.key + 1, juce::dontSendNotification);
    barScaleMenu.setSelectedId(bs.scale + 1, juce::dontSendNotification);
    divSelector.setSelectedId(bs.div, juce::dontSendNotification);
    cmplxSlider.setValue(bs.cmplx, juce::dontSendNotification);
    entrpSlider.setValue(bs.entrp, juce::dontSendNotification);
    autoGlideSlider.setValue(bs.autoGlide, juce::dontSendNotification);
    anchorToggle.setToggleState(bs.anchor, juce::dontSendNotification);
    lockCmplx.setToggleState(bs.lockCmplx, juce::dontSendNotification);
    lockEntrp.setToggleState(bs.lockEntrp, juce::dontSendNotification);
    lockGlide.setToggleState(bs.lockGlide, juce::dontSendNotification);

    modeToggle.setToggleState(bs.useCodeMode, juce::dontSendNotification);
    modeToggle.setButtonText(bs.useCodeMode ? "MODE: CHORD" : "MODE: SCALE");
    chordLockBtn.setToggleState(bs.lockChords, juce::dontSendNotification);

    // --- コード/インバージョンメニューの表示制御 ---
    int numBeats = audioProcessor.timeSigNumerator.load();
    // 設定画面が開いていない時だけコードメニューを表示
    bool isMainView = !isSettingsView && !isSettings2View;

    for (int i = 0; i < 16; ++i) {
        bool beatInRange = (i < numBeats);

        // メイン画面かつCHORDモードなら表示
        chordMenus[i].setVisible(bs.useCodeMode && beatInRange && isMainView);
        // 設定2画面（インバージョン設定）かつCHORDモードなら表示
        inversionMenus[i].setVisible(bs.useCodeMode && beatInRange && isSettings2View);

        // --- updateBarSettingsUI 関数内の該当箇所を修正 ---
        if (bs.useCodeMode && beatInRange) {
            // ★ 7 ではなく 15 を使う（これがズレの最大の原因です）
            int chordID = static_cast<int>(bs.chords[i].degree) * 15 + static_cast<int>(bs.chords[i].quality) + 1;
            chordMenus[i].setSelectedId(chordID, juce::dontSendNotification);
            inversionMenus[i].setSelectedId(bs.chords[i].inversion + 1, juce::dontSendNotification);
        }
    }

    barScaleMenu.setEnabled(!bs.useCodeMode);

    // 表示状態が変わった可能性が高いため、最後に再配置を強制
    resized();
}
void BassLineMatrixAudioProcessorEditor::updateRangeSettingsUI() {
    auto& gs = audioProcessor.genSettings;
    velRange.setMinAndMaxValues(gs.vel.min, gs.vel.max, juce::dontSendNotification); lenRange.setMinAndMaxValues(gs.len.min, gs.len.max, juce::dontSendNotification);
    octRange.setMinAndMaxValues(gs.oct.min, gs.oct.max, juce::dontSendNotification); cmplxRange.setMinAndMaxValues(gs.cmplx.min, gs.cmplx.max, juce::dontSendNotification);
    entrpRange.setMinAndMaxValues(gs.entrp.min, gs.entrp.max, juce::dontSendNotification); glideRange.setMinAndMaxValues(gs.glide.min, gs.glide.max, juce::dontSendNotification);
    humRange.setMinAndMaxValues(gs.hum.min, gs.hum.max, juce::dontSendNotification);
    velRLabel.setText("Velocity [" + juce::String(gs.vel.min) + " - " + juce::String(gs.vel.max) + "]", juce::dontSendNotification);
    lenRLabel.setText("Length [" + juce::String(gs.len.min) + " - " + juce::String(gs.len.max) + "]", juce::dontSendNotification);
    octRLabel.setText("Octave [" + juce::String(gs.oct.min) + " - " + juce::String(gs.oct.max) + "]", juce::dontSendNotification);
    cmplxRLabel.setText("Cmplx [" + juce::String(gs.cmplx.min) + " - " + juce::String(gs.cmplx.max) + "]", juce::dontSendNotification);
    entrpRLabel.setText("Entrp [" + juce::String(gs.entrp.min) + " - " + juce::String(gs.entrp.max) + "]", juce::dontSendNotification);
    glideRLabel.setText("Glide% [" + juce::String(gs.glide.min) + " - " + juce::String(gs.glide.max) + "]", juce::dontSendNotification);
    humRLabel.setText("Humanize [Max +/- " + juce::String(gs.hum.max) + " Ticks]", juce::dontSendNotification);
    // ★ ここに追記：Staccatoスライダーの表示を現在の値（85等）に合わせる
    staccatoRatioSlider.setValue(audioProcessor.staccatoRatio.load() * 100.0f, juce::dontSendNotification);
   // ★ テンポ表示を更新
    if (audioProcessor.isSyncEnabled.load()) {
        tempoLabel.setText("DAW BPM", juce::dontSendNotification);
    }
    else {
        // 一時的にエディット判定を無視して最新の数値をセット
        tempoLabel.setText(juce::String(audioProcessor.internalTempo.load(), 1) + " BPM", juce::dontSendNotification);
    }

    // ★ ここから挿入：スライダー横のテキストラベルを現在の数値に書き換える
    velRLabel.setText("Velocity [" + juce::String(gs.vel.min) + " - " + juce::String(gs.vel.max) + "]", juce::dontSendNotification);
    lenRLabel.setText("Length [" + juce::String(gs.len.min) + " - " + juce::String(gs.len.max) + "]", juce::dontSendNotification);
    octRLabel.setText("Octave [" + juce::String(gs.oct.min) + " - " + juce::String(gs.oct.max) + "]", juce::dontSendNotification);
    cmplxRLabel.setText("Cmplx [" + juce::String(gs.cmplx.min) + " - " + juce::String(gs.cmplx.max) + "]", juce::dontSendNotification);
    entrpRLabel.setText("Entrp [" + juce::String(gs.entrp.min) + " - " + juce::String(gs.entrp.max) + "]", juce::dontSendNotification);
    glideRLabel.setText("Glide% [" + juce::String(gs.glide.min) + " - " + juce::String(gs.glide.max) + "]", juce::dontSendNotification);
    humRLabel.setText("Humanize [Max +/- " + juce::String(gs.hum.max) + " Ticks]", juce::dontSendNotification);
}

// --- PluginEditor.cpp の updateInspectorValues() を以下に上書き ---

void BassLineMatrixAudioProcessorEditor::updateInspectorValues() {
    if (seqGrid.selectedTick != -1 && seqGrid.selectedRow != -1) {
        int slot = audioProcessor.currentSlot.load();
        const auto& d = audioProcessor.patternUI[slot][seqGrid.selectedRow][seqGrid.selectedTick];

        velSlider.setValue(d.velocity, juce::dontSendNotification);
        lenSlider.setValue(d.length, juce::dontSendNotification);
        octSlider.setValue(d.octave, juce::dontSendNotification);
        glideToggle.setToggleState(d.glide, juce::dontSendNotification);
        staccatoToggle.setToggleState(d.staccato, juce::dontSendNotification);
        lockToggle.setToggleState(d.locked, juce::dontSendNotification);

        // ==============================================================================
        // ★ 修正ブロック 7: PluginEditor.cpp / updateInspectorValues() 内
        // ==============================================================================
                // ==========================================================
                // ★ 追加：音名の再計算と表示更新
                // ==========================================================
                // 現在のノート番号を、オクターブ設定と全体ピッチシフトを含めて取得
                // ★ 課題1の修正: globalPitchShiftの重複加算を削除
        int noteNum = audioProcessor.getMidiNoteFromRow(seqGrid.selectedRow, seqGrid.selectedTick, d.octave, slot, false);

        // 0〜127の範囲内に安全に収める
        noteNum = juce::jlimit(0, 127, noteNum);
        // JUCEの機能を使ってMIDIノート番号を「C3」などの文字列に変換（真ん中のCを3とする）
        juce::String nameStr = juce::MidiMessage::getMidiNoteName(noteNum, true, true, 3);
        noteNameLabel.setText(nameStr, juce::dontSendNotification);
        // ==========================================================
    }
    else {
        // 何も選択されていない時は "--" に戻す
        noteNameLabel.setText("--", juce::dontSendNotification);
    }
}

void BassLineMatrixAudioProcessorEditor::timerCallback() {
    // 1. テンポ表示の更新
    if (audioProcessor.isSyncEnabled.load()) {
        if (tempoLabel.getText() != "DAW BPM")
            tempoLabel.setText("DAW BPM", juce::dontSendNotification);
    }
    else if (!tempoLabel.isBeingEdited()) {
        tempoLabel.setText(juce::String(audioProcessor.internalTempo.load(), 1) + " BPM", juce::dontSendNotification);
    }

    // 2. フラグに基づいた一括更新 (ここが重要です)
    if (audioProcessor.uiNeedsUpdate.exchange(false)) {
        // ジャンルメニューの同期
        genreMenu.setSelectedId(audioProcessor.currentGenre.load(), juce::dontSendNotification);

        // ★ 追加：TimeSig（拍子）と Bars（小節数）の表示を内部数値と同期
        timeSigNumMenu.setSelectedId(audioProcessor.timeSigNumerator.load(), juce::dontSendNotification);
        timeSigDenMenu.setSelectedId(audioProcessor.timeSigDenominator.load(), juce::dontSendNotification);
        barCountMenu.setSelectedId(audioProcessor.globalBarCount.load(), juce::dontSendNotification);

        // その他のスライダーやパネルの更新
        updateRangeSettingsUI();
        updateBarSettingsUI();
        updateTabColors();
    }
    // ★ ここに挿入 ★
    lockStaccatoBtn.setToggleState(audioProcessor.isStaccatoLocked.load(), juce::dontSendNotification);

    // 3. インスペクターの数値更新
    updateInspectorValues();

    // 4. 再生位置の追従とグリッドの再描画
    if (audioProcessor.isPlayingInternal.load() || audioProcessor.isSyncEnabled.load()) {
        int playingBar = audioProcessor.currentPlayingBar.load();
        if (followBtn.getToggleState() && playingBar != currentViewBar && playingBar >= 0 && playingBar < 8) {
            currentViewBar = playingBar;
            seqGrid.updateView(currentViewBar);
            updateBarSettingsUI();
            updateTabColors();
        }
        seqGrid.repaint();
    }
}

void BassLineMatrixAudioProcessorEditor::resized() {
    auto area = getLocalBounds().reduced(15);

    auto r1 = area.removeFromTop(40);
    syncButton.setBounds(r1.removeFromLeft(60).reduced(2)); playButton.setBounds(r1.removeFromLeft(60).reduced(2)); stopButton.setBounds(r1.removeFromLeft(60).reduced(2));
    r1.removeFromLeft(5); followBtn.setBounds(r1.removeFromLeft(70).reduced(2));

    r1.removeFromLeft(10); tempoLabel.setBounds(r1.removeFromLeft(65).reduced(2));
    tempoLockBtn.setBounds(r1.removeFromLeft(20).reduced(0, 8));

    r1.removeFromLeft(10); timeSigLabel.setBounds(r1.removeFromLeft(60).reduced(2)); timeSigNumMenu.setBounds(r1.removeFromLeft(60).reduced(4)); timeSigSlash.setBounds(r1.removeFromLeft(15).reduced(2)); timeSigDenMenu.setBounds(r1.removeFromLeft(60).reduced(4));

    r1.removeFromLeft(10); barCountLabel.setBounds(r1.removeFromLeft(40).reduced(2)); barCountMenu.setBounds(r1.removeFromLeft(100).reduced(4));
    r1.removeFromLeft(10); shiftLabel.setBounds(r1.removeFromLeft(40).reduced(2)); shiftSlider.setBounds(r1.removeFromLeft(60).reduced(2));

    btnClearAll.setBounds(r1.removeFromRight(80).reduced(2)); chordMidiBtn.setBounds(r1.removeFromRight(100).reduced(2)); bassMidiBtn.setBounds(r1.removeFromRight(90).reduced(2));

    area.removeFromTop(10);
    auto r2 = area.removeFromTop(35);
    for (int i = 0; i < 4; ++i) barTabs[i].setBounds(r2.removeFromLeft(80).reduced(2));
    r2.removeFromLeft(20);
    for (int i = 0; i < 4; ++i) slotBtns[i].setBounds(r2.removeFromLeft(50).reduced(2));

    settingsBtn.setBounds(r2.removeFromLeft(80).reduced(4));
    settings2Btn.setBounds(r2.removeFromLeft(90).reduced(4));

    // ★ 追加: SETTING 2 の右隣に配置
    globalLockBtn.setBounds(r2.removeFromLeft(80).reduced(4));

    generateButton.setBounds(r2.removeFromRight(180).reduced(2)); genreMenu.setBounds(r2.removeFromRight(150).reduced(2));
    auto bottomArea = area;
    auto bottom = bottomArea.removeFromBottom(180);
    area.removeFromBottom(190);

    auto rChord = area.removeFromTop(30);
    chordLockBtn.setBounds(rChord.removeFromLeft(50).reduced(2));
    auto chordGridArea = rChord;
    int numBeats = juce::jmax(1, audioProcessor.timeSigNumerator.load()); float pxPerBeat = chordGridArea.getWidth() / (float)numBeats;

    for (int i = 0; i < 16; ++i) {
        if (i < numBeats && !isSettingsView && !isSettings2View) chordMenus[i].setBounds((int)(chordGridArea.getX() + i * pxPerBeat + 1), rChord.getY() + 2, (int)(pxPerBeat - 2), rChord.getHeight() - 4);
        else chordMenus[i].setBounds(0, 0, 0, 0);
    }

    inspectorGroup.setBounds(bottom.removeFromLeft(550).reduced(5));
    auto iArea = inspectorGroup.getBounds().reduced(15); iArea.removeFromTop(15);

    auto iSliders = iArea.removeFromLeft(200);
    auto vRow = iSliders.removeFromTop(iSliders.getHeight() / 3); velLabel.setBounds(vRow.removeFromLeft(60)); velSlider.setBounds(vRow.reduced(2));
    auto lRow = iSliders.removeFromTop(iSliders.getHeight() / 2); lenLabel.setBounds(lRow.removeFromLeft(60)); lenSlider.setBounds(lRow.reduced(2));
    auto oRow = iSliders; octLabel.setBounds(oRow.removeFromLeft(60)); octSlider.setBounds(oRow.reduced(2));

    auto iToggles = iArea.removeFromLeft(100).reduced(10, 0);
    glideToggle.setBounds(iToggles.removeFromTop(iToggles.getHeight() / 3).reduced(0, 2));
    staccatoToggle.setBounds(iToggles.removeFromTop(iToggles.getHeight() / 2).reduced(0, 2));
    lockToggle.setBounds(iToggles.reduced(0, 2));

    noteNameLabel.setBounds(iArea.reduced(20));

    barSettingsGroup.setBounds(bottom.reduced(5));
    auto bArea = barSettingsGroup.getBounds().reduced(15); bArea.removeFromTop(15);

    auto bCol1 = bArea.removeFromLeft(130);
    auto bkRow = bCol1.removeFromTop(bCol1.getHeight() / 3); barKeyLabel.setBounds(bkRow.removeFromLeft(40)); barKeyMenu.setBounds(bkRow.reduced(2));
    auto bsRow = bCol1.removeFromTop(bCol1.getHeight() / 2); barScaleLabel.setBounds(bsRow.removeFromLeft(40)); barScaleMenu.setBounds(bsRow.reduced(2));
    auto bdRow = bCol1; barDivLabel.setBounds(bdRow.removeFromLeft(40)); divSelector.setBounds(bdRow.reduced(2));

    auto bCol2 = bArea.removeFromLeft(110).reduced(5, 0);
    modeToggle.setBounds(bCol2.removeFromTop(bCol2.getHeight() / 2).reduced(0, 2)); anchorToggle.setBounds(bCol2.reduced(0, 2));

    auto bCol3 = bArea.reduced(5, 0);
    auto cRow = bCol3.removeFromTop(bCol3.getHeight() / 3); cmplxLabel.setBounds(cRow.removeFromLeft(45)); lockCmplx.setBounds(cRow.removeFromRight(30).reduced(2)); cmplxSlider.setBounds(cRow.reduced(2));
    auto eRow = bCol3.removeFromTop(bCol3.getHeight() / 2); entrpLabel.setBounds(eRow.removeFromLeft(45)); lockEntrp.setBounds(eRow.removeFromRight(30).reduced(2)); entrpSlider.setBounds(eRow.reduced(2));
    auto aRow = bCol3; autoGlideLabel.setBounds(aRow.removeFromLeft(45)); lockGlide.setBounds(aRow.removeFromRight(30).reduced(2)); autoGlideSlider.setBounds(aRow.reduced(2));

    if (isSettingsView) {
        settingsPanel.setBounds(area);
        auto sArea = settingsPanel.getLocalBounds().reduced(20); sArea.removeFromTop(10);
        auto setupRangeRow = [&sArea](juce::Label& l, juce::Slider& s) { auto row = sArea.removeFromTop(38); l.setBounds(row.removeFromLeft(160)); s.setBounds(row.reduced(10, 2)); };
        setupRangeRow(velRLabel, velRange); setupRangeRow(lenRLabel, lenRange); setupRangeRow(octRLabel, octRange); setupRangeRow(cmplxRLabel, cmplxRange); setupRangeRow(entrpRLabel, entrpRange); setupRangeRow(glideRLabel, glideRange); setupRangeRow(humRLabel, humRange);
        sArea.removeFromTop(10); closeSettingsBtn.setBounds(sArea.removeFromTop(30).withSizeKeepingCentre(150, 30));
    }
    else if (isSettings2View) {
        settings2Panel.setBounds(area);
        auto s2Area = settings2Panel.getLocalBounds().reduced(20); s2Area.removeFromTop(20);
        // ★ 追加：右側の250ピクセルを推奨スケール表示エリアとして切り取る
        auto recArea = s2Area.removeFromRight(250);
        recommendedScaleLabel.setBounds(recArea.reduced(10, 0));

        auto chordRow = s2Area.removeFromTop(45);
        chordOnBtn.setBounds(chordRow.removeFromLeft(100).reduced(5)); chordSoundMenu.setBounds(chordRow.removeFromLeft(140).reduced(5, 10));
        chordVolLabel.setBounds(chordRow.removeFromLeft(30).reduced(0, 10)); chordVolSlider.setBounds(chordRow.removeFromLeft(80).reduced(0, 10));
        chordTriggerMenu.setBounds(chordRow.removeFromLeft(150).reduced(5, 10));
        chordOctLabel.setBounds(chordRow.removeFromLeft(50).reduced(0, 10)); chordOctSlider.setBounds(chordRow.removeFromLeft(60).reduced(0, 10));

        auto bassRow = s2Area.removeFromTop(45);
        bassOnBtn.setBounds(bassRow.removeFromLeft(100).reduced(5)); bassSoundMenu.setBounds(bassRow.removeFromLeft(140).reduced(5, 10));
        bassVolLabel.setBounds(bassRow.removeFromLeft(30).reduced(0, 10)); bassVolSlider.setBounds(bassRow.removeFromLeft(80).reduced(0, 10));

        auto staccatoRow = s2Area.removeFromTop(45);
        staccatoRatioLabel.setBounds(staccatoRow.removeFromLeft(150).reduced(5));
        // Lボタンをスライダーの左か右に配置（ここでは右側に配置します）
        lockStaccatoBtn.setBounds(staccatoRow.removeFromRight(40).reduced(2, 10));
        staccatoRatioSlider.setBounds(staccatoRow.reduced(5, 10));

        s2Area.removeFromTop(20);
        inversionTitleLabel.setBounds(s2Area.removeFromTop(30));
        auto invRow = s2Area.removeFromTop(30);
        float invWidth = invRow.getWidth() / (float)numBeats;

        for (int i = 0; i < 16; ++i) {
            if (i < numBeats) inversionMenus[i].setBounds((int)(invRow.getX() + i * invWidth + 2), invRow.getY(), (int)(invWidth - 4), invRow.getHeight());
            else inversionMenus[i].setBounds(0, 0, 0, 0);
        }

        s2Area.removeFromTop(20);
        closeSettings2Btn.setBounds(s2Area.removeFromTop(30).withSizeKeepingCentre(150, 30));
    }
    else {
        seqGrid.setBounds(area);
        auto gridLocal = seqGrid.getLocalBounds(); seqGrid.labelArea = gridLocal.removeFromLeft(50); seqGrid.mainGridArea = gridLocal;
    }
}

void BassLineMatrixAudioProcessorEditor::paint(juce::Graphics& g) {
    g.fillAll(juce::Colours::darkgrey.darker());
}