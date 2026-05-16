// ==============================================================================
// Source/PluginProcessor.h
// ==============================================================================
#pragma once
#include <juce_gui_basics/juce_gui_basics.h>
#include <juce_audio_processors/juce_audio_processors.h>
#include <juce_dsp/juce_dsp.h>
#include <atomic>
#include <array>
#include <cstring>
#include "GenreAlgorithms.h"

extern const int scalePatterns[40][8];
extern const int scaleLengths[40];
extern const int scaleOutNotes[40][4];

class BassVoice {
public:
    BassVoice() {
        currentFreq.setCurrentAndTargetValue(440.0f);
        smoothedVelocity.setCurrentAndTargetValue(0.0f);
        setSoundType(0);
    }

    void setSampleRate(double sr) {
        sampleRate = sr;
        osc.prepare({ sr, 128, 1 });
        filter.prepare({ sr, 128, 1 });
        filter.setType(juce::dsp::StateVariableTPTFilterType::lowpass);
        filter.setResonance(1.8f);

        adsr.setSampleRate(sr);
        updateEnvelope();
        currentFreq.reset(sampleRate, 0.005);
        smoothedVelocity.reset(sampleRate, 0.005);
    }

    void setSoundType(int type) {
        currentType = type;
        if (type == 1) {
            osc.initialise([](float x) { return x / juce::MathConstants<float>::pi; });
        }
        else if (type == 2) {
            osc.initialise([](float x) { return x < 0.0f ? -1.0f : 1.0f; });
        }
        else if (type == 3) {
            osc.initialise([](float x) { return (std::sin(x) + 0.2f * std::sin(2.0f * x) + 0.05f * std::sin(3.0f * x)) * 0.9f; });
        }
        else {
            osc.initialise([](float x) { return std::sin(x) * 0.8f + (x / juce::MathConstants<float>::pi) * 0.2f; });
        }
        updateEnvelope();
    }

    void setStaccatoRatio(float ratio) {
        staccatoRatio = juce::jlimit(0.1f, 0.9f, ratio);
    }

    void setGlideTime(double ms) {
        glideTimeSec = ms / 1000.0;
    }

    void trigger(float frequency, float velocity, bool isGlide) {
        if (isGlide) {
            currentFreq.reset(sampleRate, glideTimeSec);
        }
        else {
            currentFreq.reset(sampleRate, 0.005);
        }
        currentFreq.setTargetValue(frequency);
        smoothedVelocity.setTargetValue(velocity / 127.0f);
        adsr.noteOn();
    }

    void release() {
        adsr.noteOff();
    }

    float process() {
        if (!adsr.isActive()) { currentEnv = 0.0f; return 0.0f; }

        float freq = currentFreq.getNextValue();
        float currentVel = smoothedVelocity.getNextValue();

        osc.setFrequency(freq);
        float sig = osc.processSample(0.0f);
        currentEnv = adsr.getNextSample();

        float cutoff = freq;
        if (currentType == 3) {
            cutoff += (currentEnv * 1500.0f * currentVel);
        }
        else {
            cutoff += (currentEnv * 5000.0f * currentVel);
        }
        filter.setCutoffFrequency(juce::jlimit(20.0f, 20000.0f, cutoff));

        return filter.processSample(0, sig) * currentEnv * currentVel;
    }

    float getStaccatoRatio() const { return staccatoRatio; }

private:
    void updateEnvelope() {
        juce::ADSR::Parameters params;
        if (currentType == 3) {
            params.attack = 0.005f;
            params.decay = 0.2f;
            params.sustain = 0.05f;
            params.release = 0.1f;
        }
        else {
            params.attack = 0.005f;
            params.decay = 0.3f;
            params.sustain = 0.2f;
            params.release = 0.05f;
        }
        adsr.setParameters(params);
    }

