// ==============================================================================
// Source/PluginProcessor.cpp
// ==============================================================================
#include "PluginProcessor.h"
#include "PluginEditor.h" 
#include <algorithm>
#include <cmath>

// 各スケールのインターバル（他ジャンルとの互換性を完全維持）
const int scalePatterns[40][8] = {
	{0, 2, 4, 5, 7, 9, 11, -1}, {0, 2, 3, 5, 7, 8, 10, -1}, {0, 2, 4, 7, 9, -1, -1, -1}, {0, 3, 5, 7, 10, -1, -1, -1},
	{0, 2, 3, 5, 7, 9, 10, -1}, {0, 2, 3, 5, 7, 8, 11, -1}, {0, 2, 4, 6, 7, 9, 11, -1}, {0, 2, 4, 5, 7, 9, 10, -1},
	{0, 1, 3, 5, 7, 8, 10, -1}, {0, 2, 3, 5, 7, 9, 11, -1}, {0, 3, 5, 6, 7, 10, -1, -1}, {0, 2, 3, 4, 7, 9, -1, -1},
	{0, 2, 4, 6, 7, 9, 10, -1}, {0, 1, 3, 4, 6, 8, 10, -1}, {0, 1, 3, 5, 6, 8, 10, -1}, {0, 2, 3, 5, 6, 8, 10, -1},
	{0, 2, 4, 5, 7, 9, 10, 11}, {0, 2, 4, 6, 8, 10, -1, -1}, {0, 1, 3, 4, 6, 7, 9, 10}, {0, 2, 3, 5, 6, 8, 9, 11},
	{0, 3, 4, 7, 8, 11, -1, -1}, {0, 1, 4, 5, 7, 8, 10, -1}, {0, 1, 4, 5, 7, 8, 11, -1}, {0, 2, 3, 6, 7, 8, 11, -1},
	{0, 2, 3, 6, 7, 8, 10, -1}, {0, 1, 4, 5, 6, 8, 11, -1}, {0, 1, 4, 5, 6, 9, 10, -1}, {0, 2, 3, 7, 8, -1, -1, -1},
	{0, 1, 5, 7, 10, -1, -1, -1}, {0, 1, 5, 6, 10, -1, -1, -1}, {0, 2, 3, 7, 9, -1, -1, -1}, {0, 1, 3, 7, 8, -1, -1, -1},
	{0, 2, 4, 5, 7, 8, 11, -1}, {0, 1, 3, 5, 7, 9, 11, -1}, {0, 1, 3, 5, 7, 8, 11, -1}, {0, 2, 4, 6, 9, 10, -1, -1},
	{0, 1, 4, 6, 9, 10, -1, -1}, {0, 2, 4, 6, 8, 9, 11, -1}, {0, 2, 4, 6, 7, 8, 10, -1}, {0, 1, 4, 6, 8, 10, 11, -1}
};

const int scaleLengths[40] = {
	7, 7, 5, 5, 7, 7, 7, 7, 7, 7, 6, 6, 7, 7, 7, 7, 8, 6, 8, 8, 6, 7, 7, 7, 7, 7, 7, 5, 5, 5, 5, 5, 7, 7, 7, 6, 6, 7, 7, 7
};

// スケールアウト音（Urban/IDM 用に一部インデックスを最適化）
const int scaleOutNotes[40][4] = {
	{1, 3, 6, 10}, {1, 4, 6, 11}, {1, 3, 5, 10}, {1, 4, 6, 11},
	{1, 4, 6, 11}, // Index 4 (UI 5. Dorian): 変更なし
	{1, 4, 6, 10},
	{1, 3, 5, 10}, // Index 6 (UI 7. Lydian): 変更なし
	{1, 3, 6, 11},
	{2, 4, 6, 11},
	{1, 6, 8, 10}, // Index 9 (UI 10. Melodic Minor): Urban Jazz用にテンション最適化
	{1, 2, 4, 11}, {1, 5, 8, 10}, {1, 3, 5, 11}, {2, 5, 7, 11}, {2, 4, 7, 11}, {1, 4, 7, 11},
	{1, 3, 6, 8},
	{1, 3, 7, 11}, // Index 17 (UI 18. Whole Tone): IDM用に最適化
	{2, 5, 8, 11}, // Index 18 (UI 19. Half-Whole Dim): 変更なし
	{1, 4, 7, 10},
	{1, 2, 5, 9},  {2, 3, 6, 11}, {2, 3, 6, 10}, {1, 4, 5, 10}, {1, 4, 5, 11}, {2, 3, 7, 10}, {2, 3, 7, 11}, {1, 4, 6, 10}, {2, 3, 6, 9},  {2, 3, 7, 9},
	{1, 4, 6, 10}, {2, 4, 6, 10}, {1, 3, 6, 10}, {2, 4, 6, 10}, {2, 4, 6, 10}, {1, 3, 5, 7},  {2, 3, 5, 7},  {1, 3, 5, 7},  {1, 3, 5, 11}, {2, 3, 5, 7}
};

BassLineMatrixAudioProcessor::BassLineMatrixAudioProcessor()
#ifndef JucePlugin_PreferredChannelConfigurations
	: AudioProcessor(BusesProperties()
#if ! JucePlugin_IsMidiEffect
#if ! JucePlugin_IsSynth
		.withInput("Input", juce::AudioChannelSet::stereo(), true)
#endif
		.withOutput("Output", juce::AudioChannelSet::stereo(), true)
#endif
	)
#endif
{
	for (int s = 0; s < 4; ++s) {
		for (int i = 0; i < 8; ++i) {
			barSettingsUI[s][i] = BarSetting();
			barSettingsDSP[s][i] = barSettingsUI[s][i];
		}
	}
}

BassLineMatrixAudioProcessor::~BassLineMatrixAudioProcessor() {}

void BassLineMatrixAudioProcessor::prepareToPlay(double sampleRate, int samplesPerBlock) {
	resetPosition();
	internalSynth.setSampleRate(sampleRate);
	internalSynth.setGlideTime(glideTimeMs.load());
	// 【追加】全ノートのカウントダウンをリセット
	std::fill(std::begin(activeNoteCountdowns), std::end(activeNoteCountdowns), 0);
	lastProcessedTick = -1;
	isCurrentlyGliding = false;
	for (auto& v : chordPreviewVoices) v.setSampleRate(sampleRate);
	chordOffCountdown = 0;
	for (int s = 0; s < 4; ++s) {
		for (int r = 0; r < 12; ++r) std::memcpy(patternDSP[s][r], patternUI[s][r], sizeof(patternUI[s][r]));
		for (int i = 0; i < 8; ++i) barSettingsDSP[s][i] = barSettingsUI[s][i];
	}
	timeSigNumDSP = timeSigNumerator.load(); timeSigDenDSP = timeSigDenominator.load();
	globalBarCountDSP = globalBarCount.load(); currentSlotDSP = currentSlot.load();
	globalPitchShiftDSP = globalPitchShift.load(); patternUpdated.store(false);
}

void BassLineMatrixAudioProcessor::releaseResources() {}

#ifndef JucePlugin_PreferredChannelConfigurations
bool BassLineMatrixAudioProcessor::isBusesLayoutSupported(const BusesLayout& layouts) const {
	if (layouts.getMainOutputChannelSet() != juce::AudioChannelSet::mono() &&
		layouts.getMainOutputChannelSet() != juce::AudioChannelSet::stereo()) return false;
	return true;
}
#endif

juce::String BassLineMatrixAudioProcessor::getNoteName(int midiNote) const {
	if (midiNote < 0 || midiNote > 127) return "";
	const char* names[] = { "C", "C#", "D", "D#", "E", "F", "F#", "G", "G#", "A", "A#", "B" };
	int octave = (midiNote / 12) - 1;
	return juce::String(names[midiNote % 12]) + juce::String(octave);
}

int BassLineMatrixAudioProcessor::getMidiNoteFromRow(int row, int tick, int octaveOffset, int slot, bool useDSPData) const {
	int beats = useDSPData ? timeSigNumDSP : timeSigNumerator.load();
	int ticksPerBar = juce::jmax(1, beats) * 24;
	int barIndex = tick / ticksPerBar;
	int beatInBar = (tick % ticksPerBar) / 24;
	if (barIndex < 0 || barIndex >= 8) barIndex = 0;
	if (slot < 0 || slot > 3) slot = 0;
	const BarSetting& bs = useDSPData ? barSettingsDSP[slot][barIndex] : barSettingsUI[slot][barIndex];

	int noteOffset = 0;

	if (!bs.useCodeMode) {
		int scale = bs.scale;
		if (row < 8) {
			noteOffset = (row / scaleLengths[scale]) * 12 + scalePatterns[scale][row % scaleLengths[scale]];
		}
		else {
			noteOffset = scaleOutNotes[scale][row - 8];
		}
	}
	else {
		// --- CHORD MODE 正常化ロジック ---
		ChordDef chord = bs.chords[juce::jlimit(0, 15, beatInBar)];
		int scale = bs.scale;

		int chordRootOffset = 0;
		int degreeIndex = std::clamp((int)chord.degree, 0, 6);
		chordRootOffset = scalePatterns[scale][degreeIndex];

		if (row < 8) {
			int deg = row % 7;
			int tone = scalePatterns[scale][(degreeIndex + deg) % 7];
			if ((degreeIndex + deg) >= 7) tone += 12;

			// 各度数をコードのQualityに合わせて強制上書き（不足していたQualityを補完）
			switch (deg) {
			case 2: // 3rd
				tone = chordRootOffset + ((chord.quality == ChordQuality::Minor || chord.quality == ChordQuality::Min7 ||
					chord.quality == ChordQuality::Min9 || chord.quality == ChordQuality::Dim ||
					chord.quality == ChordQuality::Dim7 || chord.quality == ChordQuality::HalfDim) ? 3 : 4);
				break;
			case 4: // 5th
				tone = chordRootOffset + ((chord.quality == ChordQuality::Dim || chord.quality == ChordQuality::Dim7 ||
					chord.quality == ChordQuality::HalfDim || chord.quality == ChordQuality::Dom7alt) ? 6 :
					(chord.quality == ChordQuality::Aug ? 8 : 7));
				break;
			case 6: // 7th
				tone = chordRootOffset + ((chord.quality == ChordQuality::Maj7 || chord.quality == ChordQuality::Maj9) ? 11 :
					(chord.quality == ChordQuality::Dim7 ? 9 : 10));
				break;
			}
			noteOffset = ((row / 7) * 12) + tone;
		}
		else {
			int outNotes[4] = { 0, 0, 0, 0 };
			switch (chord.quality) {
			case ChordQuality::Major:
			case ChordQuality::Maj7:
			case ChordQuality::Maj9:
			case ChordQuality::Aug:
				outNotes[0] = -1; outNotes[1] = 3; outNotes[2] = 6; outNotes[3] = 10;
				break;
			case ChordQuality::Minor:
			case ChordQuality::Min7:
			case ChordQuality::Min9:
				outNotes[0] = -2; outNotes[1] = 2; outNotes[2] = 6; outNotes[3] = 9;
				break;
			case ChordQuality::Dom7:
			case ChordQuality::Dom13:
			case ChordQuality::Dom7b9:
			case ChordQuality::Power:
				outNotes[0] = -1; outNotes[1] = 3; outNotes[2] = 6; outNotes[3] = 9;
				break;
			case ChordQuality::Dom7alt:
				outNotes[0] = -1; outNotes[1] = 3; outNotes[2] = 5; outNotes[3] = 8;
				break;
			case ChordQuality::Dim:
			case ChordQuality::Dim7:
			case ChordQuality::HalfDim:
				outNotes[0] = -1; outNotes[1] = 2; outNotes[2] = 5; outNotes[3] = 8;
				break;
			default:
				outNotes[0] = -1; outNotes[1] = 2; outNotes[2] = 6; outNotes[3] = 10;
				break;
			}
			int idx = std::clamp(row - 8, 0, 3);
			noteOffset = chordRootOffset + outNotes[idx];
		}
	}

	int shift = useDSPData ? globalPitchShiftDSP : globalPitchShift.load();
	int baseOct = baseOctave.load() + 1 + octaveOffset;
	return juce::jlimit(0, 127, baseOct * 12 + bs.key + noteOffset + shift);
}

bool BassLineMatrixAudioProcessor::isTickLocked(int tick, int slot) const {
	for (int r = 0; r < 12; ++r) { if (patternUI[slot][r][tick].locked) return true; }
	return false;
}

void BassLineMatrixAudioProcessor::clearSpecificPattern(int slot) {
	for (int r = 0; r < 12; ++r) {
		for (int s = 0; s < 1024; ++s) {
			if (!patternUI[slot][r][s].locked) patternUI[slot][r][s] = StepData();
		}
	}
	patternUpdated.store(true); uiNeedsUpdate.store(true);
}

void BassLineMatrixAudioProcessor::clearPattern() { clearSpecificPattern(currentSlot.load()); }

// ==============================================================================
// 1. Techno
// ==============================================================================
void BassLineMatrixAudioProcessor::generateTechno(int slot, int bars, int numBeats, int ticksPerBar, const GenreDefinition& dna) {
	int subStyle = random.nextInt(4);
	int stepsPerBar = ticksPerBar / 6;

	static const int technoRhythms[4][20][16] = {
		{{4, 0, 0, 0,  4, 0, 0, 0,  4, 0, 0, 0,  4, 0, 0, 0}, {4, 0, 4, 0,  4, 0, 4, 0,  4, 0, 4, 0,  4, 0, 4, 0}, {4, 2, 4, 0,  4, 2, 4, 0,  4, 2, 4, 0,  4, 2, 4, 0}, {4, 2, 4, 2,  4, 2, 4, 2,  4, 2, 4, 2,  4, 2, 4, 2}, {6, 0, 6, 0,  6, 0, 6, 0,  6, 0, 6, 0,  6, 0, 6, 0}, {6, 0, 4, 2,  6, 0, 4, 2,  6, 0, 4, 2,  6, 0, 4, 2}, {4, 4, 4, 4,  4, 0, 4, 0,  4, 4, 4, 4,  4, 0, 4, 0}, {4, 4, 4, 4,  4, 4, 4, 0,  4, 4, 4, 4,  4, 4, 4, 0}, {4, 4, 4, 4,  4, 4, 4, 4,  4, 4, 4, 4,  4, 4, 4, 0}, {4, 4, 4, 4,  4, 4, 4, 4,  4, 4, 4, 4,  4, 4, 4, 4}, {3, 3, 3, 3,  3, 3, 3, 3,  3, 3, 3, 3,  3, 3, 3, 3}, {3, 3, 2, 4,  3, 3, 2, 4,  3, 3, 2, 4,  3, 3, 2, 4}, {2, 2, 2, 2,  4, 0, 4, 0,  2, 2, 2, 2,  4, 0, 4, 0}, {2, 2, 2, 2,  2, 2, 2, 2,  4, 0, 4, 0,  4, 0, 4, 0}, {2, 2, 2, 2,  2, 2, 2, 2,  2, 2, 2, 2,  4, 4, 4, 4}, {2, 2, 2, 2,  2, 2, 2, 2,  2, 2, 2, 2,  2, 2, 2, 2}, {6, 2, 6, 2,  6, 2, 6, 2,  6, 2, 6, 2,  6, 2, 6, 2}, {6, 4, 2, 6,  4, 2, 6, 4,  2, 6, 4, 2,  6, 4, 2, 6}, {4, 2, 4, 2,  4, 2, 4, 2,  4, 2, 4, 2,  4, 2, 4, 2}, {2, 2, 2, 2,  2, 2, 2, 2,  2, 2, 2, 2,  2, 2, 2, 2}},
		{{0,  0, 6, 0,  0, 0, 6, 0,  0, 0, 6, 0,  0, 0, 6, 0}, {0,  0, 12,0,  0, 0, 12,0,  0, 0, 12,0,  0, 0, 12,0}, {0,  0, 6, 6,  0, 0, 6, 6,  0, 0, 6, 6,  0, 0, 6, 6}, {0,  0, 4, 2,  0, 0, 4, 2,  0, 0, 4, 2,  0, 0, 4, 2}, {0,  6, 6, 0,  0, 6, 6, 0,  0, 6, 6, 0,  0, 6, 6, 0}, {0,  6, 6, 6,  0, 6, 6, 6,  0, 6, 6, 6,  0, 6, 6, 6}, {4,  2, 6, 0,  4,  2, 6, 0,  4,  2, 6, 0,  4,  2, 6, 0}, {4,  2, 6, 2,  4,  2, 6, 2,  4,  2, 6, 2,  4,  2, 6, 2}, {6,  0, 6, 0,  6,  0, 6, 0,  6,  0, 6, 0,  6,  0, 6, 0}, {6,  4, 2, 0,  6,  4, 2, 0,  6,  4, 2, 0,  6,  4, 2, 0}, {3,  3, 3, 3,  0,  0, 6, 0,  3,  3, 3, 3,  0,  0, 6, 0}, {3,  3, 3, 3,  3,  3, 3, 3,  3,  3, 3, 3,  3,  3, 3, 3}, {2,  2, 8, 2,  2,  2, 8, 2,  2,  2, 8, 2,  2,  2, 8, 2}, {2,  2, 2, 2,  6,  0, 6, 0,  2,  2, 2, 2,  6,  0, 6, 0}, {4,  4, 4, 4,  4,  4, 4, 4,  0,  0, 0, 0,  4,  4, 4, 4}, {2,  2, 2, 2,  2,  2, 2, 2,  2,  2, 2, 2,  4,  2, 4, 2}, {6,  6, 6, 6,  0,  0, 0, 0,  6,  6, 6, 6,  0,  0, 0, 0}, {4,  4, 4, 4,  2,  2, 2, 2,  4,  4, 4, 4,  2,  2, 2, 2}, {6,  0, 6, 0,  6,  0, 6, 0,  6,  0, 6, 0,  6,  0, 6, 0}, {4,  2, 4, 2,  4,  2, 4, 2,  4,  2, 4, 2,  4,  2, 4, 2}},
		{{6,  0, 0, 6,  0, 0, 6, 0,  0, 6, 0, 0,  6, 0, 0, 6}, {6,  0, 0, 6,  0, 0, 6, 0,  0, 6, 0, 0,  12,0, 0, 0}, {4,  0, 0, 4,  0, 0, 4, 0,  0, 4, 0, 0,  4, 0, 0, 4}, {4,  2, 0, 4,  2, 0, 4, 2,  0, 4, 2, 0,  4, 2, 0, 4}, {10, 0, 0, 0,  0, 10,0, 0,  0, 0, 10,0,  0, 0, 0, 0}, {10, 0, 4, 10, 0, 4, 10,0,  4, 10,0, 4,  10,0, 4, 0}, {6,  6, 0, 6,  6, 0, 6, 6,  0, 6, 6, 0,  6, 6, 0, 6}, {4,  4, 4, 0,  4, 4, 4, 0,  4, 4, 4, 0,  4, 4, 4, 0}, {3,  3, 3, 3,  3, 3, 0, 0,  3, 3, 3, 3,  3, 3, 0, 0}, {3,  3, 3, 3,  3, 3, 3, 3,  3, 3, 3, 3,  3, 3, 3, 3}, {2,  2, 2, 2,  2, 2, 2, 2,  2, 2, 2, 2,  2, 2, 2, 2}, {6,  0, 0, 0,  6, 0, 0, 0,  6, 0, 0, 0,  6, 0, 0, 0}, {8,  0, 0, 8,  0, 0, 8, 0,  0, 8, 0, 0,  8, 0, 0, 8}, {4,  2, 4, 2,  4, 2, 0, 0,  4, 2, 4, 2,  4, 2, 0, 0}, {2,  2, 2, 2,  4, 4, 4, 4,  2,  2, 2, 2,  4, 4, 4, 4}, {6,  4, 2, 0,  6, 4, 2, 0,  6, 4, 2, 0,  6, 4, 2, 0}, {4,  4, 4, 4,  4, 4, 4, 4,  0,  0, 0, 0,  0, 0, 0, 0}, {2,  2, 2, 2,  2, 2, 2, 2,  0,  0, 0, 0,  0, 0, 0, 0}, {12, 0, 0, 0,  12,0, 0, 0,  12, 0, 0, 0,  12,0, 0, 0}, {18, 0, 0, 0,  18,0, 0, 0,  18, 0, 0, 0,  18,0, 0, 0}},
		{{24, 0, 0, 0,  0, 0, 0, 0,  24, 0, 0, 0,  0, 0, 0, 0}, {24, 0, 0, 0,  0, 0, 12,0,  24, 0, 0, 0,  0, 0, 0, 0}, {18, 0, 0, 6,  0, 0, 0, 0,  18, 0, 0, 6,  0, 0, 0, 0}, {18, 0, 0, 6,  0, 0, 12,0,  18, 0, 0, 6,  0, 0, 0, 0}, {12, 0, 0, 0,  12, 0, 0, 0, 12, 0, 0, 0,  12, 0, 0, 0}, {12, 0, 6, 0,  12, 0, 6, 0, 12, 0, 6, 0,  12, 0, 6, 0}, {6,  6, 0, 0,  12, 0, 0, 0, 6,  6, 0, 0,  12, 0, 0, 0}, {6,  6, 0, 0,  6,  6, 0, 0, 6,  6, 0, 0,  6,  6, 0, 0}, {6,  4, 2, 0,  6,  4, 2, 0, 6,  4, 2, 0,  6,  4, 2, 0}, {4,  4, 4, 4,  0,  0, 0, 0, 4,  4, 4, 4,  0,  0, 0, 0}, {2,  2, 2, 2,  12, 0, 0, 0, 2,  2, 2, 2,  12, 0, 0, 0}, {3,  3, 3, 3,  6,  6, 6, 6, 3,  3, 3, 3,  6,  6, 6, 6}, {12, 0, 0, 0,  0,  0, 0, 0, 0,  0, 0, 0,  0,  0, 0, 0}, {18, 0, 0, 0,  0,  0, 0, 0, 0,  0, 0, 0,  0,  0, 0, 0}, {24, 0, 0, 0,  0,  0, 0, 0, 0,  0, 0, 0,  0,  0, 0, 0}, {30, 0, 0, 0,  0,  0, 0, 0, 0,  0, 0, 0,  0,  0, 0, 0}, {36, 0, 0, 0,  0,  0, 0, 0, 0,  0, 0, 0,  0,  0, 0, 0}, {42, 0, 0, 0,  0,  0, 0, 0, 0,  0, 0, 0,  0,  0, 0, 0}, {48, 0, 0, 0,  0,  0, 0, 0, 0,  0, 0, 0,  0,  0, 0, 0}, {90, 0, 0, 0,  0,  0, 0, 0, 0,  0, 0, 0,  0,  0, 0, 0}}
	};

	for (int bar = 0; bar < bars; ++bar) {
		auto& bs = barSettingsUI[slot][bar];
		int barStart = bar * ticksPerBar;
		if (barStart >= 1024) break;

		// ★ 型安全な明示的キャスト
		int cIdx = juce::jlimit<int>(0, 19, static_cast<int>(bs.cmplx) / 5);
		int eIdx = juce::jlimit<int>(0, 9, static_cast<int>(bs.entrp) / 10);
		int nextAvailableTick = -1;

		for (int step = 0; step < stepsPerBar; ++step) {
			int len = technoRhythms[subStyle][cIdx][step % 16];
			if (len == 0) continue;

			int tick = barStart + step * 6;
			if (tick >= 1024 || tick < nextAvailableTick || isTickLocked(tick, slot)) continue;

			int row = 0, oct = 0;
			int vel = (step % 4 == 0) ? 127 : 100 + random.nextInt(15);

			if (eIdx <= 2) row = 0;
			else if (eIdx <= 5) row = (random.nextInt(100) < 85) ? 0 : 4;
			else if (eIdx <= 7) { const int rows[] = { 0, 4, 2 }; row = rows[random.nextInt(3)]; }
			else { if (random.nextBool()) row = 0; else row = random.nextInt(7); }

			if (eIdx >= 3 && step % 2 != 0) { int octProb = (eIdx - 2) * 10; if (random.nextInt(100) < octProb) oct = 1; }
			if (eIdx >= 8 && step >= 12) { if (random.nextInt(100) < (eIdx * 2)) { row = 8 + random.nextInt(2); vel -= 10; len = 2; oct = 0; } }

			auto& d = patternUI[slot][row][tick % 1024];
			d.velocity = juce::jlimit<int>(1, 127, vel);
			d.length = std::max(2, len);
			d.offset = 0;
			if (len > 6 && random.nextInt(100) < static_cast<int>(bs.autoGlide)) d.glide = true; else d.glide = false;
			d.staccato = (d.length <= 4);
			d.octave = juce::jlimit<int>(0, 1, oct);
			nextAvailableTick = tick + d.length;
		}
	}
}

