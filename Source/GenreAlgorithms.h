// ==============================================================================
// Source/GenreAlgorithms.h
// ==============================================================================
#pragma once
#include <array>

// --- GenreAlgorithms.h 内 ---
enum class ChordQuality {
    Major,
    Minor,      // 「Min」ではなく「Minor」にする
    Dom7,
    Min7,
    Maj7,
    Dim,
    HalfDim,
    Dim7,
    Power,
    Min9,
    Maj9,
    Dom7b9,
    Dom7alt,
    Dom13,
    Aug
};
struct ChordDef {
    int degree = 0;
    ChordQuality quality = ChordQuality::Major;
    int inversion = 0;
};

struct RangeSetting { int min; int max; };

struct GenreDefinition {
    int id;
    const char* name;

    int minTempo; int maxTempo;
    int defaultTimeSigNum; int defaultTimeSigDen;
    int defaultDiv;
    int humanizeMax;

    bool enableGlide; int glideProb;
    int staccatoProb; int octaveJumpProb;

    int pRoot; int p3rd; int p5th; int p7th;
    int probDownbeat; int probBackbeat; int prob16thOff; int probSyncopation;

    int probScaleOut;
    std::array<int, 4> chordOutNotes;

    int defaultBassSound;
    int defaultChordSound;
    float defaultBassVol;
    float defaultChordVol;
    float defaultStaccato;

    // ★ 新規: Setting 1 に適用されるジャンル専用のMin/Max値
    RangeSetting defVel;
    RangeSetting defLen;
    RangeSetting defCmplx;
    RangeSetting defEntrp;
    RangeSetting defGlide;

    std::array<std::array<ChordDef, 16>, 16> chordPatterns;
};

extern const std::array<GenreDefinition, 23> genreRegistry;