    double sampleRate = 48000.0;
    double glideTimeSec = 0.06;
    int currentType = 0;
    float currentEnv = 0.0f;
    juce::dsp::Oscillator<float> osc;
    juce::dsp::StateVariableTPTFilter<float> filter;
    juce::ADSR adsr;
    juce::SmoothedValue<float, juce::ValueSmoothingTypes::Multiplicative> currentFreq;
    juce::SmoothedValue<float, juce::ValueSmoothingTypes::Linear> smoothedVelocity;
    float staccatoRatio = 0.3f;
};

class ChordVoice {
public:
    ChordVoice() {
        currentFreq.setCurrentAndTargetValue(440.0f);
        setSoundType(0);
    }

    void setSampleRate(double sr) {
        sampleRate = sr;
        osc.prepare({ sr, 128, 1 });
        filter.prepare({ sr, 128, 1 });
        filter.setType(juce::dsp::StateVariableTPTFilterType::lowpass);
        filter.setResonance(1.1f);
        adsr.setSampleRate(sr);
        currentFreq.reset(sampleRate, 0.01);
        updateEnvelope();
    }

    void setSoundType(int type) {
        currentType = type;
        if (type % 3 == 1) {
            osc.initialise([](float x) { return x / juce::MathConstants<float>::pi; });
        }
        else if (type % 3 == 2) {
            osc.initialise([](float x) { return x < 0.0f ? -1.0f : 1.0f; });
        }
        else {
            osc.initialise([](float x) { return std::sin(x); });
        }
        updateEnvelope();
    }

    void trigger(float frequency) {
        currentFreq.reset(sampleRate, 0.01);
        currentFreq.setTargetValue(frequency);
        adsr.noteOn();
    }

    void release() {
        adsr.noteOff();
    }

    float process() {
        if (!adsr.isActive()) { currentEnv = 0.0f; return 0.0f; }
        float freq = currentFreq.getNextValue();
        osc.setFrequency(freq);
        float sig = osc.processSample(0.0f);
        currentEnv = adsr.getNextSample();
        float cutoff = freq + (currentType < 3 ? (currentEnv * 4000.0f) : (currentEnv * 800.0f));
        filter.setCutoffFrequency(juce::jlimit(20.0f, 20000.0f, cutoff));
        return filter.processSample(0, sig) * currentEnv * 0.15f;
    }

private:
    void updateEnvelope() {
        juce::ADSR::Parameters params;
        if (currentType < 3) {
            params.attack = 0.01f; params.decay = 0.3f; params.sustain = 0.0f; params.release = 0.15f;
        }
        else {
            params.attack = 0.2f; params.decay = 1.0f; params.sustain = 0.8f; params.release = 0.6f;
        }
        adsr.setParameters(params);
    }

    double sampleRate = 48000.0;
    int currentType = 0;
    float currentEnv = 0.0f;
    juce::dsp::Oscillator<float> osc;
    juce::dsp::StateVariableTPTFilter<float> filter;
    juce::SmoothedValue<float, juce::ValueSmoothingTypes::Multiplicative> currentFreq;
    juce::ADSR adsr;
};

// --- PluginProcessor.h 内 ---
// PluginProcessor.h 内を確認してください
struct StepData {
    int velocity = 0; int length = 6; int octave = 0;
    bool glide = false; bool staccato = false; bool locked = false;
    int offset = 0; // ★ これがあることを確認してください！
};
// --- PluginProcessor.h 内の BarSetting 構造体を差し替え ---
struct BarSetting {
    int key = 0; int scale = 1; bool anchor = true; int div = 6;
    int cmplx = 50; int entrp = 15; int autoGlide = 10;
    int patternIndex = 0; // ★これが必要：どのコード進行(0-15)を選択しているか
    bool lockCmplx = false; bool lockEntrp = false; bool lockGlide = false;
    bool useCodeMode = false; bool lockChords = false;
    std::array<ChordDef, 16> chords;
};
// ★ RangeSetting は GenreAlgorithms.h で定義されているため、ここからは削除済