// ==============================================================================
// 2. House
// ==============================================================================
void BassLineMatrixAudioProcessor::generateHouse(int slot, int bars, int numBeats, int ticksPerBar, const GenreDefinition& dna) {
	int subStyle = random.nextInt(4);
	int stepsPerBar = ticksPerBar / 6;

	static const int houseRhythms[4][20][16] = {
		{{12, 0, 0, 0,  12, 0, 0, 0,  12, 0, 0, 0,  12, 0, 0, 0}, {12, 0, 0, 0,  0,  0, 12,0,  12, 0, 0, 0,  0,  0, 12,0}, {6,  0, 6, 0,  12, 0, 0, 0,  6,  0, 6, 0,  12, 0, 0, 0}, {6,  0, 6, 0,  0,  0, 12,0,  6,  0, 6, 0,  0,  0, 12,0}, {6,  0, 0, 6,  12, 0, 0, 0,  6,  0, 0, 6,  12, 0, 0, 0}, {6,  0, 0, 6,  0,  0, 6, 6,  6,  0, 0, 6,  0,  0, 6, 6}, {0,  0, 6, 6,  0,  0, 6, 6,  0,  0, 6, 6,  0,  0, 6, 6}, {0,  0, 6, 6,  12, 0, 0, 0,  0,  0, 6, 6,  12, 0, 0, 0}, {6,  0, 4, 2,  6,  0, 0, 6,  6,  0, 4, 2,  6,  0, 0, 6}, {6,  0, 4, 2,  0,  0, 4, 2,  6,  0, 4, 2,  0,  0, 4, 2}, {4,  2, 6, 0,  6,  0, 6, 0,  4,  2, 6, 0,  6,  0, 6, 0}, {4,  2, 0, 6,  0,  6, 4, 2,  4,  2, 0, 6,  0,  6, 4, 2}, {6,  0, 4, 2,  4,  2, 4, 2,  6,  0, 4, 2,  4,  2, 4, 2}, {4,  2, 4, 2,  6,  0, 0, 6,  4,  2, 4, 2,  6,  0, 0, 6}, {4,  2, 4, 2,  4,  2, 4, 2,  6,  0, 6, 0,  6,  0, 6, 0}, {4,  2, 4, 2,  4,  2, 4, 2,  4,  2, 4, 2,  4,  2, 4, 2}, {2,  4, 2, 4,  0,  6, 6, 0,  2,  4, 2, 4,  0,  6, 6, 0}, {2,  4, 2, 4,  2,  4, 2, 4,  6,  0, 4, 2,  4,  2, 6, 0}, {2,  4, 2, 4,  2,  4, 2, 4,  2,  4, 2, 4,  2,  4, 2, 4}, {2,  2, 2, 6,  2,  2, 2, 6,  2,  2, 2, 6,  2,  2, 2, 6}},
		{{24, 0, 0, 0,  0,  0, 0, 0,  24, 0, 0, 0,  0,  0, 0, 0}, {18, 0, 0, 6,  0,  0, 0, 0,  18, 0, 0, 6,  0,  0, 0, 0}, {12, 0, 0, 0,  12, 0, 0, 0,  12, 0, 0, 0,  12, 0, 0, 0}, {12, 0, 0, 0,  0,  0, 12,0,  12, 0, 0, 0,  0,  0, 12,0}, {18, 0, 0, 0,  0,  0, 6, 0,  18, 0, 0, 0,  0,  0, 6, 0}, {12, 0, 6, 0,  12, 0, 0, 0,  12, 0, 6, 0,  12, 0, 0, 0}, {12, 0, 0, 6,  12, 0, 0, 0,  12, 0, 0, 6,  12, 0, 0, 0}, {12, 0, 6, 0,  0,  0, 12,0,  12, 0, 6, 0,  0,  0, 12,0}, {6,  6, 12,0,  0,  0, 0, 0,  6,  6, 12,0,  0,  0, 0, 0}, {6,  6, 0, 6,  12, 0, 0, 0,  6,  6, 0, 6,  12, 0, 0, 0}, {12, 0, 6, 6,  0,  0, 0, 0,  12, 0, 6, 6,  0,  0, 0, 0}, {8,  0, 4, 0,  12, 0, 0, 0,  8,  0, 4, 0,  12, 0, 0, 0}, {8,  0, 4, 0,  8,  0, 4, 0,  8,  0, 4, 0,  8,  0, 4, 0}, {6,  6, 6, 6,  0,  0, 0, 0,  6,  6, 6, 6,  0,  0, 0, 0}, {6,  6, 6, 6,  12, 0, 0, 0,  6,  6, 6, 6,  12, 0, 0, 0}, {6,  0, 6, 0,  4,  2, 6, 0,  6,  0, 6, 0,  4,  2, 6, 0}, {4,  2, 6, 0,  12, 0, 0, 0,  4,  2, 6, 0,  12, 0, 0, 0}, {4,  2, 4, 2,  12, 0, 0, 0,  4,  2, 4, 2,  12, 0, 0, 0}, {4,  2, 4, 2,  4,  2, 6, 0,  4,  2, 4, 2,  4,  2, 6, 0}, {4,  2, 4, 2,  4,  2, 4, 2,  4,  2, 4, 2,  4,  2, 4, 2}},
		{{0,  0, 12,0,  0,  0, 12,0,  0,  0, 12,0,  0,  0, 12,0}, {0,  0, 6, 0,  0,  0, 12,0,  0,  0, 6, 0,  0,  0, 12,0}, {6,  0, 6, 0,  0,  0, 12,0,  6,  0, 6, 0,  0,  0, 12,0}, {0,  0, 6, 6,  0,  0, 12,0,  0,  0, 6, 6,  0,  0, 12,0}, {0,  0, 6, 0,  6,  0, 6, 0,  0,  0, 6, 0,  6,  0, 6, 0}, {0,  0, 12,0,  0,  0, 6, 6,  0,  0, 12,0,  0,  0, 6, 6}, {0,  6, 6, 0,  0,  6, 6, 0,  0,  6, 6, 0,  0,  6, 6, 0}, {0,  6, 6, 0,  0,  0, 12,0,  0,  6, 6, 0,  0,  0, 12,0}, {6,  0, 0, 6,  6,  0, 6, 0,  6,  0, 0, 6,  6,  0, 6, 0}, {6,  0, 0, 6,  0,  0, 6, 6,  6,  0, 0, 6,  0,  0, 6, 6}, {4,  2, 6, 0,  0,  0, 12,0,  4,  2, 6, 0,  0,  0, 12,0}, {0,  0, 4, 2,  6,  0, 6, 0,  0,  0, 4, 2,  6,  0, 6, 0}, {0,  0, 4, 2,  0,  0, 4, 2,  0,  0, 4, 2,  0,  0, 4, 2}, {4,  2, 0, 6,  4,  2, 0, 6,  4,  2, 0, 6,  4,  2, 0, 6}, {2,  4, 6, 0,  2,  4, 6, 0,  2,  4, 6, 0,  2,  4, 6, 0}, {2,  4, 0, 6,  2,  4, 0, 6,  2,  4, 0, 6,  2,  4, 0, 6}, {4,  2, 4, 2,  0,  0, 12,0,  4,  2, 4, 2,  0,  0, 12,0}, {0,  0, 4, 2,  4,  2, 4, 2,  0,  0, 4, 2,  4,  2, 4, 2}, {4,  2, 4, 2,  0,  6, 6, 0,  4,  2, 4, 2,  0,  6, 6, 0}, {2,  4, 2, 4,  2,  4, 2, 4,  2,  4, 2, 4,  2,  4, 2, 4}},
		{{12, 0, 0, 0,  0,  0, 12,0,  12, 0, 0, 0,  0,  0, 12,0}, {18, 0, 0, 0,  0,  0, 6, 0,  18, 0, 0, 0,  0,  0, 6, 0}, {6,  0, 6, 0,  0,  0, 12,0,  6,  0, 6, 0,  0,  0, 12,0}, {12, 0, 0, 0,  0,  6, 6, 0,  12, 0, 0, 0,  0,  6, 6, 0}, {6,  0, 0, 6,  0,  0, 12,0,  6,  0, 0, 6,  0,  0, 12,0}, {6,  0, 6, 0,  0,  6, 6, 0,  6,  0, 6, 0,  0,  6, 6, 0}, {6,  0, 0, 6,  0,  6, 6, 0,  6,  0, 0, 6,  0,  6, 6, 0}, {12, 0, 0, 0,  0,  4, 8, 0,  12, 0, 0, 0,  0,  4, 8, 0}, {8,  0, 4, 0,  0,  0, 12,0,  8,  0, 4, 0,  0,  0, 12,0}, {6,  0, 4, 2,  0,  0, 12,0,  6,  0, 4, 2,  0,  0, 12,0}, {6,  0, 6, 0,  0,  4, 8, 0,  6,  0, 6, 0,  0,  4, 8, 0}, {4,  2, 6, 0,  0,  0, 12,0,  4,  2, 6, 0,  0,  0, 12,0}, {6,  0, 0, 6,  0,  4, 8, 0,  6,  0, 0, 6,  0,  4, 8, 0}, {4,  2, 0, 6,  0,  0, 12,0,  4,  2, 0, 6,  0,  0, 12,0}, {4,  2, 6, 0,  0,  4, 8, 0,  4,  2, 6, 0,  0,  4, 8, 0}, {4,  2, 4, 2,  0,  0, 12,0,  4,  2, 4, 2,  0,  0, 12,0}, {6,  0, 4, 2,  0,  4, 4, 4,  6,  0, 4, 2,  0,  4, 4, 4}, {4,  2, 4, 2,  0,  6, 6, 0,  4,  2, 4, 2,  0,  6, 6, 0}, {4,  2, 4, 2,  0,  4, 8, 0,  4,  2, 4, 2,  0,  4, 8, 0}, {4,  2, 4, 2,  0,  2, 4, 6,  4,  2, 4, 2,  0,  2, 4, 6}}
	};

	for (int bar = 0; bar < bars; ++bar) {
		auto& bs = barSettingsUI[slot][bar];
		int barStart = bar * ticksPerBar;
		if (barStart >= 1024) break;

		// ★ 型安全キャスト
		int cIdx = juce::jlimit<int>(0, 19, static_cast<int>(bs.cmplx) / 5);
		int eIdx = juce::jlimit<int>(0, 9, static_cast<int>(bs.entrp) / 10);
		int nextAvailableTick = -1;

		for (int step = 0; step < stepsPerBar; ++step) {
			int len = houseRhythms[subStyle][cIdx][step % 16];
			if (len == 0) continue;

			int tick = barStart + step * 6;
			if (tick >= 1024 || tick < nextAvailableTick || isTickLocked(tick, slot)) continue;

			int row = 0, oct = 0;
			int vel = (step % 2 == 0) ? 100 : 115;
			vel = juce::jlimit<int>(1, 127, vel + random.nextInt(15) - 7);

			if (step % 8 == 0) { if (eIdx < 3) row = 0; else row = (random.nextBool()) ? 0 : 4; }
			else { if (eIdx == 0) row = 0; else if (eIdx <= 2) row = (random.nextBool()) ? 0 : 4; else if (eIdx <= 4) { const int rows[] = { 0, 2, 4, 6 }; row = rows[random.nextInt(4)]; } else if (eIdx <= 7) { const int rows[] = { 0, 1, 2, 4, 5, 6 }; row = rows[random.nextInt(6)]; } else row = random.nextInt(7); }
			if (step % 2 != 0 && eIdx >= 3) { if (random.nextInt(100) < eIdx * 4) { oct = 1; vel += 10; } }
			if (eIdx >= 7 && step % 16 == 15) { if (random.nextInt(100) < eIdx * 3) { row = 8 + random.nextInt(4); vel -= 15; len = 2; oct = 0; } }

			auto& d = patternUI[slot][row][tick % 1024];
			d.velocity = juce::jlimit<int>(1, 127, vel);
			d.length = std::max(2, len);
			d.offset = 0;
			if (len > 6 && random.nextInt(100) < static_cast<int>(bs.autoGlide)) d.glide = true; else d.glide = false;
			d.staccato = (d.length <= 4);
			d.octave = juce::jlimit<int>(0, 1, oct);
			nextAvailableTick = tick + d.length;
		}
	}
}

// ==============================================================================
// 3. UK Garage
// ==============================================================================
void BassLineMatrixAudioProcessor::generateUKGarage(int slot, int bars, int numBeats, int ticksPerBar, const GenreDefinition& dna) {
	int subStyle = random.nextInt(4);
	int stepsPerBar = ticksPerBar / 6;

	static const int ukgRhythms[4][10][16] = {
		{{12, 0, 0, 0,  0, 0, 0, 0,  12, 0, 0, 0,  0, 0, 0, 0}, {12, 0, 0, 0,  0, 0, 0, 6,  12, 0, 0, 0,  0, 0, 0, 6}, {10, 0, 4, 0,  0, 0, 0, 6,  10, 0, 4, 0,  0, 0, 0, 6}, {8,  0, 4, 0,  0, 0, 6, 4,  8,  0, 4, 0,  0, 0, 6, 4}, {6,  0, 6, 0,  0, 0, 6, 6,  6,  0, 6, 0,  0, 0, 6, 6}, {6,  0, 4, 2,  0, 0, 6, 4,  6,  0, 4, 2,  0, 0, 6, 4}, {6,  2, 4, 0,  0, 0, 4, 6,  6,  2, 4, 0,  0, 0, 4, 6}, {4,  2, 4, 2,  0, 0, 6, 4,  4,  2, 4, 2,  0, 0, 6, 4}, {4,  4, 4, 0,  0, 4, 4, 4,  4,  4, 4, 0,  0, 4, 4, 4}, {4,  4, 4, 4,  0, 4, 4, 4,  4,  4, 4, 4,  0, 4, 4, 4}},
		{{24, 0, 0, 0,  0, 0, 0, 0,  0,  0, 0, 0,  0, 0, 0, 0}, {18, 0, 0, 0,  0, 0, 6, 0,  0,  0, 0, 0,  0, 0, 0, 0}, {12, 0, 0, 0,  0, 0, 12,0,  0,  0, 0, 0,  0, 0, 0, 0}, {12, 0, 0, 0,  0, 0, 6, 6,  0,  0, 0, 0,  0, 0, 0, 0}, {12, 0, 0, 0,  6, 0, 6, 0,  12, 0, 0, 0,  0, 0, 6, 0}, {8,  0, 4, 0,  0, 0, 6, 4,  12, 0, 0, 0,  6, 0, 6, 0}, {6,  6, 0, 0,  12,0, 0, 0,  6,  6, 0, 0,  6, 0, 6, 0}, {6,  4, 2, 0,  6, 0, 6, 0,  6,  4, 2, 0,  6, 0, 6, 0}, {4,  4, 4, 0,  6, 6, 0, 0,  4,  4, 4, 0,  6, 6, 0, 0}, {4,  2, 4, 2,  4, 2, 4, 2,  4,  2, 4, 2,  4, 2, 4, 2}},
		{{0,  0, 12,0,  0, 0, 0, 0,  0,  0, 12,0,  0, 0, 0, 0}, {0,  0, 12,0,  0, 0, 0, 0,  12, 0, 0, 0,  0, 0, 0, 0}, {6,  0, 6, 0,  0, 0, 0, 0,  6,  0, 6, 0,  0, 0, 0, 0}, {6,  0, 0, 6,  0, 0, 12,0,  6,  0, 0, 6,  0, 0, 12,0}, {6,  0, 0, 6,  0, 0, 6, 6,  6,  0, 0, 6,  0, 0, 6, 6}, {4,  0, 4, 4,  0, 0, 6, 6,  4,  0, 4, 4,  0, 0, 6, 6}, {4,  0, 2, 6,  0, 0, 4, 6,  4,  0, 2, 6,  0, 0, 4, 6}, {4,  2, 0, 6,  0, 4, 2, 6,  4,  2, 0, 6,  0, 4, 2, 6}, {2,  4, 2, 4,  0, 6, 2, 4,  2,  4, 2, 4,  0, 6, 2, 4}, {2,  2, 2, 6,  0, 2, 2, 8,  2,  2, 2, 6,  0, 2, 2, 8}},
		{{12, 0, 0, 0,  12, 0, 0, 0, 12, 0, 0, 0,  12, 0, 0, 0}, {12, 0, 0, 0,  0,  0, 12,0, 12, 0, 0, 0,  0,  0, 12,0}, {6,  0, 6, 0,  12, 0, 0, 0, 6,  0, 6, 0,  12, 0, 0, 0}, {6,  0, 6, 0,  0,  0, 12,0, 6,  0, 6, 0,  0,  0, 12,0}, {6,  0, 6, 0,  6,  0, 6, 0, 6,  0, 6, 0,  6,  0, 6, 0}, {6,  0, 6, 0,  4,  2, 6, 0, 6,  0, 6, 0,  4,  2, 6, 0}, {6,  0, 4, 2,  4,  2, 6, 0, 6,  0, 4, 2,  4,  2, 6, 0}, {4,  2, 4, 2,  6,  0, 6, 0, 4,  2, 4, 2,  6,  0, 6, 0}, {4,  2, 4, 2,  4,  2, 4, 2, 4,  2, 4, 2,  4,  2, 4, 2}, {3,  3, 3, 3,  6,  0, 6, 0, 3,  3, 3, 3,  6,  0, 6, 0}}
	};

	for (int bar = 0; bar < bars; ++bar) {
		auto& bs = barSettingsUI[slot][bar];
		int barStart = bar * ticksPerBar;
		if (barStart >= 1024) break;

		// ★ 型安全キャスト
		int cIdx = juce::jlimit<int>(0, 9, static_cast<int>(bs.cmplx) / 10);
		int eIdx = juce::jlimit<int>(0, 9, static_cast<int>(bs.entrp) / 10);

		for (int step = 0; step < stepsPerBar; ++step) {
			int len = ukgRhythms[subStyle][cIdx][step % 16];
			if (len == 0) continue;

			int tick = barStart + step * 6;
			if (tick >= 1024 || isTickLocked(tick, slot)) continue;

			int row = 0, oct = 0;
			int vel = (step % 4 == 0) ? 115 : 95;
			vel = juce::jlimit<int>(1, 127, vel + random.nextInt(15) - 7);

			if (step % 8 == 0) { if (eIdx < 3) row = 0; else row = (random.nextBool()) ? 0 : 4; }
			else { if (eIdx == 0) row = 0; else if (eIdx == 1) row = (random.nextBool()) ? 0 : 4; else if (eIdx <= 3) { const int rows[] = { 0, 2, 4, 6 }; row = rows[random.nextInt(4)]; } else if (eIdx <= 6) { const int rows[] = { 0, 1, 2, 4, 5, 6 }; row = rows[random.nextInt(6)]; } else row = random.nextInt(7); }
			if (eIdx >= 7 && step % 2 != 0) { if (random.nextInt(100) < (eIdx == 7 ? 10 : (eIdx == 8 ? 20 : 30))) oct = 1; }
			if (eIdx == 9 && step % 16 >= 14 && random.nextInt(100) < 15) { row = 8 + random.nextInt(4); vel -= 15; len = 2; oct = 0; }

			auto& d = patternUI[slot][row][tick % 1024];
			d.velocity = vel;
			d.length = std::max(2, len);
			d.offset = 0;
			if (len > 6 && random.nextInt(100) < static_cast<int>(bs.autoGlide)) d.glide = true; else d.glide = false;
			d.staccato = (d.length <= 4);
			d.octave = juce::jlimit<int>(0, 1, oct);
		}
	}
}

// ==============================================================================
// 4. Drum & Bass
// ==============================================================================
void BassLineMatrixAudioProcessor::generateDrumAndBass(int slot, int bars, int numBeats, int ticksPerBar, const GenreDefinition& dna) {
	int subStyle = random.nextInt(4);
	int stepsPerBar = ticksPerBar / 6;

	static const int dnbRhythms[4][20][16] = {
		{{48, 0, 0, 0,  0, 0, 0, 0,  0, 0, 0, 0,  0, 0, 0, 0}, {48, 0, 0, 0,  0, 0, 0, 0,  12,0, 0, 0,  0, 0, 0, 0}, {48, 0, 0, 0,  0, 0, 0, 0,  24,0, 0, 0,  0, 0, 0, 0}, {48, 0, 0, 0,  0, 0, 0, 0,  12,0, 6, 0,  0, 0, 0, 0}, {36, 0, 0, 0,  0, 0, 12,0,  12,0, 0, 0,  12,0, 0, 0}, {36, 0, 0, 0,  0, 0, 12,0,  12,0, 6, 0,  0, 0, 6, 0}, {24, 0, 0, 0,  12,0, 0, 0,  24,0, 0, 0,  12,0, 0, 0}, {24, 0, 0, 0,  12,0, 0, 0,  12,0, 12,0,  6, 0, 6, 0}, {24, 0, 0, 0,  0, 0, 12,0,  12,0, 6, 0,  6, 0, 6, 0}, {24, 0, 0, 0,  6, 0, 6, 0,  12,0, 6, 0,  6, 0, 6, 0}, {12, 0, 6, 0,  0, 0, 12,0,  12,0, 6, 0,  6, 0, 6, 0}, {12, 0, 6, 0,  6, 0, 12,0,  12,0, 6, 0,  6, 0, 6, 0}, {12, 0, 6, 0,  6, 0, 6, 0,  12,0, 6, 0,  6, 0, 6, 0}, {12, 0, 6, 0,  6, 0, 6, 0,  6, 6, 6, 0,  6, 0, 6, 0}, {12, 0, 0, 6,  6, 0, 6, 0,  6, 6, 6, 0,  6, 0, 6, 0}, {6,  6, 0, 6,  6, 0, 6, 0,  6, 6, 6, 0,  6, 0, 6, 0}, {6,  6, 0, 6,  6, 0, 6, 0,  6, 4, 2, 0,  6, 0, 6, 0}, {6,  4, 2, 6,  6, 0, 6, 0,  6, 4, 2, 0,  6, 4, 2, 0}, {6,  4, 2, 6,  4, 2, 6, 0,  6, 4, 2, 0,  6, 4, 2, 0}, {6,  4, 2, 4,  2, 4, 2, 6,  6, 4, 2, 4,  2, 4, 2, 6}},
		{{6,  0, 0, 0,  0, 0, 0, 0,  6,  0, 0, 0,  0, 0, 0, 0}, {6,  0, 0, 0,  0, 0, 6, 0,  6,  0, 0, 0,  0, 0, 0, 0}, {6,  0, 0, 0,  0, 0, 6, 0,  6,  0, 0, 0,  0, 0, 6, 0}, {6,  0, 0, 6,  0, 0, 6, 0,  6,  0, 0, 0,  0, 0, 6, 0}, {6,  0, 6, 0,  0, 0, 6, 0,  6,  0, 0, 6,  0, 0, 6, 0}, {6,  0, 6, 0,  0, 0, 6, 0,  6,  0, 6, 0,  0, 0, 6, 0}, {6,  0, 6, 0,  6,  0, 6, 0, 6,  0, 6, 0,  0, 0, 6, 0}, {6,  0, 6, 0,  6,  0, 6, 0, 6,  0, 6, 0,  6,  0, 6, 0}, {4,  0, 4, 0,  4,  0, 4, 0, 6,  0, 6, 0,  6,  0, 6, 0}, {4,  0, 4, 0,  4,  0, 4, 0, 4,  0, 4, 0,  4,  0, 4, 0}, {4,  2, 4, 0,  4,  0, 4, 0, 4,  0, 4, 0,  4,  0, 4, 0}, {4,  2, 4, 0,  4,  2, 4, 0, 4,  0, 4, 0,  4,  0, 4, 0}, {4,  2, 4, 0,  4,  2, 4, 0, 4,  2, 4, 0,  4,  0, 4, 0}, {4,  2, 4, 0,  4,  2, 4, 0, 4,  2, 4, 0,  4,  2, 4, 0}, {4,  2, 4, 2,  4,  2, 4, 0, 4,  2, 4, 0,  4,  2, 4, 0}, {4,  2, 4, 2,  4,  2, 4, 2, 4,  2, 4, 0,  4,  2, 4, 0}, {4,  2, 4, 2,  4,  2, 4, 2, 4,  2, 4, 2,  4,  2, 4, 0}, {4,  2, 4, 2,  4,  2, 4, 2, 4,  2, 4, 2,  4,  2, 4, 2}, {4,  2, 4, 2,  4,  2, 4, 2, 4,  4, 4, 4,  4,  2, 4, 2}, {2,  2, 2, 2,  4,  2, 4, 2, 4,  4, 4, 4,  2,  2, 2, 2}},
		{{18, 0, 0, 18, 0,  0, 12,0, 0,  0, 0, 0,  0,  0, 0, 0}, {18, 0, 0, 18, 0,  0, 12,0, 18, 0, 0, 18, 0,  0, 12,0}, {18, 0, 0, 18, 0,  0, 12,0, 18, 0, 0, 18, 0,  0, 12,0}, {18, 0, 0, 18, 0,  0, 12,0, 18, 0, 0, 18, 0,  0, 12,0}, {18, 0, 0, 18, 0,  0, 12,0, 18, 0, 0, 18, 0,  0, 12,0}, {12, 0, 6, 18, 0,  0, 12,0, 18, 0, 0, 18, 0,  0, 12,0}, {12, 0, 6, 12, 0,  6, 12,0, 18, 0, 0, 18, 0,  0, 12,0}, {12, 0, 6, 12, 0,  6, 12,0, 12, 0, 6, 18, 0,  0, 12,0}, {12, 0, 6, 12, 0,  6, 12,0, 12, 0, 6, 12, 0,  6, 12,0}, {12, 0, 6, 12, 0,  6, 12,0, 12, 0, 6, 12, 0,  6, 12,0}, {6,  6, 6, 12, 0,  6, 12,0, 12, 0, 6, 12, 0,  6, 12,0}, {6,  6, 6, 6,  6,  6, 12,0, 12, 0, 6, 12, 0,  6, 12,0}, {6,  6, 6, 6,  6,  6, 12,0, 6,  6, 6, 12, 0,  6, 12,0}, {6,  6, 6, 6,  6,  6, 12,0, 6,  6, 6, 6,  6,  6, 12,0}, {6,  6, 6, 6,  6,  6, 6, 6, 6,  6, 6, 6,  6,  6, 12,0}, {6,  4, 2, 6,  6,  6, 6, 6, 6,  6, 6, 6,  6,  6, 12,0}, {6,  4, 2, 6,  4,  2, 6, 6, 6,  6, 6, 6,  6,  6, 12,0}, {6,  4, 2, 6,  4,  2, 6, 6, 6,  4, 2, 6,  6,  6, 12,0}, {6,  4, 2, 6,  4,  2, 6, 6, 6,  4, 2, 6,  4,  2, 12,0}, {6,  4, 2, 6,  4,  2, 6, 6, 6,  4, 2, 6,  4,  2, 6, 6}},
		{{12, 0, 0, 0,  0,  0, 0, 0, 12, 0, 0, 0,  0,  0, 0, 0}, {12, 0, 0, 0,  0,  0, 0, 0, 6,  0, 6, 0,  0,  0, 0, 0}, {12, 0, 0, 0,  0,  0, 6, 0, 6,  0, 6, 0,  0,  0, 0, 0}, {6,  0, 6, 0,  0,  0, 6, 0, 6,  0, 6, 0,  0,  0, 0, 0}, {6,  0, 6, 0,  0,  0, 6, 0, 6,  0, 6, 0,  0,  0, 6, 0}, {6,  0, 6, 0,  6,  0, 6, 0, 6,  0, 6, 0,  0,  0, 6, 0}, {6,  0, 6, 0,  6,  0, 6, 0, 6,  0, 6, 0,  6,  0, 6, 0}, {4,  2, 6, 0,  6,  0, 6, 0, 6,  0, 6, 0,  6,  0, 6, 0}, {4,  2, 6, 0,  4,  2, 6, 0, 6,  0, 6, 0,  6,  0, 6, 0}, {4,  2, 6, 0,  4,  2, 6, 0, 4,  2, 6, 0,  6,  0, 6, 0}, {4,  2, 6, 0,  4,  2, 6, 0, 4,  2, 6, 0,  4,  2, 6, 0}, {2,  2, 2, 6,  4,  2, 6, 0, 4,  2, 6, 0,  4,  2, 6, 0}, {2,  2, 2, 6,  2,  2, 2, 6, 4,  2, 6, 0,  4,  2, 6, 0}, {2,  2, 2, 6,  2,  2, 2, 6, 2,  2, 2, 6,  4,  2, 6, 0}, {2,  2, 2, 6,  2,  2, 2, 6, 2,  2, 2, 6,  2,  2, 2, 6}, {2,  4, 2, 4,  2,  2, 2, 6, 2,  2, 2, 6,  2,  2, 2, 6}, {2,  4, 2, 4,  2,  4, 2, 4, 2,  2, 2, 6,  2,  2, 2, 6}, {2,  4, 2, 4,  2,  4, 2, 4, 2,  4, 2, 4,  2,  2, 2, 6}, {2,  4, 2, 4,  2,  4, 2, 4, 2,  4, 2, 4,  2,  4, 2, 4}, {2,  2, 2, 2,  2,  2, 2, 2, 2,  2, 2, 2,  2,  2, 2, 2}}
	};

	for (int bar = 0; bar < bars; ++bar) {
		auto& bs = barSettingsUI[slot][bar];
		int barStart = bar * ticksPerBar;
		if (barStart >= 1024) break;

		// ★ 型安全キャスト
		int cIdx = juce::jlimit<int>(0, 19, static_cast<int>(bs.cmplx) / 5);
		int eIdx = juce::jlimit<int>(0, 9, static_cast<int>(bs.entrp) / 10);
		int nextAvailableTick = -1;

		for (int step = 0; step < stepsPerBar; ++step) {
			int len = dnbRhythms[subStyle][cIdx][step % 16];
			if (len == 0) continue;

			int tick = barStart + step * 6;
			if (tick >= 1024 || tick < nextAvailableTick || isTickLocked(tick, slot)) continue;

			int row = 0, oct = 0;
			int vel = (step % 8 == 0) ? 120 : 95;
			vel = juce::jlimit<int>(1, 127, vel + random.nextInt(20) - 10);

			if (len >= 18) row = 0;
			else { if (eIdx == 0) row = 0; else if (eIdx == 1) row = (random.nextBool()) ? 0 : 4; else if (eIdx <= 3) { const int rows[] = { 0, 4, 2 }; row = rows[random.nextInt(3)]; } else if (eIdx <= 6) { const int rows[] = { 0, 2, 4, 6 }; row = rows[random.nextInt(4)]; } else row = random.nextInt(7); }
			if (len <= 6 && eIdx >= 5 && step % 2 != 0) { int octProb = (eIdx - 4) * 10; if (random.nextInt(100) < octProb) { oct = 1; vel += 10; } }
			if (eIdx >= 7 && step % 16 >= 12 && len <= 6) { if (random.nextInt(100) < (eIdx * 2)) { row = 8 + random.nextInt(4); vel -= 15; len = 2; oct = 0; } }

			auto& d = patternUI[slot][row][tick % 1024];
			d.velocity = vel;
			d.length = std::max(2, len);
			d.offset = 0;
			d.glide = false;
			d.staccato = (d.length <= 4);
			d.octave = juce::jlimit<int>(0, 1, oct);
			nextAvailableTick = tick + d.length;
		}
	}
}