// --- PluginProcessor.h 内 ---
struct GlobalSettings {
    RangeSetting vel{ 40, 120 };
    RangeSetting len{ 2, 8 };
    RangeSetting oct{ 0, 1 };
    RangeSetting cmplx{ 40, 60 };
    RangeSetting entrp{ 10, 20 };
    RangeSetting glide{ 0, 15 };
    RangeSetting hum{ 0, 4 };
};
class BassLineMatrixAudioProcessor : public juce::AudioProcessor {
public:
    BassLineMatrixAudioProcessor();
    ~BassLineMatrixAudioProcessor() override;

    void prepareToPlay(double sampleRate, int samplesPerBlock) override;
    void releaseResources() override;
    bool isBusesLayoutSupported(const BusesLayout& layouts) const override;
    void processBlock(juce::AudioBuffer<float>&, juce::MidiBuffer&) override;

    juce::AudioProcessorEditor* createEditor() override;
    bool hasEditor() const override;
    const juce::String getName() const override;
    bool acceptsMidi() const override; bool producesMidi() const override; bool isMidiEffect() const override;
    double getTailLengthSeconds() const override; int getNumPrograms() override;
    int getCurrentProgram() override; void setCurrentProgram(int index) override;
    const juce::String getProgramName(int index) override; void changeProgramName(int index, const juce::String& newName) override;
    void getStateInformation(juce::MemoryBlock& destData) override;
    void setStateInformation(const void* data, int sizeInBytes) override;

    std::atomic<int> timeSigNumerator{ 4 }; std::atomic<int> timeSigDenominator{ 4 };
    std::atomic<int> globalBarCount{ 4 }; std::atomic<double> internalTempo{ 120.0 };
    std::atomic<bool> isTempoLocked{ false }; std::atomic<bool> isSyncEnabled{ false };
    std::atomic<bool> isPlayingInternal{ false };
    std::atomic<int> currentSlot{ 0 }; std::atomic<int> globalPitchShift{ 0 };
    std::atomic<int> currentGenre{ 1 }; std::atomic<int> baseOctave{ 1 };
    std::atomic<int> glideTimeMs{ 60 };

    std::atomic<bool> isChordOn{ true }; std::atomic<bool> isBassOn{ true };
    std::atomic<int> chordSoundType{ 0 }; std::atomic<int> bassSoundType{ 0 };
    std::atomic<float> staccatoRatio{ 0.3f };
    std::atomic<bool> isStaccatoLocked{ false }; // ★ここに挿入
    std::atomic<float> chordVolume{ 0.1f }; std::atomic<float> bassVolume{ 0.8f };
    std::atomic<int> chordTriggerMode{ 1 }; std::atomic<int> chordOctave{ 0 };

    std::atomic<int> previewNoteMidi{ -1 };

    BarSetting barSettingsUI[4][8];
    StepData patternUI[4][12][1024];
    GlobalSettings genSettings;

    std::atomic<bool> patternUpdated{ false };
    std::atomic<bool> uiNeedsUpdate{ false };
    std::atomic<int> currentPlayingBar{ 0 };
    juce::Random random;

    int getCurrentStep() const { return currentStep; }
    void resetPosition() { samplesInLoop = 0; currentPlayingBar.store(0); currentStep = -1; }
    void clearPattern();
    void clearSpecificPattern(int slot);
    void generateBassline();