// ==============================================================================
// 5. Trap
// ==============================================================================
void BassLineMatrixAudioProcessor::generateTrap(int slot, int bars, int numBeats, int ticksPerBar, const GenreDefinition& dna) {
	int subStyle = random.nextInt(4);
	int stepsPerBar = ticksPerBar / 6;

	static const int trapRhythms[4][20][16] = {
		{{24, 0, 0, 0,  0,  0, 0, 0,  12, 0, 0, 0,  0,  0, 0, 0}, {24, 0, 0, 0,  0,  0, 0, 0,  24, 0, 0, 0,  0,  0, 0, 0}, {24, 0, 0, 0,  0,  0, 0, 0,  12, 0, 0, 0,  6,  0, 6, 0}, {24, 0, 0, 0,  0,  0, 6, 0,  24, 0, 0, 0,  0,  0, 0, 0}, {24, 0, 0, 0,  0,  0, 6, 6,  24, 0, 0, 0,  0,  0, 0, 0}, {24, 0, 0, 0,  0,  0, 6, 0,  12, 0, 0, 0,  6,  0, 6, 0}, {24, 0, 0, 0,  0,  0, 12,0,  24, 0, 0, 0,  0,  0, 12,0}, {24, 0, 0, 0,  6,  0, 6, 0,  24, 0, 0, 0,  0,  0, 12,0}, {12, 0, 12,0,  0,  0, 6, 6,  24, 0, 0, 0,  0,  0, 12,0}, {24, 0, 0, 0,  0,  0, 12,0,  12, 0, 6, 0,  6,  0, 12,0}, {24, 0, 0, 0,  6,  0, 6, 6,  12, 0, 12,0,  0,  0, 6, 6}, {12, 0, 12,0,  6,  0, 6, 0,  24, 0, 0, 0,  6,  0, 6, 0}, {12, 0, 6, 6,  0,  0, 12,0,  12, 0, 6, 6,  0,  0, 12,0}, {12, 0, 6, 0,  6,  0, 6, 6,  12, 0, 12,0,  6,  0, 6, 6}, {12, 0, 6, 6,  6,  0, 6, 0,  12, 0, 6, 6,  6,  0, 6, 0}, {12, 0, 6, 0,  6,  0, 6, 6,  12, 0, 6, 0,  6,  6, 6, 6}, {12, 0, 6, 6,  6,  0, 6, 6,  12, 0, 6, 6,  6,  0, 6, 6}, {6,  0, 6, 6,  6,  0, 6, 6,  12, 0, 6, 0,  6,  6, 6, 6}, {6,  6, 6, 0,  6,  6, 6, 0,  12, 0, 6, 6,  6,  6, 6, 6}, {6,  6, 6, 6,  6,  6, 6, 6,  6,  6, 6, 6,  6,  6, 6, 6}},
		{{24, 0, 0, 0,  0,  0, 0, 0,  0,  0, 24,0,  0,  0, 0, 0}, {24, 0, 0, 0,  0,  0, 0, 0,  0,  0, 12,0,  12, 0, 0, 0}, {18, 0, 0, 18, 0,  0, 12,0,  0,  0, 0, 0,  0,  0, 0, 0}, {18, 0, 0, 18, 0,  0, 12,0,  18, 0, 0, 18, 0,  0, 12,0}, {24, 0, 0, 0,  12, 0, 0, 0,  0,  0, 12,0,  12, 0, 0, 0}, {12, 0, 0, 12, 0,  0, 24,0,  0,  0, 12,0,  12, 0, 0, 0}, {12, 0, 12,0,  0,  0, 12,0,  0,  0, 12,0,  0,  0, 12,0}, {12, 0, 0, 12, 0,  0, 12,0,  12, 0, 0, 12, 0,  0, 12,0}, {12, 0, 6, 0,  6,  0, 12,0,  0,  0, 12,0,  12, 0, 0, 0}, {12, 0, 6, 0,  12, 0, 12,0,  0,  0, 12,0,  6,  0, 6, 0}, {6,  0, 6, 0,  12, 0, 12,0,  6,  0, 12,0,  6,  0, 12,0}, {6,  0, 6, 0,  6,  0, 6, 0,  12, 0, 12,0,  12, 0, 0, 0}, {12, 0, 6, 6,  12, 0, 0, 0,  12, 0, 6, 6,  12, 0, 0, 0}, {12, 0, 6, 6,  6,  0, 6, 6,  12, 0, 6, 6,  6,  0, 6, 6}, {6,  6, 6, 0,  12, 0, 12,0,  6,  6, 6, 0,  12, 0, 12,0}, {6,  0, 6, 6,  6,  0, 6, 6,  12, 0, 12,0,  6,  6, 6, 6}, {6,  6, 6, 6,  12, 0, 12,0,  6,  6, 6, 6,  12, 0, 12,0}, {6,  6, 6, 6,  6,  6, 6, 6,  12, 0, 12,0,  6,  6, 6, 6}, {6,  6, 6, 6,  6,  6, 6, 6,  6,  6, 6, 6,  6,  6, 6, 6}, {4,  2, 4, 2,  4,  2, 4, 2,  4,  2, 4, 2,  4,  2, 4, 2}},
		{{96, 0, 0, 0,  0,  0, 0, 0,  0,  0, 0, 0,  0,  0, 0, 0}, {72, 0, 0, 0,  0,  0, 0, 0,  0,  0, 0, 0,  12, 0, 12,0}, {48, 0, 0, 0,  0,  0, 0, 0,  48, 0, 0, 0,  0,  0, 0, 0}, {48, 0, 0, 0,  0,  0, 0, 0,  24, 0, 0, 0,  24, 0, 0, 0}, {36, 0, 0, 0,  0,  0, 12,0,  48, 0, 0, 0,  0,  0, 0, 0}, {36, 0, 0, 0,  0,  0, 12,0,  24, 0, 0, 0,  12, 0, 12,0}, {48, 0, 0, 0,  0,  0, 0, 0,  24, 0, 0, 0,  12, 0, 6, 6}, {24, 0, 0, 0,  24, 0, 0, 0,  48, 0, 0, 0,  0,  0, 0, 0}, {24, 0, 0, 0,  12, 0, 12,0,  48, 0, 0, 0,  0,  0, 0, 0}, {24, 0, 0, 0,  12, 0, 12,0,  24, 0, 0, 0,  24, 0, 0, 0}, {36, 0, 0, 0,  0,  0, 12,0,  12, 0, 12,0,  24, 0, 0, 0}, {24, 0, 0, 0,  12, 0, 6, 6,  48, 0, 0, 0,  0,  0, 0, 0}, {24, 0, 0, 0,  12, 0, 6, 6,  24, 0, 0, 0,  12, 0, 12,0}, {12, 0, 12,0,  24, 0, 0, 0,  12, 0, 12,0,  24, 0, 0, 0}, {24, 0, 0, 0,  12, 0, 12,0,  12, 0, 12,0,  12, 0, 12,0}, {12, 0, 12,0,  12, 0, 12,0,  24, 0, 0, 0,  6,  6, 6, 6}, {24, 0, 0, 0,  6,  6, 6, 6,  48, 0, 0, 0,  0,  0, 0, 0}, {24, 0, 0, 0,  6,  6, 6, 6,  24, 0, 0, 0,  12, 0, 12,0}, {12, 0, 12,0,  6,  6, 6, 6,  12, 0, 12,0,  12, 0, 12,0}, {12, 0, 6, 6,  12, 0, 6, 6,  12, 0, 6, 6,  12, 0, 6, 6}},
		{{12, 0, 0, 0,  12, 0, 0, 0,  12, 0, 0, 0,  12, 0, 0, 0}, {12, 0, 0, 0,  6,  0, 6, 0,  12, 0, 0, 0,  12, 0, 0, 0}, {12, 0, 0, 0,  6,  0, 6, 0,  12, 0, 0, 0,  6,  0, 6, 0}, {12, 0, 6, 0,  12, 0, 0, 0,  12, 0, 6, 0,  12, 0, 0, 0}, {12, 0, 6, 0,  6,  0, 6, 0,  12, 0, 6, 0,  6,  0, 6, 0}, {6,  0, 6, 0,  6,  0, 12,0,  6,  0, 6, 0,  6,  0, 12,0}, {6,  0, 12,0,  6,  0, 12,0,  6,  0, 12,0,  6,  0, 12,0}, {12, 0, 6, 6,  12, 0, 0, 0,  12, 0, 6, 6,  12, 0, 0, 0}, {6,  0, 6, 6,  6,  0, 12,0,  6,  0, 6, 6,  6,  0, 12,0}, {6,  0, 6, 6,  6,  0, 6, 0,  6,  0, 6, 6,  6,  0, 6, 0}, {6,  6, 12,0,  6,  6, 12,0,  6,  6, 12,0,  6,  6, 12,0}, {12, 0, 6, 6,  6,  0, 6, 6,  12, 0, 6, 6,  6,  0, 6, 6}, {6,  6, 6, 6,  12, 0, 12,0,  6,  6, 6, 6,  12, 0, 12,0}, {6,  6, 6, 6,  6,  0, 6, 0,  6,  6, 6, 6,  6,  0, 6, 0}, {6,  6, 6, 6,  6,  6, 6, 6,  12, 0, 12,0,  12, 0, 12,0}, {6,  6, 6, 6,  6,  6, 6, 6,  6,  6, 6, 6,  6,  6, 6, 6}, {4,  2, 6, 0,  4,  2, 6, 0,  4,  2, 6, 0,  4,  2, 6, 0}, {4,  2, 4, 2,  12, 0, 12,0,  4,  2, 4, 2,  12, 0, 12,0}, {4,  2, 4, 2,  4,  2, 4, 2,  4,  2, 4, 2,  4,  2, 4, 2}, {2,  2, 2, 2,  2,  2, 2, 2,  2,  2, 2, 2,  2,  2, 2, 2}}
	};

	for (int bar = 0; bar < bars; ++bar) {
		auto& bs = barSettingsUI[slot][bar];
		int barStart = bar * ticksPerBar;
		if (barStart >= 1024) break;

		// ★ 型安全キャスト
		int cmplxVal = static_cast<int>(bs.cmplx);
		int eIdx = juce::jlimit<int>(0, 9, static_cast<int>(bs.entrp) / 10);
		int cIdx = juce::jlimit<int>(0, 19, cmplxVal / 5);
		int nextAvailableTick = -1;

		for (int step = 0; step < stepsPerBar; ++step) {
			int len = trapRhythms[subStyle][cIdx][step % 16];
			if (len == 0) continue;

			int tick = barStart + step * 6;
			if (tick >= 1024 || tick < nextAvailableTick || isTickLocked(tick, slot)) continue;

			int row = 0, oct = 0, vel = 115;
			bool isDownbeat = (step % 4 == 0);

			if (isDownbeat || len >= 24) row = 0;
			else { if (eIdx <= 2) row = 0; else if (eIdx <= 5) row = (random.nextBool()) ? 0 : 7; else if (eIdx <= 7) { const int rows[] = { 0, 7, 1, 11 }; row = rows[random.nextInt(4)]; } else row = random.nextInt(12); }

			bool doGlide = false;
			if (!isDownbeat && len <= 12 && eIdx >= 2) { if (random.nextInt(100) < (eIdx * 8 + 20)) { oct = 1; doGlide = true; vel = 127; } }

			auto& d = patternUI[slot][row][tick % 1024];
			d.velocity = juce::jlimit<int>(1, 127, vel);
			d.length = std::max(2, len);
			d.offset = (!isDownbeat) ? random.nextInt(4) : 0;
			d.glide = doGlide || (random.nextInt(100) < static_cast<int>(bs.autoGlide));
			d.octave = juce::jlimit<int>(0, 1, oct);
			nextAvailableTick = tick + d.length;
		}
	}
}

// ==============================================================================
// 6. Footwork
// ==============================================================================
void BassLineMatrixAudioProcessor::generateFootwork(int slot, int bars, int numBeats, int ticksPerBar, const GenreDefinition& dna) {
	int subStyle = random.nextInt(4);
	int ticksPerBeat = ticksPerBar / numBeats;

	for (int bar = 0; bar < bars; ++bar) {
		auto& bs = barSettingsUI[slot][bar];
		int barStart = bar * ticksPerBar;
		if (barStart >= 1024) break;

		// ★ 型安全キャスト
		int cIdx = juce::jlimit<int>(0, 19, static_cast<int>(bs.cmplx) / 5);
		int eIdx = juce::jlimit<int>(0, 9, static_cast<int>(bs.entrp) / 10);
		int nextAvailableTick = -1;

		for (int t = 0; t < ticksPerBar; t += 2) {
			int tick = barStart + t;
			if (tick >= 1024 || tick < nextAvailableTick || isTickLocked(tick, slot)) continue;

			int beat = t / ticksPerBeat;
			bool isTriplet = (t % 8 == 0);
			bool is16th = (t % 6 == 0);

			if ((beat == 1 || beat == 3) && (t % ticksPerBeat) == 0) continue;

			bool hit = false; int len = 6;
			if (subStyle == 0) { if (isTriplet && (cIdx >= 10 || random.nextInt(20) < cIdx + 5)) { hit = true; len = 6; } }
			else if (subStyle == 1) { if (is16th && (cIdx >= 12 || random.nextInt(20) < cIdx + 8)) { hit = true; len = 4; } else if (cIdx > 15 && t % 3 == 0 && random.nextInt(100) < 30) { hit = true; len = 2; } }
			else if (subStyle == 2) { if (isTriplet && cIdx > 5) { hit = true; len = 5; } else if (is16th && !isTriplet && cIdx > 12) { hit = true; len = 3; } }
			else { if (t == 0) { hit = true; len = 22; } else if (beat == 2 && t % ticksPerBeat == 16 && cIdx > 10) { hit = true; len = 6; } }

			if (!hit) continue;

			int row = 0, oct = 0, vel = 100;
			vel = juce::jlimit<int>(1, 127, vel + random.nextInt(20) - 10);
			if (t % ticksPerBeat == 0) row = (eIdx < 4) ? 0 : (random.nextBool() ? 0 : 4);
			else { if (eIdx == 0) row = 0; else if (eIdx <= 2) row = (random.nextBool()) ? 0 : 4; else if (eIdx <= 5) row = (random.nextInt(100) < 50) ? 0 : 2; else row = random.nextInt(7); }
			if (!isTriplet && eIdx >= 6 && random.nextInt(100) < (eIdx * 3)) { oct = 1; vel -= 15; }
			if (beat == 3 && t >= (ticksPerBar * 3 / 4) && eIdx >= 8 && random.nextInt(100) < 20) { row = 8 + random.nextInt(4); vel -= 15; len = 2; oct = 0; }

			auto& d = patternUI[slot][row][tick % 1024];
			d.velocity = vel; d.length = std::max(2, len); d.offset = 0;
			if (len > 6 && random.nextInt(100) < static_cast<int>(bs.autoGlide)) d.glide = true; else d.glide = false;
			d.staccato = (d.length <= 4); d.octave = juce::jlimit<int>(0, 1, oct);
			nextAvailableTick = tick + d.length;
		}
	}
}

// ==============================================================================
// ★ 独立アルゴリズム：07. IDM (Intelligent Jazz-Bass / Syncopation Ver.)
// ==============================================================================
void BassLineMatrixAudioProcessor::generateIDM(int slot, int bars, int numBeats, int ticksPerBar, const GenreDefinition& dna) {
	int subStyle = random.nextInt(4);
	int ticksPerBeat = ticksPerBar / numBeats;
	int stepsPerBeat = ticksPerBeat / 6;

	for (int bar = 0; bar < bars; ++bar) {
		auto& bs = barSettingsUI[slot][bar];
		int barStart = bar * ticksPerBar;
		if (barStart >= 1024) break;

		for (int beat = 0; beat < numBeats; ++beat) {
			int scaleOutProb = 0;
			if (beat == 0) scaleOutProb = 5;
			else if (beat == 1 || beat == 2) scaleOutProb = 10;
			else if (beat == 3) scaleOutProb = 15;

			bool isSnareBeat = (beat == 1 || beat == 3);
			int restStartTick = -1;
			int restEndTick = -1;

			if (isSnareBeat) {
				int restType = random.nextInt(100);
				if (restType < 20) { restStartTick = 0; restEndTick = ticksPerBeat; }
				else if (restType < 60) { restStartTick = 0; restEndTick = ticksPerBeat / 2; }
				else { restStartTick = 0; restEndTick = 6; }
			}

			for (int step = 0; step < stepsPerBeat; ++step) {
				int tInBeat = step * 6;
				int t = beat * ticksPerBeat + tInBeat;
				int tick = barStart + t;

				if (tick >= 1024 || isTickLocked(tick, slot)) continue;
				if (isSnareBeat && tInBeat >= restStartTick && tInBeat < restEndTick) continue;
				if (subStyle == 3 && beat == 0) continue;

				bool hit = false;
				int row = 0;
				int vel = 90;
				int len = 4;
				int oct = 0;
				int modStep = step % 4;

				if (subStyle == 0) {
					if (modStep == 0) { hit = true; vel = 110; len = 6; }
					else if (modStep == 2 && random.nextBool()) { hit = true; vel = 85; len = 4; }
					else if (modStep == 3 && random.nextInt(100) < static_cast<int>(bs.cmplx)) { hit = true; vel = 95; len = 5; }
				}
				else if (subStyle == 1) {
					if (modStep == 0 || modStep == 3) { hit = true; vel = 100; len = 5; }
					else if (random.nextInt(100) < (static_cast<int>(bs.cmplx) / 2)) { hit = true; vel = 75; len = 3; }
				}
				else if (subStyle == 2) {
					if (modStep == 0) { hit = true; vel = 105; len = 8; }
					else if (modStep == 2 && random.nextInt(100) < static_cast<int>(bs.entrp)) { hit = true; vel = 85; len = 2; }
				}
				else if (subStyle == 3) {
					if (modStep == 0) { hit = false; }
					else if (modStep == 2) { hit = true; vel = 100; len = 6; }
					else if (modStep == 3 && random.nextInt(100) < 85) { hit = true; vel = 115; len = 10; }
					else if (modStep == 1 && random.nextInt(100) < (static_cast<int>(bs.cmplx) / 2)) { hit = true; vel = 70; len = 3; }
				}

				if (hit) {
					int rVal = random.nextInt(100);
					if (modStep == 0) {
						row = (rVal < 75) ? 0 : 4;
					}
					else {
						if (rVal < 40) row = 0;
						else if (rVal < 70) row = 2;
						else row = 6;
					}

					if (row == 0 && random.nextInt(100) < 15) {
						oct = 1;
						vel += 10;
					}

					if (random.nextInt(100) < scaleOutProb) {
						row = 8 + random.nextInt(4);
						vel -= 15;
						len = 2;
					}

					auto& d = patternUI[slot][row][tick % 1024];

					len = std::max(2, len);

					// ★ 型推論エラー回避のために <int> を明示
					d.velocity = juce::jlimit<int>(1, 127, vel + random.nextInt(10) - 5);
					d.length = len;
					d.offset = 0;
					d.staccato = (len <= 4);
					d.octave = oct;

					if (len > 6 && random.nextInt(100) < static_cast<int>(bs.autoGlide)) {
						d.glide = true;
					}
					else {
						d.glide = false;
					}
				}
			}
		}
	}
}

// ==============================================================================
// 8. Dubstep
// ==============================================================================
void BassLineMatrixAudioProcessor::generateDubstep(int slot, int bars, int numBeats, int ticksPerBar, const GenreDefinition& dna) {
	int subStyle = random.nextInt(4);
	int stepsPerBar = ticksPerBar / 6;

	static const int dubRhythms[4][20][16] = {
		{{40, 0, 0, 0,  0, 0, 0, 0,  0, 0, 0, 0,  0, 0, 0, 0}, {40, 0, 0, 0,  0, 0, 0, 0,  0, 0, 0, 0,  12,0, 0, 0}, {40, 0, 0, 0,  0, 0, 0, 0,  0, 0, 0, 0,  12,0, 6, 0}, {40, 0, 0, 0,  0, 0, 0, 0,  0, 0, 0, 0,  6, 0, 6, 0}, {40, 0, 0, 0,  0, 0, 0, 0,  0, 0, 0, 0,  6, 6, 6, 0}, {40, 0, 0, 0,  0, 0, 0, 0,  0, 0, 0, 0,  4, 2, 4, 2}, {24, 0, 0, 0,  12,0, 0, 0,  0, 0, 0, 0,  12,0, 0, 0}, {24, 0, 0, 0,  12,0, 0, 0,  0, 0, 0, 0,  6, 0, 6, 0}, {24, 0, 0, 0,  12,0, 0, 0,  0, 0, 0, 0,  6, 6, 6, 0}, {24, 0, 0, 0,  6, 6, 0, 0,  0, 0, 0, 0,  6, 6, 6, 0}, {24, 0, 0, 0,  6, 6, 0, 0,  0, 0, 0, 0,  4, 2, 4, 2}, {12, 0, 6, 0,  6, 6, 0, 0,  0, 0, 0, 0,  12,0, 6, 0}, {12, 0, 6, 0,  6, 6, 0, 0,  0, 0, 0, 0,  6, 6, 6, 0}, {12, 0, 6, 0,  4, 2, 4, 2,  0, 0, 0, 0,  6, 6, 6, 0}, {12, 0, 6, 0,  4, 2, 4, 2,  0, 0, 0, 0,  4, 2, 4, 2}, {6,  6, 6, 0,  4, 2, 4, 2,  0, 0, 0, 0,  6, 6, 6, 0}, {6,  6, 6, 0,  4, 2, 4, 2,  0, 0, 0, 0,  4, 2, 4, 2}, {6,  4, 2, 0,  4, 2, 4, 2,  0, 0, 0, 0,  4, 2, 4, 2}, {4,  2, 4, 2,  4, 2, 4, 2,  0, 0, 0, 0,  4, 2, 4, 2}, {4,  2, 4, 2,  4, 2, 4, 2,  0, 0, 0, 0,  2, 2, 2, 2}},
		{{24, 0, 0, 0,  0, 0, 0, 0,  0, 0, 0, 0,  12,0, 0, 0}, {12, 0, 0, 0,  12,0, 0, 0,  0, 0, 0, 0,  12,0, 0, 0}, {12, 0, 0, 0,  12,0, 0, 0,  0, 0, 0, 0,  6, 0, 6, 0}, {12, 0, 0, 0,  12,0, 0, 0,  0, 0, 0, 0,  6, 6, 0, 0}, {12, 0, 0, 0,  12,0, 0, 0,  0, 0, 0, 0,  6, 6, 6, 0}, {12, 0, 0, 0,  6, 0, 6, 0,  0, 0, 0, 0,  6, 6, 6, 0}, {12, 0, 0, 0,  6, 0, 6, 0,  0, 0, 0, 0,  4, 4, 4, 0}, {12, 0, 0, 0,  6, 0, 6, 0,  0, 0, 0, 0,  4, 2, 4, 2}, {6,  0, 6, 0,  6, 0, 6, 0,  0, 0, 0, 0,  4, 2, 4, 2}, {6,  0, 6, 0,  6, 0, 6, 0,  0, 0, 0, 0,  2, 2, 2, 2}, {6,  6, 0, 0,  6, 0, 6, 0,  0, 0, 0, 0,  4, 2, 4, 2}, {6,  6, 0, 0,  6, 6, 0, 0,  0, 0, 0, 0,  4, 2, 4, 2}, {4,  2, 4, 0,  6, 6, 0, 0,  0, 0, 0, 0,  4, 2, 4, 2}, {4,  2, 4, 0,  4, 2, 4, 0,  0, 0, 0, 0,  4, 2, 4, 2}, {4,  2, 4, 2,  4, 2, 4, 0,  0, 0, 0, 0,  4, 2, 4, 2}, {4,  2, 4, 2,  4, 2, 4, 2,  0, 0, 0, 0,  4, 2, 4, 2}, {4,  2, 4, 2,  4, 2, 4, 2,  0, 0, 0, 0,  2, 2, 2, 2}, {2,  2, 2, 2,  4, 2, 4, 2,  0, 0, 0, 0,  2, 2, 2, 2}, {2,  2, 2, 2,  2, 2, 2, 2,  0, 0, 0, 0,  2, 2, 2, 2}, {2,  2, 2, 2,  2, 2, 2, 2,  0, 0, 0, 0,  2, 2, 2, 2}},
		{{12, 0, 0, 0,  12, 0, 0, 0, 0, 0, 0, 0,  12, 0, 0, 0}, {12, 0, 0, 0,  6,  0, 6, 0, 0, 0, 0, 0,  12, 0, 0, 0}, {6,  0, 6, 0,  6,  0, 6, 0, 0, 0, 0, 0,  12, 0, 0, 0}, {6,  0, 6, 0,  6,  0, 6, 0, 0, 0, 0, 0,  6,  0, 6, 0}, {6,  0, 6, 0,  4,  2, 6, 0, 0, 0, 0, 0,  6,  0, 6, 0}, {4,  2, 6, 0,  4,  2, 6, 0, 0, 0, 0, 0,  6,  0, 6, 0}, {4,  2, 6, 0,  4,  2, 6, 0, 0, 0, 0, 0,  4,  2, 6, 0}, {4,  2, 4, 2,  4,  2, 6, 0, 0, 0, 0, 0,  4,  2, 6, 0}, {4,  2, 4, 2,  4,  2, 4, 2, 0, 0, 0, 0,  4,  2, 6, 0}, {4,  2, 4, 2,  4,  2, 4, 2, 0, 0, 0, 0,  4,  2, 4, 2}, {3,  3, 6, 0,  4,  2, 4, 2, 0, 0, 0, 0,  4,  2, 4, 2}, {3,  3, 6, 0,  3,  3, 6, 0, 0, 0, 0, 0,  4,  2, 4, 2}, {3,  3, 6, 0,  3,  3, 6, 0, 0, 0, 0, 0,  3,  3, 6, 0}, {3,  3, 3, 3,  3,  3, 6, 0, 0, 0, 0, 0,  3,  3, 6, 0}, {3,  3, 3, 3,  3,  3, 3, 3, 0, 0, 0, 0,  3,  3, 6, 0}, {3,  3, 3, 3,  3,  3, 3, 3, 0, 0, 0, 0,  3,  3, 3, 3}, {2,  2, 2, 6,  3,  3, 3, 3, 0, 0, 0, 0,  3,  3, 3, 3}, {2,  2, 2, 6,  2,  2, 2, 6, 0, 0, 0, 0,  3,  3, 3, 3}, {2,  2, 2, 6,  2,  2, 2, 6, 0, 0, 0, 0,  2,  2, 2, 6}, {2,  2, 2, 2,  2,  2, 2, 2, 0, 0, 0, 0,  2,  2, 2, 2}},
		{{12, 0, 0, 0,  0, 0, 12, 0, 0, 0, 0, 0,  0, 0, 12, 0}, {12, 0, 0, 0,  0, 0, 6,  6, 0, 0, 0, 0,  0, 0, 12, 0}, {6,  0, 6, 0,  0, 0, 12, 0, 0, 0, 0, 0,  0, 0, 12, 0}, {6,  0, 6, 0,  0, 0, 6,  6, 0, 0, 0, 0,  0, 0, 12, 0}, {6,  0, 6, 0,  0, 0, 6,  6, 0, 0, 0, 0,  6, 0, 6,  0}, {6,  0, 4, 2,  0, 0, 6,  6, 0, 0, 0, 0,  6, 0, 6,  0}, {6,  0, 4, 2,  0, 0, 6,  6, 0, 0, 0, 0,  6, 0, 4,  2}, {4,  2, 4, 2,  0, 0, 6,  6, 0, 0, 0, 0,  6, 0, 4,  2}, {4,  2, 4, 2,  0, 6, 6,  0, 0, 0, 0, 0,  6, 0, 4,  2}, {4,  2, 4, 2,  0, 6, 6,  0, 0, 0, 0, 0,  4, 2, 4,  2}, {4,  2, 4, 2,  0, 6, 4,  2, 0, 0, 0, 0,  4, 2, 4,  2}, {2,  4, 2, 4,  0, 6, 6,  0, 0, 0, 0, 0,  4, 2, 4,  2}, {2,  4, 2, 4,  0, 6, 4,  2, 0, 0, 0, 0,  4, 2, 4,  2}, {2,  4, 2, 4,  0, 6, 4,  2, 0, 0, 0, 0,  2, 4, 2,  4}, {2,  2, 4, 4,  0, 6, 4,  2, 0, 0, 0, 0,  2, 4, 2,  4}, {2,  2, 4, 4,  0, 4, 2,  6, 0, 0, 0, 0,  2, 4, 2,  4}, {2,  2, 4, 4,  0, 4, 2,  6, 0, 0, 0, 0,  2, 2, 4,  4}, {2,  2, 2, 6,  0, 4, 2,  6, 0, 0, 0, 0,  2, 2, 4,  4}, {2,  2, 2, 6,  0, 4, 2,  6, 0, 0, 0, 0,  2, 2, 2,  6}, {2,  2, 2, 6,  0, 2, 2,  8, 0, 0, 0, 0,  2, 2, 2,  6}}
	};

	for (int bar = 0; bar < bars; ++bar) {
		auto& bs = barSettingsUI[slot][bar];
		int barStart = bar * ticksPerBar;
		if (barStart >= 1024) break;

		int cIdx = juce::jlimit<int>(0, 19, static_cast<int>(bs.cmplx) / 5);
		int eIdx = juce::jlimit<int>(0, 9, static_cast<int>(bs.entrp) / 10);
		int nextAvailableTick = -1;

		for (int step = 0; step < stepsPerBar; ++step) {
			int len = dubRhythms[subStyle][cIdx][step % 16];
			if (len == 0) continue;

			int tick = barStart + step * 6;
			if (tick >= 1024 || tick < nextAvailableTick || isTickLocked(tick, slot)) continue;

			int row = 0, oct = 0, vel = (step % 16 == 0) ? 127 : 100 + random.nextInt(15);
			if (step % 16 == 0) row = 0;
			else if (step % 16 >= 12) {
				if (eIdx == 0) row = 0;
				else if (eIdx <= 2) row = (step % 2 == 0) ? 2 : 4;
				else if (eIdx <= 4) row = (step % 2 == 0) ? 4 : 6;
				else if (eIdx <= 7) { const int rows[] = { 0, 2, 4, 6 }; row = rows[random.nextInt(4)]; }
				else row = random.nextInt(7);
			}
			else { row = (eIdx < 5) ? 0 : 4; }

			if (step % 16 >= 13 && eIdx >= 6) {
				if (random.nextInt(100) < (eIdx - 5) * 10) { oct = 1; vel += 10; }
			}
			if (eIdx >= 8 && step % 16 == 15) {
				if (random.nextInt(100) < (eIdx * 3)) { row = 8 + random.nextInt(2); vel -= 15; len = 2; oct = 0; }
			}

			auto& d = patternUI[slot][row][tick % 1024];
			d.velocity = juce::jlimit<int>(1, 127, vel);
			d.length = std::max(2, len);
			d.offset = 0;

			if (len > 6 && random.nextInt(100) < static_cast<int>(bs.autoGlide)) d.glide = true;
			else d.glide = false;

			d.staccato = false;
			d.octave = juce::jlimit<int>(0, 1, oct);
			nextAvailableTick = tick + d.length;
		}
	}
}

// ==============================================================================
// 9. Afrobeat
// ==============================================================================
void BassLineMatrixAudioProcessor::generateAfrobeat(int slot, int bars, int numBeats, int ticksPerBar, const GenreDefinition& dna) {
	int subStyle = random.nextInt(4);
	int stepsPerBar = ticksPerBar / 6;

	static const int afroRhythms[4][20][16] = {
		{{0, 0, 12,0,  0, 0, 12,0,  0, 0, 12,0,  0, 0, 12,0}, {0, 0, 6, 0,  0, 0, 12,0,  0, 0, 6, 0,  0, 0, 12,0}, {0, 0, 6, 0,  6, 0, 12,0,  0, 0, 6, 0,  6, 0, 12,0}, {0, 0, 6, 6,  0, 0, 12,0,  0, 0, 6, 6,  0, 0, 12,0}, {0, 0, 6, 6,  6, 0, 6, 0,  0, 0, 6, 6,  6, 0, 6, 0}, {0, 6, 6, 0,  6, 0, 6, 0,  0, 6, 6, 0,  6, 0, 6, 0}, {0, 6, 4, 2,  6, 0, 6, 0,  0, 6, 4, 2,  6, 0, 6, 0}, {0, 6, 4, 2,  4, 2, 6, 0,  0, 6, 4, 2,  4, 2, 6, 0}, {0, 6, 4, 2,  4, 2, 4, 2,  0, 6, 4, 2,  4, 2, 4, 2}, {2, 4, 4, 2,  4, 2, 4, 2,  2, 4, 4, 2,  4, 2, 4, 2}, {2, 4, 4, 2,  4, 2, 6, 0,  2, 4, 4, 2,  4, 2, 6, 0}, {2, 4, 2, 4,  4, 2, 6, 0,  2, 4, 2, 4,  4, 2, 6, 0}, {2, 4, 2, 4,  2, 4, 2, 4,  2, 4, 2, 4,  2, 4, 2, 4}, {2, 4, 2, 4,  2, 2, 2, 6,  2, 4, 2, 4,  2, 2, 2, 6}, {0, 6, 2, 4,  2, 2, 2, 6,  0, 6, 2, 4,  2, 2, 2, 6}, {0, 4, 2, 6,  2, 2, 2, 6,  0, 4, 2, 6,  2, 2, 2, 6}, {2, 2, 2, 6,  2, 2, 2, 6,  2, 2, 2, 6,  2, 2, 2, 6}, {2, 2, 2, 2,  4, 2, 4, 2,  2, 2, 2, 2,  4, 2, 4, 2}, {2, 2, 2, 2,  2, 2, 2, 2,  4, 2, 4, 2,  4, 2, 4, 2}, {2, 2, 2, 2,  2, 2, 2, 2,  2, 2, 2, 2,  2, 2, 2, 2}},
		{{0, 0, 0, 0,  0, 0, 0, 0,  0, 0, 0, 0,  12,0, 0, 0}, {0, 0, 0, 0,  0, 0, 0, 0,  0, 0, 0, 0,  6, 0, 6, 0}, {0, 0, 0, 0,  0, 0, 0, 0,  0, 0, 0, 0,  4, 2, 6, 0}, {0, 0, 0, 0,  0, 0, 0, 0,  0, 0, 0, 0,  4, 2, 4, 2}, {0, 0, 0, 0,  0, 0, 12,0,  0, 0, 0, 0,  4, 2, 4, 2}, {0, 0, 0, 0,  0, 0, 6, 6,  0, 0, 0, 0,  4, 2, 4, 2}, {0, 0, 0, 0,  0, 0, 4, 2,  0, 0, 0, 0,  4, 2, 4, 2}, {0, 0, 0, 0,  0, 0, 4, 2,  0, 0, 0, 0,  2, 2, 2, 2}, {0, 0, 0, 0,  2, 2, 2, 2,  0, 0, 0, 0,  2, 2, 2, 2}, {0, 0, 0, 0,  2, 2, 2, 2,  0, 0, 0, 0,  2, 2, 4, 4}, {2, 4, 6, 0,  0, 0, 0, 0,  2, 4, 6, 0,  0, 0, 0, 0}, {2, 2, 2, 6,  0, 0, 0, 0,  2, 2, 2, 6,  0, 0, 0, 0}, {2, 2, 2, 2,  0, 0, 0, 0,  2, 2, 2, 2,  0, 0, 0, 0}, {2, 2, 2, 2,  0, 0, 0, 0,  2, 2, 2, 2,  4, 2, 6, 0}, {2, 2, 2, 2,  0, 0, 0, 0,  2, 2, 2, 2,  2, 2, 2, 2}, {2, 2, 2, 2,  0, 0, 2, 4,  2, 2, 2, 2,  0, 0, 2, 4}, {2, 2, 2, 2,  0, 0, 2, 2,  2, 2, 2, 2,  0, 0, 2, 2}, {2, 2, 2, 2,  2, 2, 0, 0,  2, 2, 2, 2,  2, 2, 0, 0}, {2, 2, 2, 2,  2, 2, 2, 2,  0, 0, 0, 0,  2, 2, 2, 2}, {0, 0, 2, 2,  0, 0, 2, 2,  0, 0, 2, 2,  2, 2, 2, 2}},
		{{18, 0, 0, 18, 0, 0, 12,0,  0, 0, 0, 0,  0, 0, 0, 0}, {18, 0, 0, 18, 0, 0, 12,0,  24,0, 0, 0,  0, 0, 0, 0}, {18, 0, 0, 18, 0, 0, 12,0,  18,0, 0, 18, 0, 0, 12,0}, {18, 0, 0, 18, 0, 0, 12,0,  18,0, 0, 18, 0, 0, 12,0}, {12, 0, 6, 18, 0, 0, 12,0,  18,0, 0, 18, 0, 0, 12,0}, {12, 0, 6, 12, 0, 6, 12,0,  18,0, 0, 18, 0, 0, 12,0}, {12, 0, 6, 12, 0, 6, 12,0,  12,0, 6, 18, 0, 0, 12,0}, {12, 0, 6, 12, 0, 6, 12,0,  12,0, 6, 12, 0, 6, 12,0}, {12, 0, 6, 12, 0, 6, 6, 6,  12,0, 6, 12, 0, 6, 12,0}, {12, 0, 6, 12, 0, 6, 6, 6,  12,0, 6, 12, 0, 6, 6, 6}, {6,  6, 6, 12, 0, 6, 6, 6,  12,0, 6, 12, 0, 6, 6, 6}, {6,  6, 6, 12, 0, 6, 6, 6,  6, 6, 6, 12, 0, 6, 6, 6}, {6,  6, 6, 6,  6, 6, 6, 6,  6, 6, 6, 12, 0, 6, 6, 6}, {6,  6, 6, 6,  6, 6, 6, 6,  6, 6, 6, 6,  6, 6, 6, 6}, {6,  4, 2, 6,  6, 6, 6, 6,  6, 6, 6, 6,  6, 6, 6, 6}, {6,  4, 2, 6,  4, 2, 6, 6,  6, 6, 6, 6,  6, 6, 6, 6}, {6,  4, 2, 6,  4, 2, 6, 6,  6, 4, 2, 6,  6, 6, 6, 6}, {6,  4, 2, 6,  4, 2, 6, 6,  6, 4, 2, 6,  4, 2, 6, 6}, {4,  2, 4, 2,  4, 2, 6, 6,  6, 4, 2, 6,  4, 2, 6, 6}, {4,  2, 4, 2,  4, 2, 4, 2,  4, 2, 4, 2,  4, 2, 4, 2}},
		{{18, 0, 0, 6,  0, 0, 0, 0,  18, 0, 0, 6,  0, 0, 0, 0}, {18, 0, 0, 6,  12,0, 0, 0,  18, 0, 0, 6,  0, 0, 0, 0}, {18, 0, 0, 6,  12,0, 0, 0,  18, 0, 0, 6,  12,0, 0, 0}, {18, 0, 0, 6,  12,0, 12,0,  18, 0, 0, 6,  12,0, 0, 0}, {18, 0, 0, 6,  12,0, 12,0,  18, 0, 0, 6,  12,0, 12,0}, {12, 0, 6, 6,  12,0, 12,0,  18, 0, 0, 6,  12,0, 12,0}, {12, 0, 6, 6,  12,0, 12,0,  12, 0, 6, 6,  12,0, 12,0}, {12, 0, 6, 6,  12,0, 6, 6,  12, 0, 6, 6,  12,0, 12,0}, {12, 0, 6, 6,  12,0, 6, 6,  12, 0, 6, 6,  12,0, 6, 6}, {6,  6, 6, 6,  12,0, 6, 6,  12, 0, 6, 6,  12,0, 6, 6}, {6,  6, 6, 6,  12,0, 6, 6,  6,  6, 6, 6,  12,0, 6, 6}, {6,  6, 6, 6,  6, 6, 6, 6,  6,  6, 6, 6,  12,0, 6, 6}, {6,  6, 6, 6,  6, 6, 6, 6,  6,  6, 6, 6,  6, 6, 6, 6}, {6,  4, 2, 6,  6, 6, 6, 6,  6,  6, 6, 6,  6, 6, 6, 6}, {6,  4, 2, 6,  6, 6, 6, 6,  6,  4, 2, 6,  6, 6, 6, 6}, {6,  4, 2, 6,  6, 4, 2, 6,  6,  4, 2, 6,  6, 6, 6, 6}, {6,  4, 2, 6,  6, 4, 2, 6,  6,  4, 2, 6,  6, 4, 2, 6}, {4,  2, 4, 2,  6, 4, 2, 6,  6,  4, 2, 6,  6, 4, 2, 6}, {4,  2, 4, 2,  4, 2, 4, 2,  4,  2, 4, 2,  6, 4, 2, 6}, {4,  2, 4, 2,  4, 2, 4, 2,  4,  2, 4, 2,  4, 2, 4, 2}}
	};

	for (int bar = 0; bar < bars; ++bar) {
		auto& bs = barSettingsUI[slot][bar];
		int barStart = bar * ticksPerBar;
		if (barStart >= 1024) break;

		// ★ 型安全キャスト
		int cIdx = juce::jlimit<int>(0, 19, static_cast<int>(bs.cmplx) / 5);
		int eIdx = juce::jlimit<int>(0, 9, static_cast<int>(bs.entrp) / 10);
		int nextAvailableTick = -1;

		for (int step = 0; step < stepsPerBar; ++step) {
			int len = afroRhythms[subStyle][cIdx][step % 16];
			if (len == 0) continue;

			int tick = barStart + step * 6;
			if (tick >= 1024 || tick < nextAvailableTick || isTickLocked(tick, slot)) continue;

			int row = 0, oct = 0, vel = (len <= 4) ? (70 + random.nextInt(15)) : (110 + random.nextInt(17));

			if (eIdx <= 2) row = (random.nextInt(100) < 80) ? 0 : 4;
			else if (eIdx <= 5) { const int rows[] = { 0, 2, 4 }; row = rows[random.nextInt(3)]; }
			else if (eIdx <= 8) { const int rows[] = { 0, 2, 3, 4, 6 }; row = rows[random.nextInt(5)]; }
			else row = random.nextInt(7);

			if (step % 2 != 0 && eIdx >= 5) { int octProb = (eIdx - 4) * 12; if (random.nextInt(100) < octProb) { oct = 1; vel += 15; } }
			if (eIdx >= 8 && step % 16 >= 14) { if (random.nextInt(100) < (eIdx * 3)) { row = 8 + random.nextInt(2); vel -= 10; len = 2; oct = 0; } }

			auto& d = patternUI[slot][row][tick % 1024];
			d.velocity = juce::jlimit<int>(1, 127, vel);
			d.length = std::max(2, len);
			d.offset = 0;
			if (len > 6 && random.nextInt(100) < static_cast<int>(bs.autoGlide)) d.glide = true;
			else d.glide = false;
			d.staccato = (d.length <= 4);
			d.octave = juce::jlimit<int>(0, 1, oct);
			nextAvailableTick = tick + d.length;
		}
	}
}

// ==============================================================================
// 10. Gqom
// ==============================================================================
void BassLineMatrixAudioProcessor::generateGqom(int slot, int bars, int numBeats, int ticksPerBar, const GenreDefinition& dna) {
	int subStyle = random.nextInt(4);
	int stepsPerBar = ticksPerBar / 6;

	static const int gqomRhythms[4][20][16] = {
		{{12, 0, 0, 0,  0, 0, 0, 0,  12, 0, 0, 0,  0, 0, 0, 0}, {12, 0, 0, 0,  12, 0, 0, 0,  12, 0, 0, 0,  12, 0, 0, 0}, {12, 0, 0, 0,  12, 0, 0, 0,  12, 0, 0, 0,  6,  0, 6, 0}, {12, 0, 0, 0,  12, 0, 0, 0,  6,  0, 6, 0,  6,  0, 6, 0}, {12, 0, 0, 0,  6,  0, 6, 0,  12, 0, 0, 0,  6,  0, 6, 0}, {6,  0, 6, 0,  6,  0, 6, 0,  12, 0, 0, 0,  6,  0, 6, 0}, {6,  0, 6, 0,  12, 0, 0, 0,  6,  0, 6, 0,  12, 0, 0, 0}, {12, 0, 0, 0,  6,  0, 0, 6,  12, 0, 0, 0,  6,  0, 0, 6}, {6,  0, 0, 6,  12, 0, 0, 0,  6,  0, 0, 6,  12, 0, 0, 0}, {6,  0, 0, 6,  6,  0, 0, 6,  6,  0, 0, 6,  6,  0, 0, 6}, {6,  0, 4, 2,  12, 0, 0, 0,  6,  0, 4, 2,  12, 0, 0, 0}, {6,  0, 4, 2,  6,  0, 0, 6,  6,  0, 4, 2,  6,  0, 0, 6}, {4,  2, 6, 0,  12, 0, 0, 0,  4,  2, 6, 0,  12, 0, 0, 0}, {4,  2, 6, 0,  6,  0, 0, 6,  4,  2, 6, 0,  6,  0, 0, 6}, {4,  2, 6, 0,  4,  2, 6, 0,  4,  2, 6, 0,  4,  2, 6, 0}, {4,  2, 4, 2,  12, 0, 0, 0,  4,  2, 4, 2,  12, 0, 0, 0}, {4,  2, 4, 2,  6,  0, 0, 6,  4,  2, 4, 2,  6,  0, 0, 6}, {4,  2, 4, 2,  4,  2, 4, 2,  4,  2, 4, 2,  4,  2, 4, 2}, {4,  2, 4, 2,  4,  2, 4, 2,  6,  6, 6, 6,  6,  6, 6, 6}, {2,  2, 2, 2,  4,  2, 4, 2,  2,  2, 2, 2,  4,  2, 4, 2}},
		{{12, 0, 0, 0,  0,  0, 12,0,  0,  0, 0, 0,  0,  0, 12,0}, {12, 0, 0, 0,  0,  0, 12,0,  0,  0, 12,0,  0,  0, 0, 0}, {12, 0, 0, 0,  0,  0, 0, 0,  12, 0, 0, 0,  0,  0, 12,0}, {0,  0, 12,0,  12, 0, 0, 0,  0,  0, 12,0,  12, 0, 0, 0}, {0,  0, 12,0,  0,  0, 12,0,  0,  0, 12,0,  0,  0, 12,0}, {6,  0, 0, 6,  0,  0, 12,0,  6,  0, 0, 6,  0,  0, 12,0}, {0,  0, 6, 6,  12, 0, 0, 0,  0,  0, 6, 6,  12, 0, 0, 0}, {6,  0, 6, 0,  0,  0, 12,0,  0,  0, 12,0,  6,  0, 6, 0}, {12, 0, 0, 0,  6,  0, 6, 0,  0,  0, 12,0,  6,  0, 6, 0}, {0,  0, 12,0,  6,  0, 6, 0,  12, 0, 0, 0,  6,  0, 6, 0}, {6,  0, 4, 2,  0,  0, 12,0,  6,  0, 4, 2,  0,  0, 12,0}, {0,  0, 12,0,  6,  0, 4, 2,  0,  0, 12,0,  6,  0, 4, 2}, {4,  2, 6, 0,  0,  0, 12,0,  4,  2, 6, 0,  0,  0, 12,0}, {0,  0, 12,0,  4,  2, 6, 0,  0,  0, 12,0,  4,  2, 6, 0}, {4,  2, 0, 6,  0,  0, 12,0,  4,  2, 0, 6,  0,  0, 12,0}, {0,  0, 12,0,  4,  2, 0, 6,  0,  0, 12,0,  4,  2, 0, 6}, {4,  2, 4, 2,  0,  0, 12,0,  4,  2, 4, 2,  0,  0, 12,0}, {0,  0, 12,0,  4,  2, 4, 2,  0,  0, 12,0,  4,  2, 4, 2}, {4,  2, 4, 2,  0,  6, 6, 0,  4,  2, 4, 2,  0,  6, 6, 0}, {0,  6, 6, 0,  4,  2, 4, 2,  0,  6, 6, 0,  4,  2, 4, 2}},
		{{48, 0, 0, 0,  0,  0, 0, 0,  48, 0, 0, 0,  0,  0, 0, 0}, {48, 0, 0, 0,  0,  0, 0, 0,  24, 0, 0, 0,  0,  0, 0, 0}, {24, 0, 0, 0,  0,  0, 0, 0,  48, 0, 0, 0,  0,  0, 0, 0}, {36, 0, 0, 0,  0,  0, 12,0,  36, 0, 0, 0,  0,  0, 12,0}, {36, 0, 0, 0,  0,  0, 12,0,  48, 0, 0, 0,  0,  0, 0, 0}, {24, 0, 0, 0,  24, 0, 0, 0,  48, 0, 0, 0,  0,  0, 0, 0}, {24, 0, 0, 0,  12, 0, 0, 0,  48, 0, 0, 0,  0,  0, 0, 0}, {24, 0, 0, 0,  12, 0, 12,0,  48, 0, 0, 0,  0,  0, 0, 0}, {48, 0, 0, 0,  0,  0, 0, 0,  24, 0, 0, 0,  12, 0, 12,0}, {24, 0, 0, 0,  6,  0, 6, 0,  48, 0, 0, 0,  0,  0, 0, 0}, {24, 0, 0, 0,  6,  0, 6, 0,  24, 0, 0, 0,  12, 0, 0, 0}, {24, 0, 0, 0,  12, 0, 0, 0,  24, 0, 0, 0,  6,  0, 6, 0}, {18, 0, 0, 6,  24, 0, 0, 0,  18, 0, 0, 6,  24, 0, 0, 0}, {24, 0, 0, 0,  18, 0, 0, 6,  24, 0, 0, 0,  18, 0, 0, 6}, {18, 0, 0, 6,  18, 0, 0, 6,  48, 0, 0, 0,  0,  0, 0, 0}, {24, 0, 0, 0,  6,  6, 6, 6,  48, 0, 0, 0,  0,  0, 0, 0}, {48, 0, 0, 0,  0,  0, 0, 0,  24, 0, 0, 0,  6,  6, 6, 6}, {24, 0, 0, 0,  6,  6, 6, 6,  24, 0, 0, 0,  6,  6, 6, 6}, {12, 0, 12,0,  6,  6, 6, 6,  48, 0, 0, 0,  0,  0, 0, 0}, {12, 0, 12,0,  6,  6, 6, 6,  12, 0, 12,0,  6,  6, 6, 6}},
		{{6,  0, 6, 0,  6,  0, 6, 0,  6,  0, 6, 0,  6,  0, 6, 0}, {6,  0, 6, 0,  12, 0, 0, 0,  6,  0, 6, 0,  12, 0, 0, 0}, {6,  0, 6, 0,  0,  0, 12,0,  6,  0, 6, 0,  0,  0, 12,0}, {6,  0, 0, 6,  6,  0, 0, 6,  6,  0, 0, 6,  6,  0, 0, 6}, {0,  6, 6, 0,  0,  6, 6, 0,  0,  6, 6, 0,  0,  6, 6, 0}, {4,  2, 6, 0,  4,  2, 6, 0,  4,  2, 6, 0,  4,  2, 6, 0}, {4,  2, 6, 0,  12, 0, 0, 0,  4,  2, 6, 0,  12, 0, 0, 0}, {4,  2, 6, 0,  0,  0, 12,0,  4,  2, 6, 0,  0,  0, 12,0}, {6,  0, 4, 2,  6,  0, 4, 2,  6,  0, 4, 2,  6,  0, 4, 2}, {6,  0, 4, 2,  12, 0, 0, 0,  6,  0, 4, 2,  12, 0, 0, 0}, {4,  2, 4, 2,  6,  0, 6, 0,  4,  2, 4, 2,  6,  0, 6, 0}, {4,  2, 4, 2,  12, 0, 0, 0,  4,  2, 4, 2,  12, 0, 0, 0}, {4,  2, 4, 2,  0,  0, 12,0,  4,  2, 4, 2,  0,  0, 12,0}, {4,  2, 4, 2,  4,  2, 4, 2,  4,  2, 4, 2,  4,  2, 4, 2}, {2,  4, 2, 4,  2,  4, 2, 4,  2,  4, 2, 4,  2,  4, 2, 4}, {2,  2, 2, 6,  2,  2, 2, 6,  2,  2, 2, 6,  2,  2, 2, 6}, {2,  2, 2, 6,  12, 0, 0, 0,  2,  2, 2, 6,  12, 0, 0, 0}, {6,  0, 0, 0,  2,  2, 2, 6,  6,  0, 0, 0,  2,  2, 2, 6}, {2,  2, 2, 2,  6,  0, 6, 0,  2,  2, 2, 2,  6,  0, 6, 0}, {2,  2, 2, 2,  2,  2, 2, 2,  2,  2, 2, 2,  2,  2, 2, 2}}
	};

	for (int bar = 0; bar < bars; ++bar) {
		auto& bs = barSettingsUI[slot][bar];
		int barStart = bar * ticksPerBar;
		if (barStart >= 1024) break;

		// ★ 型安全キャスト
		int cIdx = juce::jlimit<int>(0, 19, static_cast<int>(bs.cmplx) / 5);
		int eIdx = juce::jlimit<int>(0, 9, static_cast<int>(bs.entrp) / 10);
		int nextAvailableTick = -1;
		bool isFourthBar = ((bar + 1) % 4 == 0);

		for (int step = 0; step < stepsPerBar; ++step) {
			int len = gqomRhythms[subStyle][cIdx][step % 16];
			if (len == 0) continue;

			int tick = barStart + step * 6;
			if (tick >= 1024 || tick < nextAvailableTick || isTickLocked(tick, slot)) continue;

			int row = 0, oct = 0, vel = (step % 4 == 0) ? 127 : 100 + random.nextInt(20);

			if (step % 16 == 0 || len >= 18) row = 0;
			else {
				if (eIdx <= 2) row = 0;
				else if (eIdx <= 4) row = (random.nextInt(100) < 85) ? 0 : 8;
				else if (eIdx <= 7) { const int rows[] = { 0, 0, 8, 4, 2 }; row = rows[random.nextInt(5)]; }
				else { const int rows[] = { 0, 0, 8, 9, 4 }; row = rows[random.nextInt(5)]; }
			}

			if (eIdx >= 6 && step % 2 != 0 && len <= 6) {
				int octProb = (eIdx - 5) * 15;
				if (random.nextInt(100) < octProb) { oct = 1; vel += 10; }
			}
			if (isFourthBar && step % 16 >= 10) {
				int outProb = (cIdx * 3) + (eIdx * 4);
				if (random.nextInt(100) < outProb) { row = 8 + random.nextInt(4); vel -= 5; len = 2; oct = 0; }
			}

			auto& d = patternUI[slot][row][tick % 1024];
			d.velocity = juce::jlimit<int>(1, 127, vel);
			d.length = std::max(2, len);
			d.offset = 0;
			if (len > 6 && random.nextInt(100) < static_cast<int>(bs.autoGlide)) d.glide = true;
			else d.glide = false;
			d.staccato = (d.length <= 4);
			d.octave = juce::jlimit<int>(0, 1, oct);
			nextAvailableTick = tick + d.length;
		}
	}
}

// ==============================================================================
// 11. Amapiano
// ==============================================================================
void BassLineMatrixAudioProcessor::generateAmapiano(int slot, int bars, int numBeats, int ticksPerBar, const GenreDefinition& dna) {
	int subStyle = random.nextInt(4);
	int ticksPerBeat = ticksPerBar / numBeats;

	for (int bar = 0; bar < bars; ++bar) {
		auto& bs = barSettingsUI[slot][bar];
		int barStart = bar * ticksPerBar;
		if (barStart >= 1024) break;

		for (int t = 0; t < ticksPerBar; t += 6) {
			int tick = barStart + t;
			if (tick >= 1024 || isTickLocked(tick, slot)) continue;

			bool hit = false; int row = 0, vel = 100, len = 4, oct = 0;
			// ★ 型安全キャスト
			int prob = static_cast<int>(bs.cmplx);

			if (subStyle == 0) { if (t == 0 || t == 18 || t == 36 || t == 60 || t == 84) { prob = 100; len = 5; if (t == 0 || t == 60) vel = 120; } else prob = 10; }
			else if (subStyle == 1) { if (t >= 72) { prob = 90; len = 3; vel = 80 + (t - 72); } else if (t % ticksPerBeat == 0) { prob = 100; len = 5; } else prob = 20; }
			else if (subStyle == 2) { if (t % 12 == 6) { prob = 90; len = 3; } else if (t % 24 == 18) { prob = 70; len = 3; } else prob = 15; }
			else { if (t == 12 || t == 30 || t == 54 || t == 78 || t == 90) { prob = 100; vel = 125; len = 2; } else prob = 0; }

			if (random.nextInt(100) < prob) hit = true;
			if (hit) {
				int rVal = random.nextInt(100);
				if (subStyle == 2) { if (rVal < 40) row = 0; else if (rVal < 70) row = 1; else row = 2; }
				else { if (rVal < 70) row = 0; else if (rVal < 90) row = 4; else row = 1; }
				if (t % ticksPerBeat == 0 || (subStyle == 0 && t == 60)) oct = -1;
				if (t == 90 && random.nextInt(100) < static_cast<int>(bs.entrp)) { row = 8 + random.nextInt(4); vel = 60 + random.nextInt(20); len = 2; }

				auto& d = patternUI[slot][row][tick % 1024];
				d.velocity = juce::jlimit<int>(20, 127, vel + random.nextInt(15));
				d.length = juce::jlimit<int>(2, 5, len);
				d.staccato = true;
				if (len > 6 && random.nextInt(100) < static_cast<int>(bs.autoGlide)) d.glide = true;
				else d.glide = false;
				d.offset = 0;

				// ★ 型安全のためにキャスト
				d.octave = juce::jlimit<int>(-1, 1, oct);

				// ★ エラーだったlastNoteTickForSlotを削除 (このアルゴリズムでは使っていないため不要です)
			}
		}
	}
}