    int getMidiNoteFromRow(int row, int tick, int octaveOffset, int slot, bool useDSPData) const;
    juce::String getNoteName(int midiNote) const;
    bool isTickLocked(int tick, int slot) const;

private:
    // ==========================================================
    // ジャンル別 生成関数（すべて引数は5つで統一）
    // ==========================================================
    void generateTechno(int slot, int bars, int numBeats, int ticksPerBar, const GenreDefinition& dna);
    void generateHouse(int slot, int bars, int numBeats, int ticksPerBar, const GenreDefinition& dna);
    void generateUKGarage(int slot, int bars, int numBeats, int ticksPerBar, const GenreDefinition& dna);
    void generateDrumAndBass(int slot, int bars, int numBeats, int ticksPerBar, const GenreDefinition& dna);
    void generateTrap(int slot, int bars, int numBeats, int ticksPerBar, const GenreDefinition& dna);
    void generateFootwork(int slot, int bars, int numBeats, int ticksPerBar, const GenreDefinition& dna);
    void generateIDM(int slot, int bars, int numBeats, int ticksPerBar, const GenreDefinition& dna);
    void generateDubstep(int slot, int bars, int numBeats, int ticksPerBar, const GenreDefinition& dna);
    void generateAfrobeat(int slot, int bars, int numBeats, int ticksPerBar, const GenreDefinition& dna);
    void generateGqom(int slot, int bars, int numBeats, int ticksPerBar, const GenreDefinition& dna);
    void generateAmapiano(int slot, int bars, int numBeats, int ticksPerBar, const GenreDefinition& dna);
    void generateIndian(int slot, int bars, int numBeats, int ticksPerBar, const GenreDefinition& dna);
    void generateLatin(int slot, int bars, int numBeats, int ticksPerBar, const GenreDefinition& dna);
    void generateTrance(int slot, int bars, int numBeats, int ticksPerBar, const GenreDefinition& dna);
    void generateSynthwave(int slot, int bars, int numBeats, int ticksPerBar, const GenreDefinition& dna);
    void generateFunk(int slot, int bars, int numBeats, int ticksPerBar, const GenreDefinition& dna);
    void generateNewJack(int slot, int bars, int numBeats, int ticksPerBar, const GenreDefinition& dna);
    void generateNeoSoul(int slot, int bars, int numBeats, int ticksPerBar, const GenreDefinition& dna);
    void generateBoomBap(int slot, int bars, int numBeats, int ticksPerBar, const GenreDefinition& dna);
    void generateUrbanJazz(int slot, int bars, int numBeats, int ticksPerBar, const GenreDefinition& dna);
    void generateMelodicTechno(int slot, int bars, int numBeats, int ticksPerBar, const GenreDefinition& dna);
    void generateWalkingBass(int slot, int bars, int numBeats, int ticksPerBar, const GenreDefinition& dna);
    void generateElectronicGeneric(int slot, int bars, int numBeats, int ticksPerBar, const GenreDefinition& dna);
    void generateGeneric(int slot, int bars, int numBeats, int ticksPerBar, const GenreDefinition& dna);

    StepData patternDSP[4][12][1024];
    BarSetting barSettingsDSP[4][8];
    // ==============================================================================
    // ★ 修正ブロック 1: PluginProcessor.h の private セクション末尾
    // ==============================================================================
private:
    int lastNoteTickForSlot[4] = { -999, -999, -999, -999 }; // 各スロットの最終発音Tickを記録
    int currentSlotDSP = 0;
    int globalPitchShiftDSP = 0;
    int timeSigNumDSP = 4; int timeSigDenDSP = 4; int globalBarCountDSP = 4;
    int samplesInLoop = 0; int currentStep = -1;

    // 全128個のMIDIノートに対し、あと何サンプルでNote Offするかを記録する配列
    int activeNoteCountdowns[128] = { 0 };

    // 前回処理したTickを記憶し、同じTick内で何度もNote Onが発火するのを防ぐ
    int lastProcessedTick = -1; bool isCurrentlyGliding = false;

    // ★ 課題2の修正: DAWの再生状態の変化（エッジ）を記録するフラグを追加
    bool lastPlayingState = false;

    BassVoice internalSynth;
    std::array<ChordVoice, 5> chordPreviewVoices;
    int chordOffCountdown = 0;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(BassLineMatrixAudioProcessor)
};