// ==============================================================================
// 12. Indian
// ==============================================================================
void BassLineMatrixAudioProcessor::generateIndian(int slot, int bars, int numBeats, int ticksPerBar, const GenreDefinition& dna) {
	int subStyle = random.nextInt(4);
	int stepsPerBar = ticksPerBar / 6;

	static const int desiRhythms[4][20][16] = {
		{{12, 0, 0, 6,  12, 0, 0, 6,  12, 0, 0, 6,  12, 0, 0, 6}, {12, 0, 0, 6,  6,  0, 6, 6,  12, 0, 0, 6,  6,  0, 6, 6}, {6,  0, 6, 6,  6,  0, 6, 6,  6,  0, 6, 6,  6,  0, 6, 6}, {6,  0, 6, 6,  6,  0, 6, 6,  6,  0, 6, 6,  6,  6, 6, 6}, {6,  0, 4, 2,  12, 0, 0, 6,  6,  0, 4, 2,  12, 0, 0, 6}, {6,  0, 4, 2,  6,  0, 6, 6,  6,  0, 4, 2,  6,  0, 6, 6}, {6,  0, 4, 2,  6,  0, 4, 2,  6,  0, 4, 2,  6,  0, 4, 2}, {6,  6, 6, 6,  6,  0, 6, 6,  6,  6, 6, 6,  6,  0, 6, 6}, {6,  6, 6, 6,  6,  6, 6, 6,  6,  6, 6, 6,  6,  6, 6, 6}, {4,  2, 6, 6,  4,  2, 6, 6,  4,  2, 6, 6,  4,  2, 6, 6}, {4,  2, 4, 2,  6,  0, 6, 6,  4,  2, 4, 2,  6,  0, 6, 6}, {4,  2, 4, 2,  4,  2, 4, 2,  4,  2, 4, 2,  4,  2, 4, 2}, {4,  2, 4, 2,  4,  2, 4, 2,  6,  6, 6, 6,  6,  6, 6, 6}, {4,  2, 4, 2,  6,  6, 6, 6,  4,  2, 4, 2,  6,  6, 6, 6}, {4,  2, 4, 2,  4,  2, 4, 2,  4,  2, 4, 2,  4,  2, 4, 2}, {2,  4, 2, 4,  2,  4, 2, 4,  2,  4, 2, 4,  2,  4, 2, 4}, {2,  2, 2, 6,  6,  0, 6, 6,  2,  2, 2, 6,  6,  0, 6, 6}, {2,  2, 2, 6,  2,  2, 2, 6,  2,  2, 2, 6,  2,  2, 2, 6}, {2,  2, 2, 6,  4,  2, 4, 2,  2,  2, 2, 6,  4,  2, 4, 2}, {2,  2, 2, 2,  2,  2, 2, 2,  2,  2, 2, 2,  2,  2, 2, 2}},
		{{18, 0, 0, 18, 0,  0, 12,0,  18, 0, 0, 18, 0,  0, 12,0}, {18, 0, 0, 18, 0,  0, 12,0,  12, 0, 6, 12, 0,  6, 12,0}, {12, 0, 6, 18, 0,  0, 12,0,  12, 0, 6, 18, 0,  0, 12,0}, {12, 0, 6, 12, 0,  6, 12,0,  12, 0, 6, 12, 0,  6, 12,0}, {12, 0, 6, 12, 0,  6, 12,0,  6,  6, 6, 12, 0,  6, 12,0}, {6,  6, 6, 18, 0,  0, 12,0,  6,  6, 6, 18, 0,  0, 12,0}, {6,  6, 6, 12, 0,  6, 12,0,  6,  6, 6, 12, 0,  6, 12,0}, {6,  6, 6, 12, 0,  6, 6, 6,  6,  6, 6, 12, 0,  6, 6, 6}, {6,  6, 6, 6,  6,  6, 12,0,  6,  6, 6, 6,  6,  6, 12,0}, {6,  6, 6, 6,  6,  6, 12,0,  6,  6, 6, 6,  6,  6, 6, 6}, {6,  6, 6, 6,  6,  6, 6, 6,  6,  6, 6, 6,  6,  6, 6, 6}, {4,  2, 6, 18, 0,  0, 12,0,  4,  2, 6, 18, 0,  0, 12,0}, {4,  2, 6, 12, 0,  6, 12,0,  4,  2, 6, 12, 0,  6, 12,0}, {4,  2, 6, 12, 0,  6, 6, 6,  4,  2, 6, 12, 0,  6, 6, 6}, {4,  2, 4, 2,  6,  6, 12,0,  4,  2, 4, 2,  6,  6, 12,0}, {4,  2, 4, 2,  4,  2, 12,0,  4,  2, 4, 2,  4,  2, 12,0}, {4,  2, 4, 2,  4,  2, 4, 2,  4,  2, 4, 2,  4,  2, 4, 2}, {4,  2, 4, 2,  4,  2, 4, 2,  2,  2, 2, 2,  12, 0, 12,0}, {2,  2, 2, 2,  6,  6, 12,0,  2,  2, 2, 2,  6,  6, 12,0}, {2,  2, 2, 2,  2,  2, 2, 2,  2,  2, 2, 2,  2,  2, 2, 2}},
		{{48, 0, 0, 0,  0, 0, 0, 0,  48, 0, 0, 0,  0, 0, 0, 0}, {48, 0, 0, 0,  0, 0, 0, 0,  24, 0, 0, 0,  12,0, 12,0}, {24, 0, 0, 0,  24,0, 0, 0,  24, 0, 0, 0,  24,0, 0, 0}, {36, 0, 0, 0,  0, 0, 12,0,  36, 0, 0, 0,  0, 0, 12,0}, {36, 0, 0, 0,  0, 0, 12,0,  24, 0, 0, 0,  12,0, 12,0}, {24, 0, 0, 0,  12,0, 12,0,  24, 0, 0, 0,  12,0, 12,0}, {24, 0, 0, 0,  12,0, 6, 6,  24, 0, 0, 0,  12,0, 6, 6}, {24, 0, 0, 0,  12,0, 6, 6,  12, 0, 12,0,  6, 6, 6, 6}, {12, 0, 12,0,  12,0, 6, 6,  12, 0, 12,0,  12,0, 6, 6}, {12, 0, 12,0,  6, 6, 6, 6,  12, 0, 12,0,  6, 6, 6, 6}, {12, 0, 6, 6,  6, 6, 6, 6,  12, 0, 6, 6,  6, 6, 6, 6}, {24, 0, 0, 0,  6, 0, 4, 2,  24, 0, 0, 0,  6, 0, 4, 2}, {12, 0, 12,0,  6, 0, 4, 2,  12, 0, 12,0,  6, 0, 4, 2}, {12, 0, 6, 6,  6, 0, 4, 2,  12, 0, 6, 6,  6, 0, 4, 2}, {6,  6, 6, 6,  6, 0, 4, 2,  6,  6, 6, 6,  6, 0, 4, 2}, {6,  6, 6, 6,  4, 2, 4, 2,  6,  6, 6, 6,  4, 2, 4, 2}, {24, 0, 0, 0,  4, 2, 4, 2,  24, 0, 0, 0,  4, 2, 4, 2}, {12, 0, 12,0,  4, 2, 4, 2,  12, 0, 12,0,  4, 2, 4, 2}, {4,  2, 4, 2,  4, 2, 4, 2,  4,  2, 4, 2,  4, 2, 4, 2}, {2,  2, 2, 2,  2, 2, 2, 2,  2,  2, 2, 2,  2, 2, 2, 2}},
		{{6,  0, 6, 0,  6,  0, 6, 0,  6,  0, 6, 0,  6,  0, 6, 0}, {6,  0, 6, 0,  6,  0, 6, 0,  6,  0, 6, 0,  6,  6, 6, 0}, {6,  0, 6, 0,  6,  6, 6, 0,  6,  0, 6, 0,  6,  6, 6, 0}, {6,  0, 6, 6,  6,  0, 6, 6,  6,  0, 6, 6,  6,  0, 6, 6}, {6,  6, 6, 6,  6,  0, 6, 0,  6,  6, 6, 6,  6,  0, 6, 0}, {6,  6, 6, 6,  6,  6, 6, 0,  6,  6, 6, 6,  6,  6, 6, 0}, {6,  6, 6, 6,  6,  6, 6, 6,  6,  6, 6, 6,  6,  0, 6, 0}, {6,  6, 6, 6,  6,  6, 6, 6,  6,  6, 6, 6,  6,  6, 6, 6}, {4,  2, 6, 6,  4,  2, 6, 6,  4,  2, 6, 6,  4,  2, 6, 6}, {6,  6, 4, 2,  6,  6, 4, 2,  6,  6, 4, 2,  6,  6, 4, 2}, {4,  2, 4, 2,  6,  6, 6, 6,  4,  2, 4, 2,  6,  6, 6, 6}, {6,  6, 6, 6,  4,  2, 4, 2,  6,  6, 6, 6,  4,  2, 4, 2}, {4,  2, 4, 2,  4,  2, 4, 2,  4,  2, 4, 2,  4,  2, 4, 2}, {4,  2, 4, 2,  4,  2, 4, 2,  6,  6, 6, 6,  6,  6, 6, 6}, {2,  4, 2, 4,  2,  4, 2, 4,  2,  4, 2, 4,  2,  4, 2, 4}, {2,  2, 2, 6,  2,  2, 2, 6,  2,  2, 2, 6,  2,  2, 2, 6}, {2,  2, 2, 6,  6,  6, 6, 6,  2,  2, 2, 6,  6,  6, 6, 6}, {2,  2, 2, 2,  4,  2, 4, 2,  2,  2, 2, 2,  4,  2, 4, 2}, {2,  2, 2, 2,  2,  2, 2, 2,  2,  2, 2, 2,  2,  2, 2, 2}, {2,  2, 2, 2,  2,  2, 2, 2,  2,  2, 2, 2,  2,  2, 2, 2}}
	};

	for (int bar = 0; bar < bars; ++bar) {
		auto& bs = barSettingsUI[slot][bar];
		int barStart = bar * ticksPerBar;
		if (barStart >= 1024) break;

		// ★ 型安全キャスト
		int cIdx = juce::jlimit<int>(0, 19, static_cast<int>(bs.cmplx) / 5);
		int eIdx = juce::jlimit<int>(0, 9, static_cast<int>(bs.entrp) / 10);
		int nextAvailableTick = -1;
		bool isFourthBar = ((bar + 1) % 4 == 0);

		for (int step = 0; step < stepsPerBar; ++step) {
			int len = desiRhythms[subStyle][cIdx][step % 16];
			if (len == 0) continue;

			int tick = barStart + step * 6;
			if (tick >= 1024 || tick < nextAvailableTick || isTickLocked(tick, slot)) continue;

			int row = 0, oct = 0, vel = (len <= 4) ? (75 + random.nextInt(15)) : (115 + random.nextInt(12));
			if (step % 16 == 0 || len >= 18) row = 0;
			else { if (eIdx <= 2) row = (random.nextBool()) ? 0 : 4; else if (eIdx <= 4) { const int rows[] = { 0, 1, 4 }; row = rows[random.nextInt(3)]; } else if (eIdx <= 6) { const int rows[] = { 0, 1, 2, 4, 5 }; row = rows[random.nextInt(5)]; } else row = random.nextInt(7); }
			if ((len <= 4 || step % 2 != 0) && eIdx >= 3) { if (random.nextInt(100) < (eIdx - 2) * 10) { oct = 1; vel += 15; } }
			if (isFourthBar && step % 16 >= 12) { if (random.nextInt(100) < (cIdx * 3 + eIdx * 4)) { row = 8 + random.nextInt(4); vel -= 10; len = 2; oct = 0; } }

			auto& d = patternUI[slot][row][tick % 1024];
			d.velocity = juce::jlimit<int>(1, 127, vel);
			d.length = std::max(2, len);
			d.offset = 0;
			if (len > 6 && random.nextInt(100) < static_cast<int>(bs.autoGlide)) d.glide = true;
			else d.glide = false;
			d.staccato = (d.length <= 4);
			d.octave = juce::jlimit<int>(0, 1, oct);
			nextAvailableTick = tick + d.length;
		}
	}
}

// ==============================================================================
// 13. Latin
// ==============================================================================
void BassLineMatrixAudioProcessor::generateLatin(int slot, int bars, int numBeats, int ticksPerBar, const GenreDefinition& dna) {
	int subStyle = random.nextInt(4);
	int stepsPerBar = ticksPerBar / 6;

	static const int latinRhythms[4][20][16] = {
		{{12, 0, 0, 12, 0,  0, 12,0,  12, 0, 0, 12, 0,  0, 12,0}, {12, 0, 0, 12, 0,  0, 12,0,  12, 0, 0, 12, 0,  0, 12,0}, {12, 0, 0, 6,  6,  0, 12,0,  12, 0, 0, 6,  6,  0, 12,0}, {12, 0, 0, 6,  6,  0, 12,0,  12, 0, 0, 6,  6,  0, 12,0}, {6,  0, 6, 6,  6,  0, 12,0,  6,  0, 6, 6,  6,  0, 12,0}, {6,  0, 6, 6,  6,  0, 12,0,  6,  0, 6, 6,  6,  0, 12,0}, {12, 0, 0, 12, 0,  0, 6, 6,  12, 0, 0, 12, 0,  0, 6, 6}, {12, 0, 0, 12, 0,  0, 6, 6,  12, 0, 0, 12, 0,  0, 6, 6}, {6,  0, 6, 12, 0,  0, 6, 6,  6,  0, 6, 12, 0,  0, 6, 6}, {6,  0, 6, 12, 0,  0, 6, 6,  6,  0, 6, 12, 0,  0, 6, 6}, {6,  6, 6, 12, 0,  0, 6, 6,  6,  6, 6, 12, 0,  0, 6, 6}, {6,  6, 6, 12, 0,  0, 6, 6,  6,  6, 6, 12, 0,  0, 6, 6}, {6,  0, 4, 2,  6,  0, 12,0,  6,  0, 4, 2,  6,  0, 12,0}, {6,  0, 4, 2,  6,  0, 12,0,  6,  0, 4, 2,  6,  0, 12,0}, {4,  2, 6, 6,  6,  0, 12,0,  4,  2, 6, 6,  6,  0, 12,0}, {4,  2, 6, 6,  6,  0, 12,0,  4,  2, 6, 6,  6,  0, 12,0}, {4,  2, 4, 2,  6,  0, 6, 6,  4,  2, 4, 2,  6,  0, 6, 6}, {4,  2, 4, 2,  6,  0, 6, 6,  4,  2, 4, 2,  6,  0, 6, 6}, {4,  2, 4, 2,  4,  2, 6, 6,  4,  2, 4, 2,  4,  2, 6, 6}, {4,  2, 4, 2,  4,  2, 4, 2,  4,  2, 4, 2,  4,  2, 4, 2}},
		{{0,  0, 0, 0,  0,  0, 12,0,  0,  0, 0, 0,  0,  0, 12,0}, {0,  0, 0, 0,  0,  0, 12,0,  0,  0, 0, 0,  0,  0, 12,0}, {0,  0, 0, 0,  0,  0, 12,0,  12, 0, 0, 0,  0,  0, 12,0}, {0,  0, 0, 0,  0,  0, 12,0,  12, 0, 0, 0,  0,  0, 12,0}, {0,  0, 0, 0,  6,  0, 12,0,  12, 0, 0, 0,  0,  0, 12,0}, {0,  0, 0, 0,  6,  0, 12,0,  12, 0, 0, 0,  0,  0, 12,0}, {0,  0, 0, 0,  6,  0, 12,0,  6,  0, 6, 0,  0,  0, 12,0}, {0,  0, 0, 0,  6,  0, 12,0,  6,  0, 6, 0,  0,  0, 12,0}, {0,  0, 12,0,  0,  0, 12,0,  0,  0, 12,0,  0,  0, 12,0}, {0,  0, 12,0,  0,  0, 12,0,  0,  0, 12,0,  0,  0, 12,0}, {0,  0, 6, 6,  0,  0, 12,0,  0,  0, 6, 6,  0,  0, 12,0}, {0,  0, 6, 6,  0,  0, 12,0,  0,  0, 6, 6,  0,  0, 12,0}, {6,  0, 0, 0,  6,  0, 12,0,  12, 0, 0, 0,  6,  0, 12,0}, {6,  0, 0, 0,  6,  0, 12,0,  12, 0, 0, 0,  6,  0, 12,0}, {4,  2, 0, 0,  6,  0, 12,0,  12, 0, 0, 0,  4,  2, 12,0}, {4,  2, 0, 0,  6,  0, 12,0,  12, 0, 0, 0,  4,  2, 12,0}, {4,  2, 0, 0,  4,  2, 12,0,  4,  2, 0, 0,  4,  2, 12,0}, {4,  2, 0, 0,  4,  2, 12,0,  4,  2, 0, 0,  4,  2, 12,0}, {4,  2, 4, 2,  4,  2, 12,0,  4,  2, 4, 2,  4,  2, 12,0}, {4,  2, 4, 2,  4,  2, 12,0,  4,  2, 4, 2,  4,  2, 12,0}},
		{{6,  0, 0, 6,  0,  6, 0, 0,  6,  0, 0, 6,  0,  6, 0, 0}, {6,  0, 0, 6,  0,  6, 0, 0,  6,  0, 0, 6,  0,  6, 0, 0}, {6,  0, 0, 6,  0,  6, 6, 0,  6,  0, 0, 6,  0,  6, 6, 0}, {6,  0, 0, 6,  0,  6, 6, 0,  6,  0, 0, 6,  0,  6, 6, 0}, {6,  0, 6, 6,  0,  6, 0, 0,  6,  0, 6, 6,  0,  6, 0, 0}, {6,  0, 6, 6,  0,  6, 0, 0,  6,  0, 6, 6,  0,  6, 0, 0}, {6,  0, 6, 6,  0,  6, 6, 0,  6,  0, 6, 6,  0,  6, 6, 0}, {6,  0, 6, 6,  0,  6, 6, 0,  6,  0, 6, 6,  0,  6, 6, 0}, {6,  6, 0, 6,  0,  6, 0, 0,  6,  6, 0, 6,  0,  6, 0, 0}, {6,  6, 0, 6,  0,  6, 0, 0,  6,  6, 0, 6,  0,  6, 0, 0}, {6,  6, 0, 6,  0,  6, 6, 0,  6,  6, 0, 6,  0,  6, 6, 0}, {6,  6, 0, 6,  0,  6, 6, 0,  6,  6, 0, 6,  0,  6, 6, 0}, {4,  2, 0, 6,  0,  6, 6, 0,  4,  2, 0, 6,  0,  6, 6, 0}, {4,  2, 0, 6,  0,  6, 6, 0,  4,  2, 0, 6,  0,  6, 6, 0}, {4,  2, 4, 2,  0,  6, 6, 0,  4,  2, 4, 2,  0,  6, 6, 0}, {4,  2, 4, 2,  0,  6, 6, 0,  4,  2, 4, 2,  0,  6, 6, 0}, {4,  2, 4, 2,  4,  2, 6, 0,  4,  2, 4, 2,  4,  2, 6, 0}, {4,  2, 4, 2,  4,  2, 6, 0,  4,  2, 4, 2,  4,  2, 6, 0}, {2,  2, 2, 6,  2,  2, 6, 0,  2,  2, 2, 6,  2,  2, 6, 0}, {2,  2, 2, 6,  2,  2, 6, 0,  2,  2, 2, 6,  2,  2, 6, 0}},
		{{12, 0, 0, 0,  0,  0, 12,0,  12, 0, 0, 0,  0,  0, 12,0}, {12, 0, 0, 0,  0,  0, 12,0,  12, 0, 0, 0,  0,  0, 12,0}, {12, 0, 0, 0,  6,  0, 12,0,  12, 0, 0, 0,  6,  0, 12,0}, {12, 0, 0, 0,  6,  0, 12,0,  12, 0, 0, 0,  6,  0, 12,0}, {6,  0, 6, 0,  6,  0, 12,0,  6,  0, 6, 0,  6,  0, 12,0}, {6,  0, 6, 0,  6,  0, 12,0,  6,  0, 6, 0,  6,  0, 12,0}, {6,  0, 6, 0,  0,  6, 12,0,  6,  0, 6, 0,  0,  6, 12,0}, {6,  0, 6, 0,  0,  6, 12,0,  6,  0, 6, 0,  0,  6, 12,0}, {6,  0, 0, 6,  6,  0, 12,0,  6,  0, 0, 6,  6,  0, 12,0}, {6,  0, 0, 6,  6,  0, 12,0,  6,  0, 0, 6,  6,  0, 12,0}, {6,  0, 4, 2,  6,  0, 12,0,  6,  0, 4, 2,  6,  0, 12,0}, {6,  0, 4, 2,  6,  0, 12,0,  6,  0, 4, 2,  6,  0, 12,0}, {4,  2, 6, 0,  6,  0, 12,0,  4,  2, 6, 0,  6,  0, 12,0}, {4,  2, 6, 0,  6,  0, 12,0,  4,  2, 6, 0,  6,  0, 12,0}, {4,  2, 4, 2,  6,  0, 12,0,  4,  2, 4, 2,  6,  0, 12,0}, {4,  2, 4, 2,  6,  0, 12,0,  4,  2, 4, 2,  6,  0, 12,0}, {4,  2, 4, 2,  4,  2, 12,0,  4,  2, 4, 2,  4,  2, 12,0}, {4,  2, 4, 2,  4,  2, 12,0,  4,  2, 4, 2,  4,  2, 12,0}, {2,  2, 2, 2,  4,  2, 12,0,  2,  2, 2, 2,  4,  2, 12,0}, {2,  2, 2, 2,  4,  2, 12,0,  2,  2, 2, 2,  4,  2, 12,0}}
	};

	for (int bar = 0; bar < bars; ++bar) {
		auto& bs = barSettingsUI[slot][bar];
		int barStart = bar * ticksPerBar;
		if (barStart >= 1024) break;

		// ★ 型安全キャスト
		int cIdx = juce::jlimit<int>(0, 19, static_cast<int>(bs.cmplx) / 5);
		int eIdx = juce::jlimit<int>(0, 9, static_cast<int>(bs.entrp) / 10);
		int nextAvailableTick = -1;
		bool isFourthBar = ((bar + 1) % 4 == 0);

		for (int step = 0; step < stepsPerBar; ++step) {
			int len = latinRhythms[subStyle][cIdx][step % 16];
			if (len == 0) continue;

			int tick = barStart + step * 6;
			if (tick >= 1024 || tick < nextAvailableTick || isTickLocked(tick, slot)) continue;

			int row = 0, oct = 0, vel = (len <= 4) ? (80 + random.nextInt(15)) : (115 + random.nextInt(12));
			if (step % 16 == 0 || len >= 12) row = 0;
			else { if (eIdx <= 2) row = (random.nextBool()) ? 0 : 4; else if (eIdx <= 4) { const int rows[] = { 0, 2, 4 }; row = rows[random.nextInt(3)]; } else if (eIdx <= 6) { const int rows[] = { 0, 2, 4, 6 }; row = rows[random.nextInt(4)]; } else row = random.nextInt(7); }
			if ((step % 2 != 0 || len <= 6) && eIdx >= 3) { if (random.nextInt(100) < (eIdx - 2) * 12) { oct = 1; vel += 10; } }
			if (isFourthBar && step % 16 >= 12) { if (random.nextInt(100) < (cIdx * 3 + eIdx * 4)) { row = 8 + random.nextInt(4); vel -= 10; len = 2; oct = 0; } }

			auto& d = patternUI[slot][row][tick % 1024];
			d.velocity = juce::jlimit<int>(1, 127, vel);
			d.length = std::max(2, len);
			d.offset = 0;
			if (len > 6 && random.nextInt(100) < static_cast<int>(bs.autoGlide)) d.glide = true;
			else d.glide = false;
			d.staccato = (d.length <= 4);
			d.octave = juce::jlimit<int>(0, 1, oct);
			nextAvailableTick = tick + d.length;
		}
	}
}

// ==============================================================================
// 14. Trance
// ==============================================================================
void BassLineMatrixAudioProcessor::generateTrance(int slot, int bars, int numBeats, int ticksPerBar, const GenreDefinition& dna) {
	int subStyle = random.nextInt(5);
	int stepsPerBar = ticksPerBar / 6;

	static const int tranceRhythms[5][20][16] = {
		{{0, 0, 4, 0,  0, 0, 4, 0,  0, 0, 4, 0,  0, 0, 4, 0}, {0, 4, 4, 0,  0, 4, 4, 0,  0, 4, 4, 0,  0, 4, 4, 0}, {0, 4, 4, 4,  0, 4, 4, 0,  0, 4, 4, 4,  0, 4, 4, 0}, {0, 4, 4, 4,  0, 4, 4, 4,  0, 4, 4, 4,  0, 4, 4, 4}, {0, 4, 4, 4,  0, 4, 4, 4,  0, 4, 4, 4,  0, 4, 4, 4}, {0, 4, 2, 4,  0, 4, 4, 4,  0, 4, 2, 4,  0, 4, 4, 4}, {0, 4, 2, 4,  0, 4, 2, 4,  0, 4, 2, 4,  0, 4, 2, 4}, {0, 6, 4, 4,  0, 6, 4, 4,  0, 6, 4, 4,  0, 6, 4, 4}, {0, 4, 4, 4,  0, 4, 4, 4,  0, 4, 4, 4,  0, 4, 4, 4}, {2, 4, 4, 4,  0, 4, 4, 4,  2, 4, 4, 4,  0, 4, 4, 4}, {2, 4, 4, 4,  2, 4, 4, 4,  2, 4, 4, 4,  2, 4, 4, 4}, {4, 4, 4, 4,  0, 4, 4, 4,  4, 4, 4, 4,  0, 4, 4, 4}, {4, 4, 4, 4,  4, 4, 4, 4,  4, 4, 4, 4,  4, 4, 4, 4}, {0, 4, 2, 4,  0, 4, 2, 4,  0, 4, 2, 4,  0, 4, 2, 4}, {0, 2, 2, 2,  0, 4, 4, 4,  0, 2, 2, 2,  0, 4, 4, 4}, {0, 2, 2, 2,  0, 2, 2, 2,  0, 2, 2, 2,  0, 2, 2, 2}, {0, 4, 4, 4,  0, 4, 4, 4,  0, 4, 4, 4,  0, 4, 2, 2}, {0, 4, 4, 4,  0, 4, 4, 4,  0, 4, 2, 2,  0, 4, 2, 2}, {0, 4, 2, 2,  0, 4, 2, 2,  0, 4, 2, 2,  0, 4, 2, 2}, {2, 2, 2, 2,  2, 2, 2, 2,  2, 2, 2, 2,  2, 2, 2, 2}},
		{{0, 4, 0, 4,  0, 4, 0, 4,  0, 4, 0, 4,  0, 4, 0, 4}, {0, 4, 2, 4,  0, 4, 0, 4,  0, 4, 2, 4,  0, 4, 0, 4}, {0, 4, 2, 4,  0, 4, 2, 4,  0, 4, 2, 4,  0, 4, 2, 4}, {0, 4, 2, 4,  0, 2, 4, 4,  0, 4, 2, 4,  0, 2, 4, 4}, {0, 6, 0, 4,  0, 6, 0, 4,  0, 6, 0, 4,  0, 6, 0, 4}, {0, 2, 2, 4,  0, 4, 4, 2,  0, 2, 2, 4,  0, 4, 4, 2}, {0, 4, 4, 4,  0, 2, 2, 4,  0, 4, 4, 4,  0, 2, 2, 4}, {0, 2, 4, 2,  0, 2, 4, 2,  0, 2, 4, 2,  0, 2, 4, 2}, {0, 2, 2, 4,  0, 2, 2, 4,  0, 2, 2, 4,  0, 2, 2, 4}, {2, 4, 2, 4,  0, 4, 2, 4,  2, 4, 2, 4,  0, 4, 2, 4}, {2, 4, 2, 4,  2, 4, 2, 4,  2, 4, 2, 4,  2, 4, 2, 4}, {0, 2, 2, 2,  0, 4, 4, 4,  0, 2, 2, 2,  0, 4, 4, 4}, {0, 4, 2, 2,  0, 4, 2, 2,  0, 4, 2, 2,  0, 4, 2, 2}, {0, 2, 2, 4,  0, 4, 2, 2,  0, 2, 2, 4,  0, 4, 2, 2}, {0, 2, 2, 2,  0, 2, 2, 2,  0, 2, 2, 2,  0, 2, 2, 2}, {4, 2, 4, 2,  0, 2, 2, 4,  4, 2, 4, 2,  0, 2, 2, 4}, {2, 2, 2, 2,  0, 4, 4, 4,  2, 2, 2, 2,  0, 4, 4, 4}, {2, 2, 2, 2,  2, 2, 2, 2,  0, 4, 4, 4,  0, 4, 4, 4}, {2, 2, 2, 2,  2, 2, 2, 2,  2, 2, 2, 2,  2, 2, 2, 2}, {2, 2, 2, 2,  2, 2, 2, 2,  2, 2, 2, 2,  2, 2, 2, 2}},
		{{0, 0, 10,0,  0, 0, 10,0,  0, 0, 10,0,  0, 0, 10,0}, {0, 0, 10,0,  0, 0, 10,0,  0, 0, 10,0,  0, 0, 10,0}, {0, 0, 10,0,  0, 0, 10,0,  0, 0, 10,0,  0, 0, 10,0}, {0, 0, 10,0,  0, 0, 10,0,  0, 0, 10,0,  0, 0, 10,0}, {0, 0, 10,0,  0, 0, 10,0,  0, 0, 10,0,  0, 0, 10,0}, {0, 0, 8, 4,  0, 0, 10,0,  0, 0, 10,0,  0, 0, 10,0}, {0, 0, 8, 4,  0, 0, 8, 4,  0, 0, 10,0,  0, 0, 10,0}, {0, 0, 8, 4,  0, 0, 8, 4,  0, 0, 8, 4,  0, 0, 8, 4}, {0, 0, 10,0,  0, 0, 10,0,  0, 0, 10,0,  0, 4, 8, 0}, {0, 0, 10,0,  0, 0, 10,0,  0, 4, 8, 0,  0, 4, 8, 0}, {0, 4, 8, 0,  0, 0, 10,0,  0, 4, 8, 0,  0, 0, 10,0}, {0, 4, 8, 0,  0, 4, 8, 0,  0, 4, 8, 0,  0, 4, 8, 0}, {0, 0, 8, 2,  0, 0, 8, 2,  0, 0, 8, 2,  0, 0, 8, 2}, {0, 2, 8, 0,  0, 0, 10,0,  0, 2, 8, 0,  0, 0, 10,0}, {0, 2, 8, 0,  0, 2, 8, 0,  0, 2, 8, 0,  0, 2, 8, 0}, {0, 0, 6, 6,  0, 0, 10,0,  0, 0, 6, 6,  0, 0, 10,0}, {0, 0, 6, 6,  0, 0, 6, 6,  0, 0, 6, 6,  0, 0, 6, 6}, {2, 0, 10,0,  0, 0, 10,0,  2, 0, 10,0,  0, 0, 10,0}, {2, 0, 10,0,  2, 0, 10,0,  2, 0, 10,0,  2, 0, 10,0}, {2, 2, 8, 0,  2, 2, 8, 0,  2, 2, 8, 0,  2, 2, 8, 0}},
		{{6, 0, 0, 6,  0, 6, 0, 0,  6, 0, 0, 6,  0, 6, 0, 0}, {6, 0, 0, 6,  0, 6, 0, 0,  6, 0, 0, 6,  0, 6, 4, 0}, {6, 0, 0, 6,  0, 6, 4, 0,  6, 0, 0, 6,  0, 6, 4, 0}, {6, 0, 4, 6,  0, 6, 0, 0,  6, 0, 4, 6,  0, 6, 0, 0}, {6, 0, 4, 6,  0, 6, 4, 0,  6, 0, 4, 6,  0, 6, 4, 0}, {4, 4, 0, 6,  0, 6, 0, 0,  4, 4, 0, 6,  0, 6, 0, 0}, {4, 4, 0, 6,  0, 6, 4, 0,  4, 4, 0, 6,  0, 6, 4, 0}, {4, 0, 4, 4,  0, 6, 0, 0,  4, 0, 4, 4,  0, 6, 0, 0}, {4, 0, 4, 4,  0, 6, 4, 0,  4, 0, 4, 4,  0, 6, 4, 0}, {4, 4, 4, 6,  0, 6, 0, 0,  4, 4, 4, 6,  0, 6, 0, 0}, {4, 4, 4, 6,  0, 6, 4, 0,  4, 4, 4, 6,  0, 6, 4, 0}, {6, 0, 0, 6,  4, 4, 4, 0,  6, 0, 0, 6,  4, 4, 4, 0}, {6, 0, 4, 6,  4, 4, 4, 0,  6, 0, 4, 6,  4, 4, 4, 0}, {4, 4, 0, 6,  4, 4, 4, 0,  4, 4, 0, 6,  4, 4, 4, 0}, {4, 0, 4, 4,  4, 4, 4, 0,  4, 0, 4, 4,  4, 4, 4, 0}, {4, 4, 4, 6,  4, 4, 4, 0,  4, 4, 4, 6,  4, 4, 4, 0}, {4, 4, 4, 6,  4, 4, 4, 4,  4, 4, 4, 6,  4, 4, 4, 4}, {4, 2, 2, 6,  0, 6, 4, 0,  4, 2, 2, 6,  0, 6, 4, 0}, {4, 2, 2, 6,  4, 2, 2, 4,  4, 2, 2, 6,  4, 2, 2, 4}, {2, 2, 2, 6,  2, 2, 2, 4,  2, 2, 2, 6,  2, 2, 2, 4}},
		{{6, 0, 6, 0,  6, 0, 6, 0,  6, 0, 6, 0,  6, 0, 6, 0}, {6, 0, 6, 0,  6, 0, 6, 0,  6, 0, 6, 0,  12,0, 0, 0}, {6, 0, 6, 0,  12,0, 0, 0,  6, 0, 6, 0,  12,0, 0, 0}, {6, 0, 4, 2,  6, 0, 6, 0,  6, 0, 4, 2,  6, 0, 6, 0}, {6, 0, 4, 2,  6, 0, 4, 2,  6, 0, 4, 2,  6, 0, 4, 2}, {4, 2, 6, 0,  6, 0, 6, 0,  4, 2, 6, 0,  6, 0, 6, 0}, {4, 2, 6, 0,  4, 2, 6, 0,  4, 2, 6, 0,  4, 2, 6, 0}, {4, 2, 4, 2,  6, 0, 6, 0,  4, 2, 4, 2,  6, 0, 6, 0}, {4, 2, 4, 2,  4, 2, 4, 2,  4, 2, 4, 2,  4, 2, 4, 2}, {6, 0, 6, 0,  6, 0, 6, 0,  6, 0, 6, 0,  4, 2, 4, 2}, {6, 0, 6, 0,  6, 0, 6, 0,  4, 2, 4, 2,  4, 2, 4, 2}, {6, 0, 4, 2,  6, 0, 4, 2,  4, 2, 4, 2,  4, 2, 4, 2}, {4, 2, 6, 0,  4, 2, 6, 0,  4, 2, 4, 2,  4, 2, 4, 2}, {4, 2, 4, 2,  4, 2, 4, 2,  4, 4, 4, 4,  4, 4, 4, 4}, {6, 0, 6, 0,  4, 4, 4, 4,  6, 0, 6, 0,  4, 4, 4, 4}, {4, 4, 4, 4,  4, 4, 4, 4,  4, 4, 4, 4,  4, 4, 4, 4}, {4, 2, 2, 4,  4, 2, 2, 4,  4, 2, 2, 4,  4, 2, 2, 4}, {2, 2, 4, 4,  2, 2, 4, 4,  2, 2, 4, 4,  2, 2, 4, 4}, {4, 4, 2, 2,  4, 4, 2, 2,  4, 4, 2, 2,  4, 4, 2, 2}, {2, 2, 2, 2,  2, 2, 2, 2,  2, 2, 2, 2,  2, 2, 2, 2}}
	};

	for (int bar = 0; bar < bars; ++bar) {
		auto& bs = barSettingsUI[slot][bar];
		int barStart = bar * ticksPerBar;
		if (barStart >= 1024) break;

		// ★ 型安全キャスト
		int cIdx = juce::jlimit<int>(0, 19, static_cast<int>(bs.cmplx) / 5);
		int eIdx = juce::jlimit<int>(0, 9, static_cast<int>(bs.entrp) / 10);
		int nextAvailableTick = -1;
		bool isFourthBar = ((bar + 1) % 4 == 0);

		for (int step = 0; step < stepsPerBar; ++step) {
			int len = tranceRhythms[subStyle][cIdx][step % 16];
			if (len == 0) continue;

			int tick = barStart + step * 6;
			if (tick >= 1024 || tick < nextAvailableTick || isTickLocked(tick, slot)) continue;

			int row = 0, oct = 0, vel = (step % 4 == 0) ? 90 : 120;
			vel = juce::jlimit<int>(1, 127, vel + random.nextInt(15) - 7);

			if (subStyle == 2) row = (eIdx < 7) ? 0 : (random.nextInt(100) < 80 ? 0 : 4);
			else if (subStyle == 4) { if (eIdx == 0) row = 0; else if (eIdx <= 3) row = (step % 2 == 0) ? 0 : 2; else if (eIdx <= 6) row = (step % 2 == 0) ? 0 : 4; else { const int rows[] = { 0, 2, 4, 6 }; row = rows[random.nextInt(4)]; } }
			else { if (eIdx <= 2) row = 0; else if (eIdx <= 5) row = (random.nextInt(100) < 80) ? 0 : 4; else if (eIdx <= 8) { if (random.nextInt(100) < 10) row = 8; else row = (random.nextBool()) ? 0 : 4; } else row = random.nextInt(7); }
			if (step % 2 != 0) { int octProb = (subStyle == 1) ? (eIdx + 1) * 8 : (eIdx >= 4 ? (eIdx - 3) * 5 : 0); if (random.nextInt(100) < octProb) { oct = 1; vel += 10; } }
			if (isFourthBar && step % 16 >= 12) { if (random.nextInt(100) < (cIdx * 4 + eIdx * 3)) { row = 8 + random.nextInt(4); vel -= 5; len = 2; oct = 0; } }

			auto& d = patternUI[slot][row][tick % 1024];
			d.velocity = juce::jlimit<int>(1, 127, vel);
			d.length = std::max(2, len);
			d.offset = 0;
			if (len > 6 && random.nextInt(100) < static_cast<int>(bs.autoGlide)) d.glide = true;
			else d.glide = false;
			d.staccato = (d.length <= 4);
			d.octave = juce::jlimit<int>(0, 1, oct);
			nextAvailableTick = tick + d.length;
		}
	}
}

// ==============================================================================
// 15. Synthwave
// ==============================================================================
void BassLineMatrixAudioProcessor::generateSynthwave(int slot, int bars, int numBeats, int ticksPerBar, const GenreDefinition& dna) {
	int subStyle = random.nextInt(4);
	int stepsPerBar = ticksPerBar / 6;

	static const int synthwaveRhythms[4][20][16] = {
		{{24, 0, 0, 0,  0,  0, 0, 0,  24, 0, 0, 0,  0,  0, 0, 0}, {24, 0, 0, 0,  24, 0, 0, 0,  24, 0, 0, 0,  24, 0, 0, 0}, {12, 0, 12,0,  12, 0, 12,0,  12, 0, 12,0,  12, 0, 12,0}, {12, 0, 12,0,  12, 0, 12,0,  12, 0, 12,0,  12, 0, 6, 6}, {12, 0, 6, 6,  12, 0, 6, 6,  12, 0, 6, 6,  12, 0, 6, 6}, {6,  6, 6, 6,  12, 0, 12,0,  6,  6, 6, 6,  12, 0, 12,0}, {6,  6, 6, 6,  6,  6, 6, 6,  12, 0, 12,0,  12, 0, 12,0}, {6,  6, 6, 6,  6,  6, 6, 6,  6,  6, 6, 6,  12, 0, 12,0}, {6,  6, 6, 6,  6,  6, 6, 6,  6,  6, 6, 6,  6,  6, 12,0}, {6,  6, 6, 6,  6,  6, 6, 6,  6,  6, 6, 6,  6,  6, 6, 6}, {6,  6, 6, 6,  6,  6, 6, 6,  6,  6, 6, 6,  6,  6, 6, 6}, {6,  6, 6, 6,  6,  6, 6, 6,  6,  6, 6, 6,  6,  6, 6, 6}, {6,  6, 6, 6,  6,  6, 6, 6,  6,  6, 6, 6,  6,  6, 6, 6}, {6,  6, 6, 6,  6,  6, 6, 6,  6,  6, 6, 6,  6,  6, 6, 6}, {6,  6, 6, 6,  6,  6, 6, 6,  6,  6, 6, 6,  6,  6, 6, 6}, {6,  6, 6, 6,  6,  6, 6, 6,  6,  6, 6, 6,  6,  6, 6, 6}, {6,  6, 6, 6,  6,  6, 6, 6,  6,  6, 6, 6,  6,  6, 6, 6}, {6,  6, 6, 6,  6,  6, 6, 6,  6,  6, 6, 6,  6,  6, 6, 6}, {6,  6, 6, 6,  6,  6, 6, 6,  6,  6, 6, 6,  6,  6, 6, 6}, {6,  6, 6, 6,  6,  6, 6, 6,  6,  6, 6, 6,  6,  6, 6, 6}},
		{{12, 0, 12,0,  12, 0, 12,0,  12, 0, 12,0,  12, 0, 12,0}, {12, 0, 12,0,  12, 0, 12,0,  12, 0, 12,0,  12, 0, 12,0}, {6,  0, 6, 0,  6,  0, 6, 0,  6,  0, 6, 0,  6,  0, 6, 0}, {6,  6, 6, 6,  12, 0, 12,0,  6,  6, 6, 6,  12, 0, 12,0}, {6,  6, 6, 6,  6,  6, 6, 6,  12, 0, 12,0,  12, 0, 12,0}, {6,  6, 6, 6,  6,  6, 6, 6,  6,  6, 6, 6,  6,  6, 6, 6}, {6,  6, 6, 6,  6,  6, 6, 6,  6,  6, 6, 6,  6,  6, 6, 6}, {6,  6, 6, 6,  6,  6, 6, 6,  6,  6, 6, 6,  6,  6, 6, 6}, {6,  6, 6, 6,  6,  6, 6, 6,  6,  6, 6, 6,  6,  6, 6, 6}, {6,  6, 6, 6,  6,  6, 6, 6,  6,  6, 6, 6,  6,  6, 6, 6}, {6,  6, 6, 6,  6,  6, 6, 6,  6,  6, 6, 6,  6,  6, 6, 6}, {6,  6, 6, 6,  6,  6, 6, 6,  6,  6, 6, 6,  6,  6, 6, 6}, {6,  6, 6, 6,  6,  6, 6, 6,  6,  6, 6, 6,  6,  6, 6, 6}, {6,  6, 6, 6,  6,  6, 6, 6,  6,  6, 6, 6,  6,  6, 6, 6}, {6,  6, 6, 6,  6,  6, 6, 6,  6,  6, 6, 6,  6,  6, 6, 6}, {6,  6, 6, 6,  6,  6, 6, 6,  6,  6, 6, 6,  6,  6, 6, 6}, {6,  6, 6, 6,  6,  6, 6, 6,  6,  6, 6, 6,  6,  6, 6, 6}, {6,  6, 6, 6,  6,  6, 6, 6,  6,  6, 6, 6,  6,  6, 6, 6}, {6,  6, 6, 6,  6,  6, 6, 6,  6,  6, 6, 6,  6,  6, 6, 6}, {6,  6, 6, 6,  6,  6, 6, 6,  6,  6, 6, 6,  6,  6, 6, 6}},
		{{18, 0, 0, 6,  18, 0, 0, 6,  18, 0, 0, 6,  18, 0, 0, 6}, {18, 0, 0, 6,  18, 0, 0, 6,  18, 0, 0, 6,  18, 0, 0, 6}, {18, 0, 0, 6,  12, 0, 6, 6,  18, 0, 0, 6,  12, 0, 6, 6}, {12, 6, 6, 0,  12, 6, 6, 0,  12, 6, 6, 0,  12, 6, 6, 0}, {6,  6, 12,0,  6,  6, 12,0,  6,  6, 12,0,  6,  6, 12,0}, {6,  6, 6, 6,  12, 0, 6, 0,  6,  6, 6, 6,  12, 0, 6, 0}, {6,  6, 6, 6,  6,  6, 6, 6,  12, 0, 12,0,  12, 0, 12,0}, {6,  6, 6, 6,  6,  6, 6, 6,  6,  6, 6, 6,  6,  6, 6, 6}, {6,  6, 6, 6,  6,  6, 6, 6,  6,  6, 6, 6,  6,  6, 6, 6}, {6,  6, 6, 6,  6,  6, 6, 6,  6,  6, 6, 6,  6,  6, 6, 6}, {6,  6, 6, 6,  6,  6, 6, 6,  6,  6, 6, 6,  6,  6, 6, 6}, {6,  6, 6, 6,  6,  6, 6, 6,  6,  6, 6, 6,  6,  6, 6, 6}, {6,  6, 6, 6,  6,  6, 6, 6,  6,  6, 6, 6,  6,  6, 6, 6}, {6,  6, 6, 6,  6,  6, 6, 6,  6,  6, 6, 6,  6,  6, 6, 6}, {6,  6, 6, 6,  6,  6, 6, 6,  6,  6, 6, 6,  6,  6, 6, 6}, {6,  6, 6, 6,  6,  6, 6, 6,  6,  6, 6, 6,  6,  6, 6, 6}, {6,  6, 6, 6,  6,  6, 6, 6,  6,  6, 6, 6,  6,  6, 6, 6}, {6,  6, 6, 6,  6,  6, 6, 6,  6,  6, 6, 6,  6,  6, 6, 6}, {6,  6, 6, 6,  6,  6, 6, 6,  6,  6, 6, 6,  6,  6, 6, 6}, {6,  6, 6, 6,  6,  6, 6, 6,  6,  6, 6, 6,  6,  6, 6, 6}},
		{{0,  0, 12,0,  0,  0, 12,0,  0,  0, 12,0,  0,  0, 12,0}, {0,  0, 12,0,  0,  0, 12,0,  12, 0, 12,0,  12, 0, 12,0}, {12, 0, 12,0,  0,  0, 12,0,  12, 0, 12,0,  0,  0, 12,0}, {6,  0, 6, 6,  0,  0, 12,0,  6,  0, 6, 6,  0,  0, 12,0}, {6,  6, 6, 6,  0,  0, 6, 6,  6,  6, 6, 6,  0,  0, 6, 6}, {6,  6, 6, 6,  6,  6, 6, 6,  0,  0, 6, 6,  0,  0, 6, 6}, {6,  6, 6, 6,  6,  6, 6, 6,  6,  6, 6, 6,  12, 0, 12,0}, {6,  6, 6, 6,  6,  6, 6, 6,  6,  6, 6, 6,  6,  6, 6, 6}, {6,  6, 6, 6,  6,  6, 6, 6,  6,  6, 6, 6,  6,  6, 6, 6}, {6,  6, 6, 6,  6,  6, 6, 6,  6,  6, 6, 6,  6,  6, 6, 6}, {6,  6, 6, 6,  6,  6, 6, 6,  6,  6, 6, 6,  6,  6, 6, 6}, {6,  6, 6, 6,  6,  6, 6, 6,  6,  6, 6, 6,  6,  6, 6, 6}, {6,  6, 6, 6,  6,  6, 6, 6,  6,  6, 6, 6,  6,  6, 6, 6}, {6,  6, 6, 6,  6,  6, 6, 6,  6,  6, 6, 6,  6,  6, 6, 6}, {6,  6, 6, 6,  6,  6, 6, 6,  6,  6, 6, 6,  6,  6, 6, 6}, {6,  6, 6, 6,  6,  6, 6, 6,  6,  6, 6, 6,  6,  6, 6, 6}, {6,  6, 6, 6,  6,  6, 6, 6,  6,  6, 6, 6,  6,  6, 6, 6}, {6,  6, 6, 6,  6,  6, 6, 6,  6,  6, 6, 6,  6,  6, 6, 6}, {6,  6, 6, 6,  6,  6, 6, 6,  6,  6, 6, 6,  6,  6, 6, 6}, {6,  6, 6, 6,  6,  6, 6, 6,  6,  6, 6, 6,  6,  6, 6, 6}}
	};

	for (int bar = 0; bar < bars; ++bar) {
		auto& bs = barSettingsUI[slot][bar];
		int barStart = bar * ticksPerBar;
		if (barStart >= 1024) break;

		int cIdx = juce::jlimit<int>(0, 19, static_cast<int>(bs.cmplx) / 5);
		int eIdx = juce::jlimit<int>(0, 9, static_cast<int>(bs.entrp) / 10);
		int nextAvailableTick = -1;

		for (int step = 0; step < stepsPerBar; ++step) {
			int len = synthwaveRhythms[subStyle][cIdx][step % 16];
			if (len == 0) continue;

			int tick = barStart + step * 6;
			if (tick >= 1024 || tick < nextAvailableTick || isTickLocked(tick, slot)) continue;

			int row = 0, oct = 0, vel = 100 + random.nextInt(12);
			if (step % 4 == 0) row = 0;
			else { if (eIdx <= 3) row = 0; else if (eIdx <= 6) row = (random.nextInt(100) < 15) ? 4 : 0; else { const int rows[] = { 0, 4, 0, 7 }; row = rows[random.nextInt(4)]; } }
			if (subStyle == 1 && step % 2 != 0) { oct = 1; vel += 10; }
			else if (step % 16 >= 14 && eIdx >= 2) { if (random.nextInt(100) < (20 + eIdx * 8)) { oct = 1; vel += 15; } }

			auto& d = patternUI[slot][row][tick % 1024];
			d.velocity = juce::jlimit<int>(1, 127, vel); d.length = std::max(2, len - 1); d.offset = 0;
			if (len > 6 && random.nextInt(100) < static_cast<int>(bs.autoGlide)) d.glide = true; else d.glide = false;
			d.staccato = (d.length <= 6); d.octave = juce::jlimit<int>(0, 1, oct);
			nextAvailableTick = tick + d.length;
		}
	}
}

// ==============================================================================
// 16. Funk
// ==============================================================================
void BassLineMatrixAudioProcessor::generateFunk(int slot, int bars, int numBeats, int ticksPerBar, const GenreDefinition& dna) {
	const int riffAnchor[] = { 0, -1, 7, -1 };
	const int riffSteady[] = { 0, 0, -1, 0 };
	const int riffWalk[] = { 0, 2, 3, 4 };
	const int riffDown[] = { 7, 5, 4, 2 };
	const int riffPenta[] = { 0, 3, 4, 6 };
	const int riffSync[] = { -1, 0, 7, 0 };
	const int riffPop[] = { 0, 7, 0, 7 };
	const int riffGallop[] = { 0, 0, 0, -1 };

	int subStyle = random.nextInt(4);
	int ticksPerBeat = ticksPerBar / numBeats;
	int stepsPerBeat = ticksPerBeat / 6;

	for (int bar = 0; bar < bars; ++bar) {
		auto& bs = barSettingsUI[slot][bar];
		int barStart = bar * ticksPerBar;
		if (barStart >= 1024) break;

		for (int beat = 0; beat < numBeats; ++beat) {
			if (beat == 1 && random.nextInt(100) < 20) continue;
			if (beat == 3 && random.nextInt(100) < 40) continue;

			const int* currentRiff; int rPick = random.nextInt(100); int entropy = static_cast<int>(bs.entrp);
			if (beat == 0) currentRiff = (rPick < 70) ? riffSteady : riffAnchor;
			else if (beat == 3) currentRiff = (rPick < 50) ? riffDown : riffPenta;
			else { if (rPick < (100 - entropy)) currentRiff = (random.nextBool()) ? riffSteady : riffWalk; else { int variety = random.nextInt(4); if (variety == 0) currentRiff = riffPenta; else if (variety == 1) currentRiff = riffSync; else if (variety == 2) currentRiff = riffPop; else currentRiff = riffGallop; } }

			for (int step = 0; step < stepsPerBeat; ++step) {
				int tick = barStart + beat * ticksPerBeat + step * 6;
				if (tick >= 1024 || isTickLocked(tick, slot)) continue;
				int row = currentRiff[step % 4];
				if (row == -1) continue;
				if (step % 2 != 0 && random.nextInt(100) > static_cast<int>(bs.cmplx)) continue;

				bool isScaleOut = false;
				if (beat == 3 && step >= 2 && random.nextInt(100) < (static_cast<int>(bs.entrp) / 2)) { row = 8 + random.nextInt(4); isScaleOut = true; }
				int vel = (step == 0) ? genSettings.vel.max : (genSettings.vel.min + random.nextInt(30));
				if (subStyle == 1) vel += 10;
				int len = (subStyle == 3) ? 10 : 5; if (isScaleOut) len = 2;
				len = std::max(2, len); bool forceStaccato = (len > 2); if (subStyle == 3 && !isScaleOut) forceStaccato = false;
				int oct = 0; if (subStyle == 2 && row == 7) oct = 1; else if (random.nextInt(100) < 15) oct = 1;

				auto& d = patternUI[slot][row][tick % 1024];
				d.velocity = juce::jlimit<int>(1, 127, vel); d.length = len; d.offset = 0;
				d.staccato = forceStaccato; d.octave = std::clamp(oct, 0, 1);
				if (len > 6 && random.nextInt(100) < static_cast<int>(bs.autoGlide)) d.glide = true; else d.glide = false;
			}
		}
	}
}

// ==============================================================================
// 17. New Jack
// ==============================================================================
void BassLineMatrixAudioProcessor::generateNewJack(int slot, int bars, int numBeats, int ticksPerBar, const GenreDefinition& dna) {
	const int riffRiley[] = { 0, -1, 7, 0 }; const int riffStreet[] = { 0, 0, -1, 0 }; const int riffDorian[] = { 0, 1, 4, 5 }; const int riffSync[] = { -1, 0, 0, -1 }; const int riffMinimal[] = { 0, -1, -1, -1 }; const int riffEnd[] = { 7, 5, 4, 2 };

	int subStyle = random.nextInt(4);
	int ticksPerBeat = ticksPerBar / numBeats;
	int stepsPerBeat = ticksPerBeat / 6;

	for (int bar = 0; bar < bars; ++bar) {
		auto& bs = barSettingsUI[slot][bar];
		int barStart = bar * ticksPerBar;
		if (barStart >= 1024) break;

		for (int beat = 0; beat < numBeats; ++beat) {
			bool isSnareBeat = (beat == 1 || beat == 3);
			if (isSnareBeat && random.nextInt(100) < 30) continue;

			const int* currentRiff; int rPick = random.nextInt(100); int entropy = static_cast<int>(bs.entrp);
			if (beat == 0) currentRiff = (rPick < 60) ? riffRiley : riffMinimal;
			else if (beat == 3 && entropy > 50) currentRiff = riffEnd;
			else { if (rPick < (100 - entropy)) currentRiff = (random.nextBool()) ? riffStreet : riffMinimal; else currentRiff = (random.nextBool()) ? riffDorian : riffSync; }

			for (int step = 0; step < stepsPerBeat; ++step) {
				int tick = barStart + beat * ticksPerBeat + step * 6;
				if (tick >= 1024 || isTickLocked(tick, slot)) continue;
				int row = currentRiff[step % 4]; if (row == -1) continue;
				if (step % 2 != 0 && random.nextInt(100) > static_cast<int>(bs.cmplx)) continue;
				bool isScaleOut = false;
				if (beat == 3 && step == 3 && random.nextInt(100) < (entropy / 2)) { row = 8 + random.nextInt(2); isScaleOut = true; }
				int vel = (step == 0) ? genSettings.vel.max : (genSettings.vel.min + random.nextInt(25));
				if (isSnareBeat && step == 0) vel = genSettings.vel.max;
				int len = isScaleOut ? 2 : (4 + random.nextInt(3)); len = std::max(2, len);
				bool forceStaccato = (len > 2); int oct = (row == 7 || (subStyle == 0 && step == 2)) ? 1 : 0;

				auto& d = patternUI[slot][row][tick % 1024];
				d.velocity = juce::jlimit<int>(1, 127, vel); d.length = len; d.offset = 0;
				d.staccato = forceStaccato; d.octave = std::clamp(oct, 0, 1);
				if (len > 6 && random.nextInt(100) < static_cast<int>(bs.autoGlide)) d.glide = true; else d.glide = false;
			}
		}
	}
}

// ==============================================================================
// 18. Neo Soul
// ==============================================================================
void BassLineMatrixAudioProcessor::generateNeoSoul(int slot, int bars, int numBeats, int ticksPerBar, const GenreDefinition& dna) {
	const int R = 0, F = 4, _ = -1, C = 100, A = 101, G = 102;
	const int patMin0[16] = { R, _, _, _,  _, _, _, _,  _, _, _, _,  _, _, _, _ };
	const int patDilla0[16] = { R, _, _, _,  _, _, _, _,  _, _, R, _,  _, _, _, _ };
	const int patGospel0[16] = { R, _, _, _,  _, _, _, _,  F, _, _, _,  _, _, _, _ };
	const int patDilla1[16] = { R, _, _, G,  C, _, _, _,  _, _, R, _,  C, _, _, _ };
	const int patGospel1[16] = { R, _, _, _,  C, _, _, G,  F, _, _, _,  C, _, R, _ };
	const int patMidnit1[16] = { R, _, _, _,  _, _, C, _,  F, _, _, _,  C, _, _, _ };
	const int patDilla2[16] = { R, _, G, G,  C, _, _, _,  F, _, R, _,  C, _, A, A };
	const int patGospel2[16] = { R, _, _, _,  C, _, C, _,  F, _, _, _,  C, _, A, A };
	const int patMidnit2[16] = { R, _, C, _,  F, _, G, _,  R, _, C, _,  F, _, A, A };

	int subStyle = random.nextInt(4);
	int stepsPerBar = ticksPerBar / 6;

	for (int bar = 0; bar < bars; ++bar) {
		auto& bs = barSettingsUI[slot][bar];
		int barStart = bar * ticksPerBar;
		if (barStart >= 1024) break;

		const int* currentPat = patMin0;
		if (bs.cmplx < 30 && bs.entrp < 30) { if (subStyle == 0) currentPat = patDilla0; else if (subStyle == 1) currentPat = patGospel0; else currentPat = patMin0; }
		else if (bs.entrp < 50) { if (subStyle == 0) currentPat = patDilla1; else if (subStyle == 1) currentPat = patGospel1; else if (subStyle == 2) currentPat = patMidnit1; else currentPat = patDilla0; }
		else { if (subStyle == 0) currentPat = patDilla2; else if (subStyle == 1) currentPat = patGospel2; else if (subStyle == 2) currentPat = patMidnit2; else currentPat = patDilla1; }

		for (int step = 0; step < stepsPerBar; ++step) {
			int tick = barStart + step * 6;
			if (tick >= 1024 || isTickLocked(tick, slot)) continue;
			int symbol = currentPat[step % 16]; if (symbol == _) continue;

			int row = 0, vel = 85, len = 12; bool isGhost = false, isApproach = false;
			if (symbol == R) { row = 0; vel = (step == 0) ? genSettings.vel.max : 85 + random.nextInt(10); }
			else if (symbol == F) { row = 4; vel = 80 + random.nextInt(10); }
			else if (symbol == C) { row = (random.nextBool()) ? 2 : 6; vel = 75 + random.nextInt(15); len = 8; }
			else if (symbol == A) { if (bs.entrp < 50) continue; row = 8; vel = 60 + random.nextInt(20); len = 2; isApproach = true; }
			else if (symbol == G) { if (bs.cmplx < 40) continue; row = 0; vel = 30 + random.nextInt(15); len = 2; isGhost = true; }

			len = std::max(2, len); bool forceStaccato = (len > 2);
			int oct = (!isGhost && !isApproach && row >= 4 && random.nextInt(100) < 15) ? 1 : 0;

			auto& d = patternUI[slot][row][tick % 1024];
			d.velocity = juce::jlimit<int>(1, 127, vel); d.length = len; d.offset = 0; d.staccato = forceStaccato; d.octave = std::clamp(oct, 0, 1);
			if (len > 6 && random.nextInt(100) < static_cast<int>(bs.autoGlide)) d.glide = true; else d.glide = false;
		}
	}
}

// ==============================================================================
// 19. Boom Bap / Lo-Fi Loop
// ==============================================================================
void BassLineMatrixAudioProcessor::generateBoomBap(int slot, int bars, int numBeats, int ticksPerBar, const GenreDefinition& dna) {
	const int R = 0, F = 4, _ = -1, C = 100, A = 101, G = 102;
	const int patGol0[16] = { R, _, _, _,  _, _, _, _,  R, _, _, _,  _, _, _, _ };
	const int patGolS0[16] = { R, _, _, R,  _, _, _, _,  R, _, _, _,  _, _, _, _ };
	const int patJaz0[16] = { R, _, _, _,  _, _, _, _,  R, _, _, _,  _, _, F, _ };
	const int patJazS0[16] = { R, _, _, _,  _, _, _, _,  R, _, _, R,  _, _, F, _ };
	const int patChi0[16] = { R, _, _, _,  _, _, _, _,  _, _, _, _,  _, _, _, _ };
	const int patChiS0[16] = { R, _, _, R,  _, _, _, _,  _, _, _, _,  _, _, _, _ };
	const int patGol1[16] = { R, _, G, _,  C, _, G, _,  R, _, _, G,  F, _, G, _ };
	const int patJaz1[16] = { R, _, C, _,  F, _, _, _,  R, _, C, _,  F, _, G, _ };
	const int patChi1[16] = { R, _, _, _,  _, _, G, _,  R, _, _, _,  C, _, _, _ };
	const int patGol2[16] = { R, _, G, G,  C, _, F, _,  R, _, G, _,  C, _, A, A };
	const int patJaz2[16] = { R, _, C, _,  F, _, G, G,  R, _, C, _,  F, _, A, A };
	const int patChi2[16] = { R, _, G, _,  _, _, C, _,  R, _, G, _,  F, _, A, _ };

	int subStyle = random.nextInt(3);
	int stepsPerBar = ticksPerBar / 6;

	for (int bar = 0; bar < bars; ++bar) {
		auto& bs = barSettingsUI[slot][bar];
		int barStart = bar * ticksPerBar;
		if (barStart >= 1024) break;

		const int* currentPat = patGol0;
		if (bs.cmplx < 30 && bs.entrp < 30) { bool useSwing = (random.nextInt(100) < 35); if (subStyle == 0) currentPat = useSwing ? patGolS0 : patGol0; else if (subStyle == 1) currentPat = useSwing ? patJazS0 : patJaz0; else currentPat = useSwing ? patChiS0 : patChi0; }
		else if (bs.entrp < 50) { if (subStyle == 0) currentPat = patGol1; else if (subStyle == 1) currentPat = patJaz1; else currentPat = patChi1; }
		else { if (subStyle == 0) currentPat = patGol2; else if (subStyle == 1) currentPat = patJaz2; else currentPat = patChi2; }

		for (int step = 0; step < stepsPerBar; ++step) {
			int beatInBar = step / 4; int stepInBeat = step % 4;
			int t = beatInBar * (ticksPerBar / numBeats);
			if (stepInBeat == 0) t += 0; else if (stepInBeat == 1) t += 7; else if (stepInBeat == 2) t += 12; else if (stepInBeat == 3) t += 19;
			int tick = barStart + t;
			if (tick >= 1024 || isTickLocked(tick, slot)) continue;
			int symbol = currentPat[step % 16]; if (symbol == _) continue;

			int row = 0, vel = 90, len = 6; bool isGhost = false, isApproach = false;
			if (symbol == R) { row = 0; vel = (step == 0) ? genSettings.vel.max : 90 + random.nextInt(10); len = 6 + random.nextInt(4); }
			else if (symbol == F) { row = 4; vel = 85 + random.nextInt(10); len = 6 + random.nextInt(2); }
			else if (symbol == C) { row = (random.nextBool()) ? 2 : 6; vel = 80 + random.nextInt(15); len = 4 + random.nextInt(3); }
			else if (symbol == A) { if (bs.entrp < 50) continue; row = 8; vel = 70 + random.nextInt(20); len = 2; isApproach = true; }
			else if (symbol == G) { if (bs.cmplx < 40) continue; row = 0; vel = 35 + random.nextInt(15); len = 2; isGhost = true; }

			len = std::max(2, len); bool forceStaccato = (len > 2);
			auto& d = patternUI[slot][row][tick % 1024];
			d.velocity = juce::jlimit<int>(1, 127, vel); d.length = len; d.offset = 0; d.staccato = forceStaccato; d.octave = 0;
			if (len > 6 && random.nextInt(100) < static_cast<int>(bs.autoGlide)) d.glide = true; else d.glide = false;
		}
	}
}


// ==============================================================================
// 20. Urban Jazz
// ==============================================================================
void BassLineMatrixAudioProcessor::generateUrbanJazz(int slot, int bars, int numBeats, int ticksPerBar, const GenreDefinition& dna) {
	int subStyle = random.nextInt(3);
	int ticksPerBeat = ticksPerBar / numBeats;

	for (int bar = 0; bar < bars; ++bar) {
		auto& bs = barSettingsUI[slot][bar];
		int barStart = bar * ticksPerBar;
		if (barStart >= 1024) break;
		int consecutive = 0;

		for (int t = 0; t < ticksPerBar; t += 6) {
			int tick = barStart + t;
			if (tick >= 1024 || isTickLocked(tick, slot)) continue;

			int beat = t / ticksPerBeat;
			int subStep = (t % ticksPerBeat) / 6;

			bool hit = false;
			if (bs.cmplx < 30 && bs.entrp < 30) {
				if (subStep == 0 && beat == 0) hit = true;
				else if (subStep == 3 && (beat == 1 || beat == 3)) hit = true;
				else if (subStep == 2 && beat == 2 && random.nextBool()) hit = true;
			}
			else {
				int prob = static_cast<int>(bs.cmplx);
				if (subStep == 3) prob += 45; else if (subStep == 0) prob -= 10; else if (subStep == 1) prob -= 30; else if (subStep == 2) prob += 15;
				if ((beat == 1 || beat == 3) && subStep == 0) prob -= 50;
				if (random.nextInt(100) < prob) hit = true;
			}

			if (hit) consecutive++; else consecutive = 0;
			if (consecutive > 2) { hit = false; consecutive = 0; }

			if (hit) {
				int row = 0, vel = 90, len = 6, oct = 0; bool isGhost = false;
				int rVal = random.nextInt(100);
				if (subStep == 0 || subStep == 3) row = (rVal < 75) ? 0 : 4;
				else { if (rVal < 40) row = 0; else if (rVal < 70) row = 2; else row = 6; }

				if (row == 0 && subStep != 0 && random.nextInt(100) < 15) { oct = 1; vel += 15; len = 3; }
				else if (subStep == 1 && random.nextInt(100) < (static_cast<int>(bs.cmplx) / 2)) { row = 0; isGhost = true; vel = 35 + random.nextInt(20); len = 2; }

				bool isPassingMoment = (beat >= 2 && subStep >= 2);
				if (!isGhost && isPassingMoment && random.nextInt(100) < (static_cast<int>(bs.entrp) / 2)) { row = 8 + random.nextInt(4); vel -= 15; len = 2; }

				if (!isGhost && len > 2) len = (subStep == 3) ? 10 : (4 + random.nextInt(4));
				len = std::max(2, len); bool forceStaccato = (len > 2);

				auto& d = patternUI[slot][row][tick % 1024];
				d.velocity = juce::jlimit<int>(1, 127, vel); d.length = len; d.offset = 0; d.staccato = forceStaccato; d.octave = std::clamp(oct, 0, 1);
				if (len > 6 && random.nextInt(100) < static_cast<int>(bs.autoGlide)) d.glide = true; else d.glide = false;
			}
		}
	}
}

// ==============================================================================
// 21. Melodic Techno
// ==============================================================================
void BassLineMatrixAudioProcessor::generateMelodicTechno(int slot, int bars, int numBeats, int ticksPerBar, const GenreDefinition& dna) {
	int subStyle = random.nextInt(4);
	int stepsPerBar = ticksPerBar / 6;

	static const int melodicRhythms[4][20][16] = {
		{{6,  0, 0, 0,  6,  0, 0, 0,  6,  0, 0, 0,  6,  0, 0, 0}, {6,  0, 6, 0,  6,  0, 6, 0,  6,  0, 6, 0,  6,  0, 6, 0}, {6,  0, 6, 0,  6,  0, 6, 0,  6,  0, 6, 0,  6,  6, 6, 0}, {6,  0, 6, 6,  6,  0, 6, 6,  6,  0, 6, 6,  6,  0, 6, 6}, {6,  6, 6, 0,  6,  6, 6, 0,  6,  6, 6, 0,  6,  6, 6, 0}, {6,  6, 6, 6,  6,  6, 6, 0,  6,  6, 6, 6,  6,  6, 6, 0}, {6,  6, 6, 6,  6,  6, 6, 6,  6,  6, 6, 6,  6,  6, 6, 0}, {6,  6, 6, 6,  6,  6, 6, 6,  6,  6, 6, 6,  6,  6, 6, 6}, {6,  6, 6, 6,  6,  6, 6, 6,  6,  6, 6, 6,  6,  6, 6, 6}, {6,  6, 6, 6,  6,  6, 6, 6,  6,  6, 6, 6,  6,  6, 6, 6}, {6,  6, 4, 2,  6,  6, 6, 6,  6,  6, 4, 2,  6,  6, 6, 6}, {6,  6, 4, 2,  6,  6, 4, 2,  6,  6, 4, 2,  6,  6, 4, 2}, {4,  2, 6, 6,  4,  2, 6, 6,  4,  2, 6, 6,  4,  2, 6, 6}, {4,  2, 4, 2,  6,  6, 6, 6,  4,  2, 4, 2,  6,  6, 6, 6}, {4,  2, 4, 2,  4,  2, 4, 2,  4,  2, 4, 2,  4,  2, 4, 2}, {4,  2, 4, 2,  4,  2, 4, 2,  4,  2, 4, 2,  4,  2, 4, 2}, {4,  2, 4, 2,  4,  2, 4, 2,  4,  2, 4, 2,  2,  2, 2, 2}, {4,  2, 4, 2,  4,  2, 4, 2,  2,  2, 2, 2,  2,  2, 2, 2}, {4,  2, 2, 2,  4,  2, 2, 2,  4,  2, 2, 2,  4,  2, 2, 2}, {2,  2, 2, 2,  2,  2, 2, 2,  2,  2, 2, 2,  2,  2, 2, 2}},
		{{12, 0, 0, 0,  12, 0, 0, 0,  12, 0, 0, 0,  12, 0, 0, 0}, {12, 0, 0, 0,  12, 0, 0, 0,  12, 0, 0, 0,  6,  0, 6, 0}, {12, 0, 0, 0,  6,  0, 6, 0,  12, 0, 0, 0,  6,  0, 6, 0}, {6,  0, 6, 0,  6,  0, 6, 0,  6,  0, 6, 0,  6,  0, 6, 0}, {6,  0, 6, 0,  6,  0, 6, 0,  6,  0, 6, 0,  6,  6, 0, 0}, {6,  0, 6, 0,  6,  0, 6, 0,  6,  0, 6, 0,  6,  6, 6, 0}, {6,  0, 6, 0,  6,  6, 0, 0,  6,  0, 6, 0,  6,  6, 6, 0}, {6,  0, 6, 0,  6,  6, 6, 0,  6,  0, 6, 0,  6,  6, 6, 0}, {6,  6, 0, 0,  6,  6, 0, 0,  6,  6, 0, 0,  6,  6, 0, 0}, {6,  6, 6, 0,  6,  6, 6, 0,  6,  6, 6, 0,  6,  6, 6, 0}, {6,  6, 6, 0,  6,  6, 6, 0,  6,  6, 6, 0,  6,  6, 6, 6}, {6,  6, 6, 0,  6,  6, 6, 6,  6,  6, 6, 0,  6,  6, 6, 6}, {6,  6, 6, 6,  6,  6, 6, 6,  6,  6, 6, 6,  6,  6, 6, 6}, {6,  6, 6, 6,  6,  6, 6, 6,  6,  6, 6, 6,  6,  6, 6, 6}, {6,  6, 6, 6,  6,  6, 6, 6,  4,  2, 4, 2,  4,  2, 4, 2}, {6,  6, 4, 2,  6,  6, 4, 2,  6,  6, 4, 2,  6,  6, 4, 2}, {4,  2, 4, 2,  6,  6, 6, 6,  4,  2, 4, 2,  6,  6, 6, 6}, {4,  2, 4, 2,  4,  2, 4, 2,  4,  2, 4, 2,  4,  2, 4, 2}, {4,  2, 4, 2,  4,  2, 4, 2,  4,  2, 4, 2,  4,  2, 4, 2}, {2,  2, 2, 2,  2,  2, 2, 2,  2,  2, 2, 2,  2,  2, 2, 2}},
		{{48, 0, 0, 0,  0, 0, 0, 0,  0,  0, 0, 0,  0, 0, 0, 0}, {48, 0, 0, 0,  0, 0, 0, 0,  24, 0, 0, 0,  0, 0, 0, 0}, {48, 0, 0, 0,  0, 0, 0, 0,  12, 0, 0, 0,  12,0, 0, 0}, {48, 0, 0, 0,  0, 0, 0, 0,  12, 0, 0, 0,  6, 0, 6, 0}, {36, 0, 0, 0,  0, 0, 12,0,  24, 0, 0, 0,  0, 0, 0, 0}, {36, 0, 0, 0,  0, 0, 12,0,  12, 0, 0, 0,  12,0, 0, 0}, {36, 0, 0, 0,  0, 0, 12,0,  12, 0, 0, 0,  6, 0, 6, 0}, {24, 0, 0, 0,  24,0, 0, 0,  24, 0, 0, 0,  0, 0, 0, 0}, {24, 0, 0, 0,  24,0, 0, 0,  12, 0, 0, 0,  12,0, 0, 0}, {24, 0, 0, 0,  24,0, 0, 0,  12, 0, 0, 0,  6, 0, 6, 0}, {24, 0, 0, 0,  12,0, 12,0,  12, 0, 0, 0,  6, 6, 6, 0}, {24, 0, 0, 0,  12,0, 12,0,  12, 0, 6, 0,  6, 6, 6, 0}, {24, 0, 0, 0,  12,0, 12,0,  6,  6, 6, 0,  6, 6, 6, 0}, {12, 0, 0, 0,  12,0, 12,0,  6,  6, 6, 0,  6, 6, 6, 0}, {12, 0, 0, 0,  12,0, 12,0,  6,  6, 6, 0,  4, 2, 4, 2}, {12, 0, 0, 0,  12,0, 6, 6,  6,  6, 6, 0,  4, 2, 4, 2}, {12, 0, 0, 0,  6, 6, 6, 6,  6,  6, 6, 0,  4, 2, 4, 2}, {12, 0, 0, 0,  6, 6, 6, 6,  4,  2, 4, 2,  4, 2, 4, 2}, {12, 0, 0, 0,  6, 6, 6, 6,  4,  2, 4, 2,  4, 2, 4, 2}, {12, 0, 0, 0,  4, 2, 4, 2,  4,  2, 4, 2,  4, 2, 4, 2}},
		{{12, 0, 0, 0,  0, 0, 12,0,  12, 0, 0, 0,  0, 0, 12,0}, {12, 0, 0, 0,  0, 0, 12,0,  6,  0, 6, 0,  0, 0, 12,0}, {6,  0, 6, 0,  0, 0, 12,0,  6,  0, 6, 0,  0, 0, 12,0}, {6,  0, 6, 0,  0, 0, 6, 6,  6,  0, 6, 0,  0, 0, 6, 6}, {0,  0, 6, 6,  0, 0, 12,0,  0,  0, 6, 6,  0, 0, 12,0}, {0,  0, 6, 6,  0, 0, 6, 6,  0,  0, 6, 6,  0, 0, 6, 6}, {0,  6, 6, 0,  0, 6, 6, 0,  0,  6, 6, 0,  0, 6, 6, 0}, {0,  6, 6, 0,  0, 0, 12,0,  0,  6, 6, 0,  0, 0, 12,0}, {6,  0, 0, 6,  6, 0, 6, 0,  6,  0, 0, 6,  6, 0, 6, 0}, {6,  0, 0, 6,  0, 0, 6, 6,  6,  0, 0, 6,  0, 0, 6, 6}, {4,  2, 6, 0,  0, 0, 12,0,  4,  2, 6, 0,  0, 0, 12,0}, {0,  0, 4, 2,  6, 0, 6, 0,  0,  0, 4, 2,  6, 0, 6, 0}, {0,  0, 4, 2,  0, 0, 4, 2,  0,  0, 4, 2,  0, 0, 4, 2}, {4,  2, 0, 6,  4, 2, 0, 6,  4,  2, 0, 6,  4, 2, 0, 6}, {2,  4, 6, 0,  2, 4, 6, 0,  2,  4, 6, 0,  2, 4, 6, 0}, {2,  4, 0, 6,  2, 4, 0, 6,  2,  4, 0, 6,  2, 4, 0, 6}, {4,  2, 4, 2,  0, 0, 12,0,  4,  2, 4, 2,  0, 0, 12,0}, {0,  0, 4, 2,  4, 2, 4, 2,  0,  0, 4, 2,  4, 2, 4, 2}, {4,  2, 4, 2,  0, 6, 6, 0,  4,  2, 4, 2,  0, 6, 6, 0}, {2,  4, 2, 4,  2, 4, 2, 4,  2,  4, 2, 4,  2, 4, 2, 4}}
	};

	for (int bar = 0; bar < bars; ++bar) {
		auto& bs = barSettingsUI[slot][bar];
		int barStart = bar * ticksPerBar;
		if (barStart >= 1024) break;

		int cIdx = juce::jlimit<int>(0, 19, static_cast<int>(bs.cmplx) / 5);
		int eIdx = juce::jlimit<int>(0, 9, static_cast<int>(bs.entrp) / 10);
		bool isFourthBar = ((bar + 1) % 4 == 0);

		for (int step = 0; step < stepsPerBar; ++step) {
			int len = melodicRhythms[subStyle][cIdx][step % 16];
			if (len == 0) continue;

			int tick = barStart + step * 6;
			if (tick >= 1024 || isTickLocked(tick, slot)) continue;

			int row = 0, oct = 0;
			int vel = (step % 4 == 0) ? 75 : ((step % 2 != 0) ? 120 : 115);

			if (step == 0) row = 0;
			else { if (eIdx <= 2) row = (random.nextBool()) ? 0 : 4; else if (eIdx <= 5) { const int rows[] = { 0, 2, 4 }; row = rows[random.nextInt(3)]; } else if (eIdx <= 8) { const int rows[] = { 0, 2, 4, 6 }; row = rows[random.nextInt(4)]; } else row = random.nextInt(7); }
			if (subStyle == 1 || eIdx >= 6) { int octProb = (subStyle == 1) ? (eIdx * 5) : ((eIdx - 5) * 8); if (step % 2 != 0 && random.nextInt(100) < octProb) { oct = 1; vel += 10; } }
			if (eIdx == 9 && step % 16 >= 14 && isFourthBar && random.nextInt(100) < 15) { row = 8 + random.nextInt(4); vel -= 5; len = 2; oct = 0; }

			auto& d = patternUI[slot][row][tick % 1024];
			d.velocity = juce::jlimit<int>(1, 127, vel); d.length = std::max(2, len); d.offset = 0; d.staccato = (len <= 4); d.octave = std::clamp(oct, 0, 1);
			if (len > 6 && random.nextInt(100) < static_cast<int>(bs.autoGlide)) d.glide = true; else d.glide = false;
		}
	}
}

// ==============================================================================
// 22. Walking Bass
// ==============================================================================
void BassLineMatrixAudioProcessor::generateWalkingBass(int slot, int bars, int numBeats, int ticksPerBar, const GenreDefinition& dna) {
	for (int bar = 0; bar < bars; ++bar) {
		auto& bs = barSettingsUI[slot][bar];
		int barStart = bar * ticksPerBar;
		if (barStart >= 1024) break;

		int cmplxVal = static_cast<int>(bs.cmplx);
		int entrpVal = static_cast<int>(bs.entrp);

		static const int anchorContours[10][4] = {
			{0, 2, 4, 2}, {0, 1, 2, 4}, {0, 4, 6, 5}, {0, 4, 2, 1}, {0, 6, 5, 4}, {4, 3, 2, 1}, {0, 2, 3, 4}, {0, 4, 0, 4}, {0, 6, 8, 8}, {0, 0, 6, 8}
		};
		int contourIdx = juce::jlimit<int>(0, 9, entrpVal / 10);
		int stepsPerBar = ticksPerBar / 6;

		for (int step = 0; step < stepsPerBar; ++step) {
			int tick = barStart + (step * 6);
			if (tick >= 1024 || isTickLocked(tick, slot)) continue;

			int beatIdx = step / 4;
			bool isBeatHead = (step % 4 == 0);
			bool isSwingStep = (step % 4 == 3);

			bool shouldHit = false; int row = 0, len = 6, offset = 0, vel = 90, oct = 0;

			if (isBeatHead) {
				shouldHit = true; row = anchorContours[contourIdx][beatIdx % 4]; len = 22;
				vel = (beatIdx % 2 != 0) ? (115 + random.nextInt(10)) : (100 + random.nextInt(10));
				if (beatIdx == 0 && contourIdx >= 8) oct = 1;
			}
			else if (isSwingStep) {
				int skipProb = (beatIdx % 2 != 0) ? cmplxVal : (cmplxVal / 2);
				if (random.nextInt(100) < skipProb) {
					shouldHit = true; int nextBeatAnchor = anchorContours[contourIdx][(beatIdx + 1) % 4];
					row = (entrpVal > 60 && random.nextInt(100) < 70) ? (8 + random.nextInt(4)) : nextBeatAnchor;
					len = 6; offset = -2; vel = 65 + random.nextInt(20); oct = 0;
				}
			}

			if (shouldHit) {
				auto& d = patternUI[slot][row][tick % 1024];
				d.velocity = juce::jlimit<int>(1, 127, vel); d.length = std::max(2, len); d.offset = offset; d.staccato = (len <= 6); d.octave = juce::jlimit<int>(0, 1, oct);
				if (len > 12 && random.nextInt(100) < static_cast<int>(bs.autoGlide)) d.glide = true; else d.glide = false;
			}
		}
	}
}



// ==============================================================================
// 23. Electronic Generic (Universal Electronic Lab)
// ==============================================================================
void BassLineMatrixAudioProcessor::generateElectronicGeneric(int slot, int bars, int numBeats, int ticksPerBar, const GenreDefinition& dna) {
	int subStyle = random.nextInt(4);
	int stepsPerBar = ticksPerBar / 6;

	static const int electronicAnchors[4][10][16] = {
		{ {1,0,0,0, 1,0,0,0, 1,0,0,0, 1,0,0,0}, {1,0,1,0, 1,0,1,0, 1,0,1,0, 1,0,1,0}, {1,1,1,1, 1,1,1,1, 1,1,1,1, 1,1,1,1}, {1,0,0,1, 1,0,0,1, 1,0,0,1, 1,0,0,1}, {1,1,0,0, 1,1,0,0, 1,1,0,0, 1,1,0,0}, {1,0,1,1, 1,0,1,1, 1,0,1,1, 1,0,1,1}, {0,1,0,1, 0,1,0,1, 0,1,0,1, 0,1,0,1}, {1,0,0,0, 0,0,1,0, 1,0,0,0, 0,0,1,0}, {1,1,1,0, 1,1,1,0, 1,1,1,0, 1,1,1,0}, {1,0,0,0, 1,0,0,0, 1,1,1,1, 1,1,1,1} },
		{ {1,0,1,0, 0,1,0,0, 1,0,1,0, 0,1,0,1}, {0,0,1,0, 0,0,1,0, 0,0,1,0, 0,0,1,0}, {1,0,0,1, 0,1,0,0, 1,0,0,1, 0,1,1,0}, {1,0,1,1, 0,1,0,1, 1,0,1,1, 0,1,0,1}, {0,1,1,0, 0,1,1,0, 0,1,1,0, 0,1,1,0}, {1,1,0,1, 1,1,0,1, 0,0,1,1, 0,0,1,1}, {1,0,0,1, 0,0,1,0, 1,0,0,1, 0,0,1,0}, {0,1,0,1, 1,0,1,0, 0,1,0,1, 1,0,1,0}, {1,0,1,0, 1,1,0,1, 1,0,1,0, 1,1,0,1}, {0,0,0,1, 0,0,0,1, 0,0,0,1, 1,1,1,1} },
		{ {1,0,0,1, 0,0,0,1, 1,0,0,0, 1,0,1,0}, {1,1,0,0, 0,1,0,1, 0,0,1,0, 1,0,0,1}, {0,1,0,1, 1,1,0,0, 0,1,0,1, 1,0,1,0}, {1,0,1,1, 0,0,1,0, 1,1,0,1, 0,0,1,0}, {0,0,1,1, 1,0,0,1, 0,0,1,1, 1,0,0,1}, {1,0,1,0, 1,0,1,1, 0,1,0,1, 0,1,1,0}, {1,1,1,0, 0,0,1,1, 1,0,1,0, 0,1,0,1}, {0,1,0,0, 1,1,1,0, 0,1,0,0, 1,1,1,0}, {1,0,0,0, 1,0,1,1, 0,0,1,0, 0,0,1,1}, {1,1,1,1, 0,0,0,0, 1,1,1,1, 0,0,1,1} },
		{ {1,0,0,0, 0,0,0,0, 1,0,0,0, 0,0,0,0}, {0,0,0,0, 1,0,0,0, 0,0,0,0, 1,0,0,0}, {1,0,0,0, 0,0,0,0, 0,0,1,0, 0,0,0,0}, {0,0,1,0, 0,0,0,0, 0,0,1,0, 0,0,0,0}, {1,0,0,0, 1,0,0,0, 0,0,0,0, 0,0,0,0}, {0,0,1,1, 0,0,0,0, 0,0,1,1, 0,0,0,0}, {1,0,1,0, 0,0,0,0, 1,0,1,0, 0,0,0,0}, {0,1,0,0, 0,1,0,0, 0,1,0,0, 0,1,0,0}, {0,0,0,0, 0,0,0,0, 0,0,0,0, 1,1,1,1}, {1,0,0,1, 0,0,0,0, 1,0,0,1, 0,0,0,0} }
	};

	for (int bar = 0; bar < bars; ++bar) {
		auto& bs = barSettingsUI[slot][bar];
		int barStart = static_cast<int>(bar) * ticksPerBar;
		if (barStart >= 1024) break;

		int cmplxVal = static_cast<int>(bs.cmplx);
		int entrpVal = static_cast<int>(bs.entrp);

		// ★ 型推論エラー回避のために <int> を明示
		int patternIdx = juce::jlimit<int>(0, 9, (cmplxVal / 10));
		int nextAvailableTick = -1;

		for (int step = 0; step < stepsPerBar; ++step) {
			int tick = barStart + (step * 6);
			if (isTickLocked(tick, slot)) continue;

			if (electronicAnchors[subStyle][patternIdx][step % 16] == 0) continue;

			int silenceProb = (100 - cmplxVal) / 3 + (entrpVal / 4);
			if (random.nextInt(100) < silenceProb) continue;

			int baseLen = 3 + (cmplxVal * 21 / 100);
			// ★ 型推論エラー回避のために <int> を明示
			int len = juce::jlimit<int>(3, 24, baseLen + random.nextInt(4) - 2);

			int row = 0;
			int oct = 0;
			if (entrpVal <= 10) {
				row = 0;
			}
			else if (entrpVal <= 30) {
				const int chordTones[] = { 0, 2, 4 };
				row = chordTones[random.nextInt(3)];
			}
			else if (entrpVal <= 60) {
				row = random.nextInt(7);
			}
			else {
				row = random.nextInt(12);
			}

			if (entrpVal > 40 && random.nextInt(100) < (entrpVal / 2)) {
				oct = 1;
			}

			if (tick >= nextAvailableTick) {
				auto& d = patternUI[slot][row][tick % 1024];
				d.velocity = 90 + (cmplxVal / 4) + random.nextInt(15);
				d.length = len;
				d.offset = 0;
				d.octave = oct;
				d.staccato = (len <= 6);
				d.glide = (random.nextInt(100) < static_cast<int>(bs.autoGlide));

				nextAvailableTick = tick + d.length;
			}
		}
	}
	patternUpdated.store(true);
	uiNeedsUpdate.store(true);
}

// ==============================================================================
// ★ 独立アルゴリズム：Generic (汎用)
// ==============================================================================
void BassLineMatrixAudioProcessor::generateGeneric(int slot, int bars, int numBeats, int ticksPerBar, const GenreDefinition& dna) {
	for (int bar = 0; bar < bars; ++bar) {
		auto& bs = barSettingsUI[slot][bar];

		// ★ 修正：24固定ではなく、動的計算済みの ticksPerBar を使用
		int barStart = bar * ticksPerBar;
		if (barStart >= 1024) break; // 境界ガード

		// ★ 修正：小節の長さも ticksPerBar に依存させる
		for (int t = 0; t < ticksPerBar; t += bs.div) {
			int tick = barStart + t;
			if (tick >= 1024 || isTickLocked(tick, slot)) continue; // 境界ガード

			// 型安全のために static_cast を追加
			if (random.nextInt(100) < (t == 0 ? 100 : static_cast<int>(bs.cmplx))) {
				int row = (t == 0 && bs.anchor) ? 0 : (random.nextInt(100) < dna.pRoot ? 0 : random.nextInt(8));

				auto& d = patternUI[slot][row][tick % 1024];
				d.velocity = genSettings.vel.min + random.nextInt(genSettings.vel.max - genSettings.vel.min + 1);
				d.length = genSettings.len.min + random.nextInt(genSettings.len.max - genSettings.len.min + 1);
				d.staccato = (random.nextInt(100) < dna.staccatoProb);
				d.glide = (random.nextInt(100) < static_cast<int>(bs.autoGlide));
				d.offset = 0;
				d.octave = 0;
			}
		}
	}
}

void BassLineMatrixAudioProcessor::generateBassline() {
	clearPattern();
	int slot = currentSlot.load();
	int numBeats = timeSigNumerator.load();   // 分子 (例: 5)
	int den = timeSigDenominator.load();      // 分母 (例: 8)
	int bars = globalBarCount.load();         // 小節数
	int genreIdx = currentGenre.load() - 1;

	if (genreIdx < 0 || genreIdx >= (int)genreRegistry.size()) genreIdx = 0;
	const auto& dna = genreRegistry[genreIdx];

	// --- ★追加：拍子に基づくTick計算 ---
	// 1拍あたりのTick数 (4分音符なら24, 8分音符なら12, 2分音符なら48)
	// 計算式: (24 * 4) / 分母
	int ticksPerBeat = (den > 0) ? (96 / den) : 24;
	int ticksPerBar = numBeats * ticksPerBeat;
	// ---------------------------------

	if (!isStaccatoLocked.load()) {
		staccatoRatio.store(dna.defaultStaccato);
	}

	uiNeedsUpdate.store(true);

	int chordPatternIdx = random.nextInt(16);

	for (int bar = 0; bar < bars; ++bar) {
		// ★ガード：配列の最大（1024 Ticks）を超える小節は処理しない
		if (bar * ticksPerBar >= 1024) break;

		auto& bs = barSettingsUI[slot][bar];

		if (!bs.lockCmplx) bs.cmplx = random.nextInt(genSettings.cmplx.max - genSettings.cmplx.min + 1) + genSettings.cmplx.min;
		if (!bs.lockEntrp) bs.entrp = random.nextInt(genSettings.entrp.max - genSettings.entrp.min + 1) + genSettings.entrp.min;
		if (!bs.lockGlide) bs.autoGlide = random.nextInt(genSettings.glide.max - genSettings.glide.min + 1) + genSettings.glide.min;

		if (bs.useCodeMode && !bs.lockChords) {
			for (int i = 0; i < 16; ++i) {
				if (i < numBeats) {
					// ==========================================================
					// ★ 修正前：絶対的な拍数で16ループ（変拍子で小節の途中でズレる原因）
					// int beatGlobal = (bar * numBeats + i) % 16;
					// bs.chords[i] = dna.chordPatterns[chordPatternIdx][beatGlobal];
					// ==========================================================

					// ==========================================================
					// ★ 修正後：4小節サイクルへの動的マッピング（伸縮）
					// ==========================================================
					// 4小節ループの中での「現在の総拍数」を計算
					int beatIn4BarCycle = (bar % 4) * numBeats + i;
					// その拍子における「4小節の総拍数」
					int totalBeatsIn4Bars = 4 * numBeats;

					// 16個のコードを、現在の総拍数に合わせて均等に引き伸ばす（または圧縮する）
					int mappedChordIdx = (beatIn4BarCycle * 16) / totalBeatsIn4Bars;

					bs.chords[i] = dna.chordPatterns[chordPatternIdx][mappedChordIdx];
				}
			}
		}
	}


	// generateBassline() 内の呼び出し部分を以下のように修正します
		// ★ 4つ目の引数として必ず ticksPerBar を渡す！
// generateBassline() 内のジャンル呼び出し部分
	if (dna.id == 1) generateTechno(slot, bars, numBeats, ticksPerBar, dna);
	else if (dna.id == 2) generateHouse(slot, bars, numBeats, ticksPerBar, dna);
	else if (dna.id == 3) generateUKGarage(slot, bars, numBeats, ticksPerBar, dna);
	else if (dna.id == 4) generateDrumAndBass(slot, bars, numBeats, ticksPerBar, dna);
	else if (dna.id == 5) generateTrap(slot, bars, numBeats, ticksPerBar, dna);
	else if (dna.id == 6) generateFootwork(slot, bars, numBeats, ticksPerBar, dna);
	else if (dna.id == 7) generateIDM(slot, bars, numBeats, ticksPerBar, dna);
	else if (dna.id == 8) generateDubstep(slot, bars, numBeats, ticksPerBar, dna);
	else if (dna.id == 9) generateAfrobeat(slot, bars, numBeats, ticksPerBar, dna);
	else if (dna.id == 10) generateGqom(slot, bars, numBeats, ticksPerBar, dna);
	else if (dna.id == 11) generateAmapiano(slot, bars, numBeats, ticksPerBar, dna);
	else if (dna.id == 12) generateIndian(slot, bars, numBeats, ticksPerBar, dna);
	else if (dna.id == 13) generateLatin(slot, bars, numBeats, ticksPerBar, dna);
	else if (dna.id == 14) generateTrance(slot, bars, numBeats, ticksPerBar, dna);
	else if (dna.id == 15) generateSynthwave(slot, bars, numBeats, ticksPerBar, dna);
	else if (dna.id == 16) generateFunk(slot, bars, numBeats, ticksPerBar, dna);
	else if (dna.id == 17) generateNewJack(slot, bars, numBeats, ticksPerBar, dna);
	else if (dna.id == 18) generateNeoSoul(slot, bars, numBeats, ticksPerBar, dna);
	else if (dna.id == 19) generateBoomBap(slot, bars, numBeats, ticksPerBar, dna);
	else if (dna.id == 20) generateUrbanJazz(slot, bars, numBeats, ticksPerBar, dna);
	else if (dna.id == 21) generateMelodicTechno(slot, bars, numBeats, ticksPerBar, dna);
	else if (dna.id == 22) generateWalkingBass(slot, bars, numBeats, ticksPerBar, dna);
	else if (dna.id == 23) generateElectronicGeneric(slot, bars, numBeats, ticksPerBar, dna);
	else generateGeneric(slot, bars, numBeats, ticksPerBar, dna);

	// ==============================================================================
		// 1. RootAnchor 強制適用（変拍子対応版）
		// ==============================================================================
		// 現在の拍子から1小節の正確な長さを算出
	int currentDen = timeSigDenominator.load();
	int currentNum = timeSigNumerator.load();
	int tPerBeat = (currentDen > 0) ? (96 / currentDen) : 24;
	int tPerBar = currentNum * tPerBeat;

	for (int bar = 0; bar < bars; ++bar) {
		int t0 = bar * tPerBar; // 24固定ではなく拍子に基づいた小節頭
		if (t0 >= 1024) break;  // 配列境界ガード

		auto& bs = barSettingsUI[slot][bar];
		if (bs.anchor) {
			bool hasNoteAtStart = false;
			int existingRow = -1;

			// 小節の頭（t0）に既に音があるかチェック
			for (int r = 0; r < 12; ++r) {
				if (patternUI[slot][r][t0].velocity > 0) {
					hasNoteAtStart = true;
					existingRow = r;
					break;
				}
			}

			if (hasNoteAtStart) {
				// 既に音がある場合、その設定を維持したまま Row 0 (Root) へ移動
				if (existingRow != 0) {
					patternUI[slot][0][t0] = patternUI[slot][existingRow][t0];
					patternUI[slot][existingRow][t0] = StepData();
				}
			}
			else {
				// 音がない場合、新しいRoot音を生成
				auto& d = patternUI[slot][0][t0];
				d.velocity = juce::jlimit(1, 127, (int)genSettings.vel.max);
				d.length = tPerBeat / 2; // 8分音符程度の長さ
				d.offset = 0;
				d.octave = 0;
				d.glide = false;
			}
		}
	}

	// ==============================================================================
	// 2. 仕上げ：ベロシティ制限、Humanize加算、および重複防止（Mono Truncation 2.0）
	// ==============================================================================
	for (int t = 0; t < 1024; ++t) {
		int activeR = -1;
		for (int r = 0; r < 12; ++r) {
			auto& node = patternUI[slot][r][t];
			if (node.velocity > 0) {
				// ベロシティを 1-127 の範囲に収める
				node.velocity = juce::jlimit(1, 127, (int)node.velocity);

				// ★ マスターキルスイッチを削除：node.glide の値はアルゴリズムの指示を維持

				// ユーザーのGlobal Humanize設定を offset に加算
				if (!node.locked && genSettings.hum.max > 0) {
					int jitter = random.nextInt(juce::Range<int>(-genSettings.hum.max, genSettings.hum.max + 1));
					node.offset += jitter;
				}
				activeR = r;
				break;
			}
		}

		if (activeR != -1) {
			// --- Mono Truncation 2.0: オフセットを考慮した重なり防止 ---
			int nextT = 1024;
			int nextOffset = 0;

			// 次に音が鳴るタイミング(nt)とそのオフセットを探索
			for (int nt = t + 1; nt < 1024; ++nt) {
				for (int r = 0; r < 12; ++r) {
					if (patternUI[slot][r][nt].velocity > 0) {
						nextT = nt;
						nextOffset = patternUI[slot][r][nt].offset;
						goto found_next_node;
					}
				}
			}

		found_next_node:
			auto& currentNode = patternUI[slot][activeR][t];

			// 物理的な発音開始位置（Tick + Offset）
			int currentEffectiveStart = t + currentNode.offset;
			int nextEffectiveStart = nextT + nextOffset;

			// GlideがONの場合は次の音に繋げ（隙間0）、OFFの場合は1 Tickの隙間を空ける
			int gap = currentNode.glide ? 0 : 1;
			int maxAllowedEnd = nextEffectiveStart - gap;
			int currentEffectiveEnd = currentEffectiveStart + currentNode.length;

			// 次の音に被る場合は長さを切り詰める
			if (currentEffectiveEnd > maxAllowedEnd) {
				currentNode.length = juce::jmax(1, maxAllowedEnd - currentEffectiveStart);
			}
		}
	}

	// UIとDSP双方に更新を通知
	patternUpdated.store(true);
	uiNeedsUpdate.store(true);
}

void BassLineMatrixAudioProcessor::processBlock(juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midiMessages) {
	juce::ScopedNoDenormals noDenormals;
	buffer.clear(); midiMessages.clear();

	if (patternUpdated.exchange(false)) {
		for (int s = 0; s < 4; ++s) {
			for (int r = 0; r < 12; ++r) std::memcpy(patternDSP[s][r], patternUI[s][r], sizeof(patternUI[s][r]));
			for (int i = 0; i < 8; ++i) barSettingsDSP[s][i] = barSettingsUI[s][i];
		}
		timeSigNumDSP = timeSigNumerator.load(); timeSigDenDSP = timeSigDenominator.load();
		globalBarCountDSP = globalBarCount.load(); currentSlotDSP = currentSlot.load();
		internalSynth.setGlideTime(glideTimeMs.load());
		internalSynth.setSoundType(bassSoundType.load());
		internalSynth.setStaccatoRatio(staccatoRatio.load());
		for (auto& v : chordPreviewVoices) v.setSoundType(chordSoundType.load());
	}

	globalPitchShiftDSP = globalPitchShift.load();
	double bpm = juce::jmax(20.0, internalTempo.load());
	bool isPlaying = isPlayingInternal.load();
	double ppqPosition = 0.0; bool hasPpq = false;

	// 1. DAWから再生位置（プレイヘッド）を取得
	if (isSyncEnabled.load()) {
		if (auto* playHead = getPlayHead()) {
			if (auto pos = playHead->getPosition()) {
				if (pos->getBpm().hasValue()) bpm = *pos->getBpm();
				isPlaying = pos->getIsPlaying();
				if (pos->getPpqPosition().hasValue()) { ppqPosition = *pos->getPpqPosition(); hasPpq = true; }
			}
		}
	}

	// ==============================================================================
	// ★ 修正ブロック 3: PluginProcessor.cpp / processBlock() の中盤
	// ==============================================================================
// 2. 変数の計算・定義（必ず if で使う前に定義する）
	double sampleRate = juce::jmax(44100.0, getSampleRate());
	double samplesPerQuarterNote = sampleRate * (60.0 / bpm);
	double samplesPerTick = (samplesPerQuarterNote * (4.0 / (double)timeSigDenDSP)) / 24.0;

	// --- 修正後：分母(Den)を考慮した動的計算 ---
	// 1拍あたりのTick数を計算（分母4なら24、分母8なら12、分母2なら48）
	int ticksPerBeat = (24 * 4) / juce::jmax(1, (int)timeSigDenDSP);

	// 1小節あたりのTick数
	int ticksPerBar = timeSigNumDSP * ticksPerBeat;

	// ★追加：ループ全体のTick数を計算（小節数 × 1小節のTick数）
	int totalTicksInLoop = static_cast<int>(globalBarCountDSP) * ticksPerBar;

	// 1024を超えないようにガード
	if (totalTicksInLoop > 1024) totalTicksInLoop = 1024;

	double loopLengthSamples = (double)totalTicksInLoop * samplesPerTick;

	// ★ 課題2の修正: 再生が「ストップした瞬間」のみ、1回だけリソースを解放・ミュートする
	if (!isPlaying && lastPlayingState) {
		samplesInLoop = 0; currentStep = -1;
		for (int n = 0; n < 128; ++n) {
			if (activeNoteCountdowns[n] > 0) {
				midiMessages.addEvent(juce::MidiMessage::noteOff(1, n), 0); // バッファ先頭で即座にNoteOff
				activeNoteCountdowns[n] = 0;
			}
		}
		internalSynth.release();
		for (auto& v : chordPreviewVoices) v.release();
	}
	lastPlayingState = isPlaying; // 状態を更新

	// 3. DAWとの同期とジッターガード
	if (isSyncEnabled.load() && isPlaying) { // ★ isPlaying の時のみ同期位置を計算
		if (hasPpq && totalTicksInLoop > 0) {
			double exactTicks = ppqPosition * 24.0 * ((double)timeSigDenDSP / 4.0);
			double exactTicksInLoop = std::fmod(exactTicks, (double)totalTicksInLoop);
			if (exactTicksInLoop < 0.0) exactTicksInLoop += totalTicksInLoop;

			int newSamplesInLoop = (int)(exactTicksInLoop * samplesPerTick);

			// 【ジッターガード】DAWの微小な揺れでダブルトリガーが発生するのを防ぐ
			if (std::abs(newSamplesInLoop - samplesInLoop) > samplesPerTick) {
				samplesInLoop = newSamplesInLoop;
				currentStep = -1; // 位置が飛んだらステップもリセット
			}
		}
	}

	auto* left = buffer.getWritePointer(0);
	auto* right = buffer.getNumChannels() > 1 ? buffer.getWritePointer(1) : nullptr;

	for (int i = 0; i < buffer.getNumSamples(); ++i) {

		// ==============================================================================
		// ★ カウントダウンと Note Off 処理
		// ==============================================================================
		for (int n = 0; n < 128; ++n) {
			if (activeNoteCountdowns[n] > 0) {
				activeNoteCountdowns[n]--;
				if (activeNoteCountdowns[n] <= 0) {
					midiMessages.addEvent(juce::MidiMessage::noteOff(1, n), i);
					internalSynth.release();
					activeNoteCountdowns[n] = 0;
				}
			}
		}

		if (chordOffCountdown > 0) {
			chordOffCountdown--;
			if (chordOffCountdown <= 0) {
				for (auto& v : chordPreviewVoices) v.release();
			}
		}

		// ==============================================================================
		// ★ プレビュー用ノートの処理
		// ==============================================================================
		int pNote = previewNoteMidi.exchange(-1);
		if (pNote >= 0) {
			for (int n = 0; n < 128; ++n) {
				if (activeNoteCountdowns[n] > 0) {
					midiMessages.addEvent(juce::MidiMessage::noteOff(1, n), i);
					activeNoteCountdowns[n] = 0;
				}
			}
			float freq = 440.0f * std::pow(2.0f, (pNote - 69) / 12.0f);
			midiMessages.addEvent(juce::MidiMessage::noteOn(1, pNote, (juce::uint8)100), i);
			internalSynth.trigger(freq, 100.0f, false);
			activeNoteCountdowns[pNote] = (int)(sampleRate * 0.3);
		}

		// （※以前ここにあった「再生停止時の処理 (if (!isPlaying))」は完全に削除されました）

		// ==============================================================================
		// ★ ループ駆動・シーケンサー処理
		// ==============================================================================
		// ★ 課題2の修正: isPlaying の時のみシーケンサーを進める構造に変更
		if (isPlaying && loopLengthSamples > 0.0) {
			samplesInLoop++; if (samplesInLoop >= loopLengthSamples) samplesInLoop = 0;
			int tick = (int)(samplesInLoop / samplesPerTick);
			if (tick >= totalTicksInLoop) tick = totalTicksInLoop - 1;
			if (totalTicksInLoop > 0) currentPlayingBar.store(tick / (timeSigNumDSP * 24));
			if (tick != currentStep && tick < 1024) {
				currentStep = tick;
				int barIdx = currentStep / (timeSigNumDSP * 24);

				// --- コードトリガー ---
				if (isChordOn.load() && tick % 24 == 0) {
					int beatInBar = (tick % (timeSigNumDSP * 24)) / 24;
					auto& currentBarSettings = barSettingsDSP[currentSlotDSP][barIdx];

					if (currentBarSettings.useCodeMode) {
						ChordDef chord = currentBarSettings.chords[beatInBar];
						bool shouldTrigger = (chordTriggerMode.load() == 1 || beatInBar == 0);

						if (!shouldTrigger && beatInBar > 0) {
							auto prev = currentBarSettings.chords[beatInBar - 1];
							if (prev.degree != chord.degree || prev.quality != chord.quality || prev.inversion != chord.inversion)
								shouldTrigger = true;
						}

						if (shouldTrigger) {
							int baseNote = 60 + currentBarSettings.key;
							int scale = currentBarSettings.scale;
							int sLen = scaleLengths[scale];
							if (sLen < 5) sLen = 7;

							int rOffset = (scalePatterns[scale][chord.degree % sLen] != -1) ?
								scalePatterns[scale][chord.degree % sLen] :
								(std::array<int, 7>{0, 2, 4, 5, 7, 9, 11})[chord.degree % 7];

							int rootMidi = baseNote + rOffset + globalPitchShiftDSP + (chordOctave.load() * 12);

							// ==========================================================
							// ★ 修正箇所：5和音・15種類完全対応のコード生成
							// ==========================================================
							int notes[5] = { rootMidi, 0, 0, 0, 0 };
							int activeVoices = 3;

							switch (chord.quality) {
							case ChordQuality::Major:   notes[1] = rootMidi + 4; notes[2] = rootMidi + 7; activeVoices = 3; break;
							case ChordQuality::Minor:   notes[1] = rootMidi + 3; notes[2] = rootMidi + 7; activeVoices = 3; break;
							case ChordQuality::Dom7:    notes[1] = rootMidi + 4; notes[2] = rootMidi + 7; notes[3] = rootMidi + 10; activeVoices = 4; break;
							case ChordQuality::Min7:    notes[1] = rootMidi + 3; notes[2] = rootMidi + 7; notes[3] = rootMidi + 10; activeVoices = 4; break;
							case ChordQuality::Maj7:    notes[1] = rootMidi + 4; notes[2] = rootMidi + 7; notes[3] = rootMidi + 11; activeVoices = 4; break;
							case ChordQuality::Dim:     notes[1] = rootMidi + 3; notes[2] = rootMidi + 6; activeVoices = 3; break;
							case ChordQuality::HalfDim: notes[1] = rootMidi + 3; notes[2] = rootMidi + 6; notes[3] = rootMidi + 10; activeVoices = 4; break;
							case ChordQuality::Dim7:    notes[1] = rootMidi + 3; notes[2] = rootMidi + 6; notes[3] = rootMidi + 9; activeVoices = 4; break;
							case ChordQuality::Power:   notes[1] = rootMidi + 7; notes[2] = rootMidi + 12; activeVoices = 3; break;
							case ChordQuality::Min9:    notes[1] = rootMidi + 3; notes[2] = rootMidi + 7; notes[3] = rootMidi + 10; notes[4] = rootMidi + 14; activeVoices = 5; break;
							case ChordQuality::Maj9:    notes[1] = rootMidi + 4; notes[2] = rootMidi + 7; notes[3] = rootMidi + 11; notes[4] = rootMidi + 14; activeVoices = 5; break;
							case ChordQuality::Dom7b9:  notes[1] = rootMidi + 4; notes[2] = rootMidi + 7; notes[3] = rootMidi + 10; notes[4] = rootMidi + 13; activeVoices = 5; break;
							case ChordQuality::Dom7alt: notes[1] = rootMidi + 4; notes[2] = rootMidi + 6; notes[3] = rootMidi + 10; notes[4] = rootMidi + 15; activeVoices = 5; break;
							case ChordQuality::Dom13:   notes[1] = rootMidi + 4; notes[2] = rootMidi + 10; notes[3] = rootMidi + 14; notes[4] = rootMidi + 21; activeVoices = 5; break;
							case ChordQuality::Aug:     notes[1] = rootMidi + 4; notes[2] = rootMidi + 8; activeVoices = 3; break;
							default:                    notes[1] = rootMidi + 4; notes[2] = rootMidi + 7; activeVoices = 3; break;
							}

							for (int inv = 0; inv < chord.inversion; ++inv) notes[inv % activeVoices] += 12;
							std::sort(notes, notes + activeVoices);

							// ★ ボイス数の上限(5)に合わせて音をトリガー
							for (int v = 0; v < activeVoices; ++v) {
								chordPreviewVoices[v].trigger(440.0f * std::pow(2.0f, (notes[v] - 69) / 12.0f));
							}

							// 使わなかった余分なボイスを確実にミュート
							for (int v = activeVoices; v < 5; ++v) {
								chordPreviewVoices[v].release();
							}

							chordOffCountdown = (int)(samplesPerTick * 24 * 0.95);
						}
					}
				}

				// --- ベーストリガー（完全モノフォニック対応） ---
				int slot = currentSlotDSP;
				for (int row = 0; row < 12; ++row) {
					const auto& step = patternDSP[slot][row][currentStep % 1024];

					if (step.velocity > 0) {
						int midiNote = getMidiNoteFromRow(row, currentStep, step.octave, slot, true);

						// 新しい音を鳴らす前に古い音を確実に止める
						for (int n = 0; n < 128; ++n) {
							if (activeNoteCountdowns[n] > 0) {
								midiMessages.addEvent(juce::MidiMessage::noteOff(1, n), i);
								activeNoteCountdowns[n] = 0;
							}
						}

						midiMessages.addEvent(juce::MidiMessage::noteOn(1, midiNote, (juce::uint8)step.velocity), i);

						// 発音の長さ（サンプル数）を計算
						int lenSamples = (int)(step.length * samplesPerTick);
						if (step.staccato) {
							lenSamples = (int)(lenSamples * staccatoRatio.load());
						}

						// ★【修正】人間の耳とシンセが認識できる「最低限の長さ（約20ミリ秒）」を強制保証する
						int minAudibleSamples = (int)(sampleRate * 0.02); // 0.02秒 = 20ms
						if (lenSamples < minAudibleSamples) {
							lenSamples = minAudibleSamples;
						}

						// 対象ノートのカウントダウンをセット
						activeNoteCountdowns[midiNote] = lenSamples;
						float freq = 440.0f * std::pow(2.0f, (midiNote - 69) / 12.0f);
						internalSynth.trigger(freq, step.velocity, step.glide);

						break;
					}
				}
			}
		}

		// ==============================================================================
		// ★ オーディオ出力合成
		// ==============================================================================
		float out = isBassOn.load() ? internalSynth.process() * bassVolume.load() : 0.0f;
		float chordOut = 0.0f;

		if (isChordOn.load()) {
			for (auto& v : chordPreviewVoices) chordOut += v.process();
			chordOut *= chordVolume.load();
		}

		if (left) left[i] = out + chordOut;
		if (right) right[i] = out + chordOut;
	}
}


const juce::String BassLineMatrixAudioProcessor::getName() const { return JucePlugin_Name; }
bool BassLineMatrixAudioProcessor::acceptsMidi() const { return false; }
bool BassLineMatrixAudioProcessor::producesMidi() const { return true; }
bool BassLineMatrixAudioProcessor::isMidiEffect() const { return false; }
double BassLineMatrixAudioProcessor::getTailLengthSeconds() const { return 0.0; }
int BassLineMatrixAudioProcessor::getNumPrograms() { return 1; }
int BassLineMatrixAudioProcessor::getCurrentProgram() { return 0; }
void BassLineMatrixAudioProcessor::setCurrentProgram(int index) {}
const juce::String BassLineMatrixAudioProcessor::getProgramName(int index) { return {}; }
void BassLineMatrixAudioProcessor::changeProgramName(int index, const juce::String& newName) {}
bool BassLineMatrixAudioProcessor::hasEditor() const { return true; }
juce::AudioProcessorEditor* BassLineMatrixAudioProcessor::createEditor() { return new BassLineMatrixAudioProcessorEditor(*this); }

// --- 保存：DAWがプロジェクトを保存するとき ---
void BassLineMatrixAudioProcessor::getStateInformation(juce::MemoryBlock& destData) {
	juce::MemoryOutputStream stream(destData, true);

	// 1. 基本設定
	stream.writeInt(timeSigNumerator.load());
	stream.writeInt(timeSigDenominator.load());
	stream.writeInt(globalBarCount.load());
	stream.writeDouble(internalTempo.load());
	stream.writeBool(isTempoLocked.load());
	stream.writeBool(isSyncEnabled.load());
	stream.writeInt(currentSlot.load());
	stream.writeInt(globalPitchShift.load());
	stream.writeInt(currentGenre.load());
	stream.writeInt(baseOctave.load());
	stream.writeInt(glideTimeMs.load());

	// 2. 音源・ボリューム設定
	stream.writeBool(isChordOn.load());
	stream.writeBool(isBassOn.load());
	stream.writeInt(chordSoundType.load());
	stream.writeInt(bassSoundType.load());
	stream.writeFloat(staccatoRatio.load());
	stream.writeBool(isStaccatoLocked.load()); // スタッカートLock状態
	stream.writeFloat(chordVolume.load());
	stream.writeFloat(bassVolume.load());
	stream.writeInt(chordTriggerMode.load());
	stream.writeInt(chordOctave.load());

	// 3. 構造体データ (Setting 1 の最小・最大設定)
	stream.write(&genSettings, sizeof(GlobalSettings));

	// 4. パターンデータと小節設定
	stream.write(patternUI, sizeof(patternUI));
	stream.write(barSettingsUI, sizeof(barSettingsUI));
}

// --- 読み込み：DAWがプロジェクトを開くとき ---
void BassLineMatrixAudioProcessor::setStateInformation(const void* data, int sizeInBytes) {
	juce::MemoryInputStream stream(data, static_cast<size_t>(sizeInBytes), false);

	// 保存した時と「1ビットもズレない正確な順番」で読み込む
	if (stream.getNumBytesRemaining() > 0) timeSigNumerator.store(stream.readInt());
	if (stream.getNumBytesRemaining() > 0) timeSigDenominator.store(stream.readInt());
	if (stream.getNumBytesRemaining() > 0) globalBarCount.store(stream.readInt());
	if (stream.getNumBytesRemaining() > 0) internalTempo.store(stream.readDouble());
	if (stream.getNumBytesRemaining() > 0) isTempoLocked.store(stream.readBool());
	if (stream.getNumBytesRemaining() > 0) isSyncEnabled.store(stream.readBool());
	if (stream.getNumBytesRemaining() > 0) currentSlot.store(stream.readInt());
	if (stream.getNumBytesRemaining() > 0) globalPitchShift.store(stream.readInt());
	if (stream.getNumBytesRemaining() > 0) currentGenre.store(stream.readInt());
	if (stream.getNumBytesRemaining() > 0) baseOctave.store(stream.readInt());
	if (stream.getNumBytesRemaining() > 0) glideTimeMs.store(stream.readInt());

	if (stream.getNumBytesRemaining() > 0) isChordOn.store(stream.readBool());
	if (stream.getNumBytesRemaining() > 0) isBassOn.store(stream.readBool());
	if (stream.getNumBytesRemaining() > 0) chordSoundType.store(stream.readInt());
	if (stream.getNumBytesRemaining() > 0) bassSoundType.store(stream.readInt());
	if (stream.getNumBytesRemaining() > 0) staccatoRatio.store(stream.readFloat());
	if (stream.getNumBytesRemaining() > 0) isStaccatoLocked.store(stream.readBool());
	if (stream.getNumBytesRemaining() > 0) chordVolume.store(stream.readFloat());
	if (stream.getNumBytesRemaining() > 0) bassVolume.store(stream.readFloat());
	if (stream.getNumBytesRemaining() > 0) chordTriggerMode.store(stream.readInt());
	if (stream.getNumBytesRemaining() > 0) chordOctave.store(stream.readInt());

	if (stream.getNumBytesRemaining() >= sizeof(GlobalSettings))
		stream.read(&genSettings, sizeof(GlobalSettings));

	if (stream.getNumBytesRemaining() >= sizeof(patternUI))
		stream.read(patternUI, sizeof(patternUI));

	if (stream.getNumBytesRemaining() >= sizeof(barSettingsUI))
		stream.read(barSettingsUI, sizeof(barSettingsUI));

	// UIとDSPの同期フラグを立てる
	patternUpdated.store(true);
	uiNeedsUpdate.store(true);
}
juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter() { return new BassLineMatrixAudioProcessor(); }