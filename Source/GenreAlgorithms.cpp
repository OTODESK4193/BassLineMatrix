// ==============================================================================
// Source/GenreAlgorithms.cpp
// ==============================================================================
#include "GenreAlgorithms.h"

// --- 展開形対応コード定義マクロ ---
#define CH_4(d,q,i) {d, ChordQuality::q, i}, {d, ChordQuality::q, i}, {d, ChordQuality::q, i}, {d, ChordQuality::q, i}
#define PAT_16(b1, b2, b3, b4) std::array<ChordDef, 16>{{ b1, b2, b3, b4 }}

// --- ★修正：16パターンのコードスロットを埋めるための新マクロ ---
#define FILL_16_SAME(p) std::array<std::array<ChordDef, 16>, 16>{{ p, p, p, p, p, p, p, p, p, p, p, p, p, p, p, p }}
#define FILL_16_PATTERNS(p1, p2, p3, p4) std::array<std::array<ChordDef, 16>, 16>{{ p1, p2, p3, p4, p1, p2, p3, p4, p1, p2, p3, p4, p1, p2, p3, p4 }}

#define R(min, max) {min, max}

const std::array<GenreDefinition, 23> genreRegistry = { {

        // 1. Techno (Dark & Hypnotic)
                // 推奨スケール: Natural Minor, Phrygian, Locrian, Harmonic Minor
                { 1, "1. Techno", 132, 132, 4, 4, 6, 0, false, 0, 30, 20, 80, 0, 20, 0, 90, 5, 40, 5,
                  15, {1, 6, 8, 10}, 2, 0, 0.9f, 0.2f, 0.85f,
                  R(60, 115), R(4, 6), R(5, 75), R(10, 30), R(0, 0),
                  std::array<std::array<ChordDef, 16>, 16>{{
                    PAT_16(CH_4(0,Minor,0), CH_4(0,Minor,0), CH_4(0,Minor,0), CH_4(0,Minor,0)), // 01. Static Drone
                    PAT_16(CH_4(0,Minor,0), CH_4(3,Minor,1), CH_4(0,Minor,0), CH_4(4,Minor,2)), // 02. Detroit Shift
                    PAT_16(CH_4(0,Minor,0), CH_4(1,Major,0), CH_4(0,Minor,0), CH_4(1,Major,0)), // 03. Phrygian Tension (i - bII)
                    PAT_16(CH_4(0,Minor,0), CH_4(0,Minor,0), CH_4(4,Minor,0), CH_4(0,Minor,0)), // 04. Minimal 5th
                    PAT_16(CH_4(0,Minor,0), CH_4(2,Major,0), CH_4(0,Minor,0), CH_4(6,Major,0)), // 05. Harmonic Walk
                    PAT_16(CH_4(0,Minor,0), CH_4(7,Dim,0), CH_4(0,Minor,0), CH_4(1,Dim,0)),     // 06. Dark Locrian
                    PAT_16(CH_4(0,Minor,0), CH_4(0,Minor,0), CH_4(3,Major,0), CH_4(3,Major,0)), // 07. Parallel Minor/Major
                    PAT_16(CH_4(0,Minor,0), CH_4(5,Major,0), CH_4(4,Minor,0), CH_4(0,Minor,0)), // 08. Epic Descent
                    PAT_16(CH_4(0,Minor,0), CH_4(1,Minor,0), CH_4(2,Minor,0), CH_4(1,Minor,0)), // 09. Chromatic Step
                    PAT_16(CH_4(0,Minor,0), CH_4(0,Minor,1), CH_4(0,Minor,2), CH_4(0,Minor,1)), // 10. Octave Leap
                    PAT_16(CH_4(0,Minor,0), CH_4(6,Minor,0), CH_4(0,Minor,0), CH_4(6,Minor,0)), // 11. Distant Relation
                    PAT_16(CH_4(0,Minor,0), CH_4(0,Minor,0), CH_4(1,Major,1), CH_4(0,Minor,0)), // 12. Sudden bII
                    PAT_16(CH_4(0,Minor,0), CH_4(4,Minor,1), CH_4(0,Minor,0), CH_4(4,Minor,1)), // 13. Call & Response
                    PAT_16(CH_4(0,Minor,0), CH_4(0,Minor,0), CH_4(5,Major,0), CH_4(4,Major,0)), // 14. Classic Walkdown
                    PAT_16(CH_4(0,Minor,0), CH_4(2,Minor,0), CH_4(4,Minor,0), CH_4(6,Minor,0)), // 15. Tension Build
                    PAT_16(CH_4(0,Minor,0), CH_4(0,Minor,0), CH_4(0,Minor,0), CH_4(1,Major,0))  // 16. Phrygian Release
                  }}
                },

    // ==============================================================================
    // Source/GenreAlgorithms.cpp (抜粋: ID 2 のDNA定義差し替え)
    // ==============================================================================

    // 2. House (Modern K-Pop / Deep Jacking Vibe)
    { 2, "2. House", 120, 128, 4, 4, 6, 0, false, 0, 20, 40, 60, 0, 40, 0, 80, 60, 60, 20,
      10, {3, 6, 10, 13}, 0, 3, 1.0f, 0.4f, 0.6f,
      R(70, 110), R(4, 12), R(5, 45), R(15, 40), R(0,0), // Glide 0, Humanize 0
      std::array<std::array<ChordDef, 16>, 16>{{
        PAT_16(CH_4(0,Min9,0), CH_4(3,Dom13,0), CH_4(0,Min9,0), CH_4(3,Dom13,0)),   // Dorian Jacking (Im9 - IV13)
        PAT_16(CH_4(1,Min9,0), CH_4(4,Dom13,0), CH_4(0,Maj9,0), CH_4(5,Dom7alt,0)), // Soulful 2-5-1
        PAT_16(CH_4(0,Min9,0), CH_4(4,Min9,0), CH_4(3,Min9,0), CH_4(0,Min9,0)),     // Deep Modal Shift
        PAT_16(CH_4(5,Min9,0), CH_4(4,Dom13,0), CH_4(3,Maj9,0), CH_4(2,Dom7alt,0)), // Emotional Walkdown
        PAT_16(CH_4(0,Maj9,0), CH_4(5,Min9,0), CH_4(1,Min9,0), CH_4(4,Dom13,0)),    // Smooth Jazz Loop
        PAT_16(CH_4(0,Min9,0), CH_4(6,Maj9,0), CH_4(5,Maj9,0), CH_4(4,Dom7alt,0)),  // Classic Acid/Deep
        PAT_16(CH_4(3,Maj9,0), CH_4(4,Dom13,0), CH_4(5,Min9,0), CH_4(0,Dom13,0)),   // R&B Lift
        PAT_16(CH_4(0,Min9,0), CH_4(1,Min9,0), CH_4(2,Maj9,0), CH_4(4,Dom7b9,0)),   // Funky Turnaround
        PAT_16(CH_4(0,Min9,0), CH_4(1,Maj9,0), CH_4(0,Min9,0), CH_4(1,Maj9,0)),     // Phrygian House
        PAT_16(CH_4(3,Maj9,0), CH_4(2,Min7,0), CH_4(1,Min9,0), CH_4(4,Dom7alt,0)),  // Falling Soul
        PAT_16(CH_4(0,Maj9,0), CH_4(0,Dim7,0), CH_4(1,Min9,0), CH_4(4,Dom13,0)),    // Passing Diminished
        PAT_16(CH_4(1,HalfDim,0), CH_4(4,Dom7alt,0), CH_4(0,Min9,0), CH_4(5,Dom7alt,0)), // Minor 2-5-1
        PAT_16(CH_4(0,Min9,0), CH_4(3,Min9,0), CH_4(6,Dom13,0), CH_4(2,Maj9,0)),    // Incognito Vibe
        PAT_16(CH_4(5,Min9,0), CH_4(1,Dom7alt,0), CH_4(4,Min9,0), CH_4(0,Dom13,0)), // Dark Jazz
        PAT_16(CH_4(0,Maj9,0), CH_4(2,Maj9,0), CH_4(5,Maj9,0), CH_4(1,Maj9,0)),     // Parallel Float
        PAT_16(CH_4(0,Dom13,0), CH_4(3,Dom13,0), CH_4(0,Dom13,0), CH_4(4,Min9,0))   // Oldschool Garage
      }}
    },
    // 3. UK Garage (K-Pop 2-Step / R&B Vibe)
    { 3, "3. UK Garage", 132, 132, 4, 4, 6, 0, false, 0, 40, 30, 50, 20, 20, 10, 70, 20, 80, 60,
      20, {1, 3, 5, 10}, 0, 1, 1.1f, 0.35f, 0.45f,
      R(75, 115), R(4, 18), R(5, 50), R(0, 30), R(0, 0), // Glide 0, Humanize 0
      std::array<std::array<ChordDef, 16>, 16>{{
        PAT_16(CH_4(3,Maj9,0), CH_4(2,Dom7alt,0), CH_4(5,Min9,0), CH_4(0,Dom13,0)), // IV - IIIalt - VIm - I13
        PAT_16(CH_4(1,Min9,0), CH_4(4,Dom13,0), CH_4(0,Maj9,0), CH_4(5,Dom7b9,0)),  // Modern 2-5-1
        PAT_16(CH_4(0,Min9,0), CH_4(6,Maj9,0), CH_4(5,Maj9,0), CH_4(4,Dom7alt,0)),  // Descending Modal
        PAT_16(CH_4(3,Maj9,0), CH_4(4,Dom13,0), CH_4(0,Min9,0), CH_4(0,Min9,0)),    // IV - V - Im
        PAT_16(CH_4(5,Min9,0), CH_4(1,Dom13,0), CH_4(4,Min9,0), CH_4(0,Dom13,0)),
        PAT_16(CH_4(0,Maj9,0), CH_4(2,Min7,0), CH_4(1,Min9,0), CH_4(4,Dom13,0)),
        PAT_16(CH_4(1,HalfDim,0), CH_4(4,Dom7alt,0), CH_4(0,Min9,0), CH_4(5,Dom7b9,0)),
        PAT_16(CH_4(3,Maj9,0), CH_4(2,Min9,0), CH_4(1,Min9,0), CH_4(0,Maj9,0)),
        PAT_16(CH_4(0,Min9,0), CH_4(1,Maj9,0), CH_4(0,Min9,0), CH_4(1,Maj9,0)),
        PAT_16(CH_4(5,Min9,0), CH_4(4,Dom13,0), CH_4(3,Maj9,0), CH_4(2,Dom7alt,0)),
        PAT_16(CH_4(0,Maj9,0), CH_4(6,HalfDim,0), CH_4(5,Dom7alt,0), CH_4(1,Min9,0)),
        PAT_16(CH_4(3,Maj9,0), CH_4(3,Min9,0), CH_4(0,Maj9,0), CH_4(0,Dom13,0)),
        PAT_16(CH_4(1,Min9,0), CH_4(1,Min9,0), CH_4(0,Maj9,0), CH_4(6,Dom13,0)),
        PAT_16(CH_4(0,Min9,0), CH_4(0,Dim7,0), CH_4(1,Min9,0), CH_4(4,Dom13,0)),
        PAT_16(CH_4(3,Maj9,0), CH_4(5,Min9,0), CH_4(0,Maj9,0), CH_4(4,Dom13,0)),
        PAT_16(CH_4(0,Min9,0), CH_4(4,Min9,0), CH_4(0,Min9,0), CH_4(4,Dom7alt,0))
      }}
    },

    // 4. Drum & Bass (Liquid / "Super Shy" Vibe)
    { 4, "4. Drum & Bass", 170, 170, 4, 4, 6, 0, false, 0, 20, 20, 70, 10, 30, 0, 80, 40, 60, 30,
      15, {1, 3, 6, 10}, 1, 4, 1.2f, 0.25f, 0.75f,
      R(80, 120), R(4, 24), R(5, 60), R(10, 35), R(0, 0), // Glide 0, Humanize 0
      std::array<std::array<ChordDef, 16>, 16>{{
        PAT_16(CH_4(5,Min9,0), CH_4(1,Min9,0), CH_4(4,Dom13,0), CH_4(0,Maj9,0)),    // VIm - IIm - V - I
        PAT_16(CH_4(3,Maj9,0), CH_4(3,Min9,0), CH_4(2,Min7,0), CH_4(5,Dom7alt,0)),  // IV - IVm - IIIm - VI
        PAT_16(CH_4(0,Min9,0), CH_4(3,Dom13,0), CH_4(6,Maj9,0), CH_4(2,HalfDim,0)),
        PAT_16(CH_4(1,Min9,0), CH_4(4,Dom13,0), CH_4(0,Maj9,0), CH_4(0,Maj9,0)),
        PAT_16(CH_4(5,Min9,0), CH_4(4,Min9,0), CH_4(3,Maj9,0), CH_4(2,Dom7alt,0)),
        PAT_16(CH_4(0,Maj9,0), CH_4(2,Min9,0), CH_4(3,Maj9,0), CH_4(4,Dom13,0)),
        PAT_16(CH_4(0,Min9,0), CH_4(1,Maj9,0), CH_4(0,Min9,0), CH_4(6,Dom7alt,0)),
        PAT_16(CH_4(3,Maj9,0), CH_4(4,Dom13,0), CH_4(5,Min9,0), CH_4(0,Dom13,0)),
        PAT_16(CH_4(1,HalfDim,0), CH_4(4,Dom7b9,0), CH_4(0,Min9,0), CH_4(0,Min9,0)),
        PAT_16(CH_4(0,Maj9,0), CH_4(6,Dom13,0), CH_4(1,Min9,0), CH_4(4,Dom7alt,0)),
        PAT_16(CH_4(5,Min9,0), CH_4(0,Dom13,0), CH_4(3,Maj9,0), CH_4(6,HalfDim,0)),
        PAT_16(CH_4(3,Maj9,0), CH_4(2,Min9,0), CH_4(1,Min9,0), CH_4(0,Maj9,0)),
        PAT_16(CH_4(0,Min9,0), CH_4(4,Dom13,0), CH_4(0,Min9,0), CH_4(4,Dom7alt,0)),
        PAT_16(CH_4(3,Maj9,0), CH_4(3,Maj9,0), CH_4(0,Min9,0), CH_4(0,Min9,0)),
        PAT_16(CH_4(1,Min9,0), CH_4(1,Min9,0), CH_4(4,Dom13,0), CH_4(4,Dom7alt,0)),
        PAT_16(CH_4(5,Min9,0), CH_4(6,Maj9,0), CH_4(0,Maj9,0), CH_4(2,Dom7alt,0))
      }}
    },

                    // ==============================================================================
                    // 5. Trap (808 Glide & Dark Cinematic Bounce)
                    // ==============================================================================
                { 5, "5. Trap", 130, 150, 4, 4, 6, 0, false, 0, 30, 40, 80, 20, 60, 50, 100, 60, 90, 50,
                  20, {1, 3, 5, 11}, 1, 4, 1.3f, 0.2f, 0.9f,
                  R(85, 127), R(6, 48), R(30, 80), R(10, 50), R(0, 0),
                  std::array<std::array<ChordDef, 16>, 16>{{
                    PAT_16(CH_4(0,Min9,0), CH_4(0,Min9,0), CH_4(0,Min9,0), CH_4(0,Min9,0)),
                    PAT_16(CH_4(0,Min9,0), CH_4(5,Min9,0), CH_4(0,Min9,0), CH_4(4,Dom13,0)),
                    PAT_16(CH_4(0,Min9,0), CH_4(5,Maj9,0), CH_4(5,Min9,0), CH_4(4,Dom13,0)),
                    PAT_16(CH_4(0,Min9,0), CH_4(4,Min9,0), CH_4(5,Min9,0), CH_4(4,Min9,0)),
                    PAT_16(CH_4(0,Min9,0), CH_4(0,Min9,0), CH_4(5,Maj9,0), CH_4(5,Maj9,0)),
                    PAT_16(CH_4(0,Min9,0), CH_4(6,Maj9,0), CH_4(5,Maj9,0), CH_4(6,Maj9,0)),
                    PAT_16(CH_4(0,Min9,0), CH_4(3,Maj9,0), CH_4(0,Maj9,0), CH_4(4,Dom13,0)),
                    PAT_16(CH_4(0,Min9,0), CH_4(6,Maj9,0), CH_4(5,Maj9,0), CH_4(4,Dom7alt,0)),
                    PAT_16(CH_4(0,Min7,0), CH_4(5,Maj9,0), CH_4(2,Maj9,0), CH_4(4,Dom13,0)),
                    PAT_16(CH_4(0,Min9,0), CH_4(5,Min9,0), CH_4(0,Min9,0), CH_4(5,Min9,0)),
                    PAT_16(CH_4(0,Min9,0), CH_4(5,Maj9,0), CH_4(2,Maj9,0), CH_4(6,Maj9,0)),
                    PAT_16(CH_4(0,Min9,0), CH_4(4,Min9,0), CH_4(0,Min9,0), CH_4(4,Min9,0)),
                    PAT_16(CH_4(0,Min9,0), CH_4(5,Maj9,0), CH_4(4,Min9,0), CH_4(5,Min9,0)),
                    PAT_16(CH_4(0,Min9,0), CH_4(5,Min9,0), CH_4(6,Maj9,0), CH_4(2,Maj9,0)),
                    PAT_16(CH_4(0,Min9,0), CH_4(0,Min7,0), CH_4(0,Min9,0), CH_4(0,Min7,0)), // ← Sus を Min7 に修正
                    PAT_16(CH_4(0,Min9,0), CH_4(2,Maj9,0), CH_4(5,Min9,0), CH_4(5,Maj9,0))
                  }}
                }, // ← カンマ等が必要な場合は適宜付与してください
                  
                  // 6. Footwork / Juke (Mellow Polyrhythm)
    { 6, "6. Footwork / Juke", 155, 155, 4, 4, 6, 0, false, 0, 50, 20, 60, 10, 30, 10, 80, 50, 70, 60,
      20, {1, 3, 6, 10}, 1, 2, 1.1f, 0.3f, 0.90f,
      R(70, 115), R(2, 8), R(0, 30), R(15, 35), R(0, 0), // Glide 0, Humanize 0
      std::array<std::array<ChordDef, 16>, 16>{{
        PAT_16(CH_4(0,Min9,0), CH_4(3,Maj9,0), CH_4(4,Dom13,0), CH_4(0,Min9,0)),
        PAT_16(CH_4(1,Min9,0), CH_4(4,Dom7alt,0), CH_4(0,Maj9,0), CH_4(5,Dom13,0)),
        PAT_16(CH_4(3,Maj9,0), CH_4(2,Min7,0), CH_4(1,Min9,0), CH_4(0,Maj9,0)),
        PAT_16(CH_4(5,Min9,0), CH_4(4,Dom13,0), CH_4(0,Maj9,0), CH_4(2,Dom7alt,0)),
        PAT_16(CH_4(0,Min9,0), CH_4(1,Maj9,0), CH_4(0,Min9,0), CH_4(6,Dom7alt,0)),
        PAT_16(CH_4(3,Maj9,0), CH_4(4,Dom13,0), CH_4(5,Min9,0), CH_4(0,Dom13,0)),
        PAT_16(CH_4(1,HalfDim,0), CH_4(4,Dom7b9,0), CH_4(0,Min9,0), CH_4(0,Min9,0)),
        PAT_16(CH_4(0,Maj9,0), CH_4(6,Dom13,0), CH_4(1,Min9,0), CH_4(4,Dom13,0)),
        PAT_16(CH_4(5,Min9,0), CH_4(1,Dom13,0), CH_4(4,Min9,0), CH_4(0,Dom13,0)),
        PAT_16(CH_4(0,Min9,0), CH_4(6,Maj9,0), CH_4(5,Maj9,0), CH_4(4,Dom7alt,0)),
        PAT_16(CH_4(3,Maj9,0), CH_4(3,Min9,0), CH_4(0,Maj9,0), CH_4(0,Maj9,0)),
        PAT_16(CH_4(0,Maj9,0), CH_4(0,Dim7,0), CH_4(1,Min9,0), CH_4(4,Dom13,0)),
        PAT_16(CH_4(1,Min9,0), CH_4(1,Min9,0), CH_4(0,Maj9,0), CH_4(0,Maj9,0)),
        PAT_16(CH_4(3,Maj9,0), CH_4(4,Dom13,0), CH_4(0,Min9,0), CH_4(0,Min9,0)),
        PAT_16(CH_4(0,Min9,0), CH_4(3,Dom13,0), CH_4(6,Maj9,0), CH_4(4,Dom7alt,0)),
        PAT_16(CH_4(5,Min9,0), CH_4(4,Min9,0), CH_4(3,Maj9,0), CH_4(2,Dom7alt,0))
      }}
    },

                // ==============================================================================
                // ★ 07. IDM: DNA定義 (Modern Jazz / Advanced Harmonics Ver.)
                // ==============================================================================
            { 7, "07. IDM", 115, 115, 4, 4, 6, 0, false, 0, 70, 50, 40, 20, 40, 30, 80, 50, 60, 40,
              20, {1, 3, 5, 10}, 1, 6, 0.8f, 0.3f, 0.55f,
              R(90, 150), R(2, 6), R(30, 80), R(20, 60), R(0, 0), // Glide 0, Humanize 0 に完全固定
              std::array<std::array<ChordDef, 16>, 16>{{
                      // 01. Modern 2-5-1 (IIm9 - V13 - IMaj9 - VI7alt)
                      PAT_16(CH_4(1,Min9,0), CH_4(4,Dom13,0), CH_4(0,Maj9,0), CH_4(5,Dom7alt,0)),
                          // 02. Coltrane/Giant Steps Vibe (IMaj9 - bIII13 - bVIMaj9 - VII7alt)
                          PAT_16(CH_4(0,Maj9,0), CH_4(2,Dom13,0), CH_4(5,Maj9,0), CH_4(6,Dom7alt,0)),
                          // 03. Modal Phrygian Shift (Im9 - bIIMaj9 - Im9 - bIIMaj9)
                          PAT_16(CH_4(0,Min9,0), CH_4(1,Maj9,0), CH_4(0,Min9,0), CH_4(1,Maj9,0)),
                          // 04. Chromatic Walkdown (Im9 - bVII13 - bVI13 - V7alt)
                          PAT_16(CH_4(0,Min9,0), CH_4(6,Dom13,0), CH_4(5,Dom13,0), CH_4(4,Dom7alt,0)),
                          // 05. Smooth R&B / D'Angelo Vibe (IVMaj9 - III7alt - VIm9 - I13)
                          PAT_16(CH_4(3,Maj9,0), CH_4(2,Dom7alt,0), CH_4(5,Min9,0), CH_4(0,Dom13,0)),
                          // 06. Diminished Passing (IMaj9 - #Idim7 - IIm9 - V13)
                          PAT_16(CH_4(0,Maj9,0), CH_4(0,Dim7,0), CH_4(1,Min9,0), CH_4(4,Dom13,0)),
                          // 07. Minor Turnaround (Im9 - IIm7b5 - V7alt - Im9)
                          PAT_16(CH_4(0,Min9,0), CH_4(1,HalfDim,0), CH_4(4,Dom7alt,0), CH_4(0,Min9,0)),
                          // 08. Parallel Step Down (IVMaj9 - IIIm7 - IIm9 - V7alt)
                          PAT_16(CH_4(3,Maj9,0), CH_4(2,Min7,0), CH_4(1,Min9,0), CH_4(4,Dom7alt,0)),
                          // 09. Neo Soul Classic (IIm9 - V13 - IIIm7 - VI7alt)
                          PAT_16(CH_4(1,Min9,0), CH_4(4,Dom13,0), CH_4(2,Min7,0), CH_4(5,Dom7alt,0)),
                          // 10. Suspended Resolution (IVMaj9 -> V13 -> IMaj9)
                          PAT_16(CH_4(3,Maj9,0), CH_4(4,Dom13,0), CH_4(0,Maj9,0), CH_4(0,Maj9,0)),
                          // 11. Lydian Dream (IVMaj9 - IVMaj9 - IMaj9 - IMaj9)
                          PAT_16(CH_4(3,Maj9,0), CH_4(3,Maj9,0), CH_4(0,Maj9,0), CH_4(0,Maj9,0)),
                          // 12. Dark Jazz (VIm9 - II7alt - Vm9 - I13)
                          PAT_16(CH_4(5,Min9,0), CH_4(1,Dom7alt,0), CH_4(4,Min9,0), CH_4(0,Dom13,0)),
                          // 13. Lady Bird Turnaround (IMaj9 - bIIIMaj9 - bVIMaj9 - bIIMaj9)
                          PAT_16(CH_4(0,Maj9,0), CH_4(2,Maj9,0), CH_4(5,Maj9,0), CH_4(1,Maj9,0)),
                          // 14. Tritone Sub (IIm9 - bII13 - IMaj9 - VI7alt)
                          PAT_16(CH_4(1,Min9,0), CH_4(1,Dom13,0), CH_4(0,Maj9,0), CH_4(5,Dom7alt,0)),
                          // 15. Funk Vamp (I13 - IV9 - I13 - IV9)
                          PAT_16(CH_4(0,Dom13,0), CH_4(3,Dom13,0), CH_4(0,Dom13,0), CH_4(3,Dom13,0)),
                          // 16. Endless Ascent (IMaj9 - IIIm9 - VIm9 - VII7alt)
                          PAT_16(CH_4(0,Maj9,0), CH_4(2,Min9,0), CH_4(5,Min9,0), CH_4(6,Dom7alt,0))
                        }}
            },

            // 8. Dubstep (Future Bass / Emotional Chillstep Vibe)
{ 8, "8. Dubstep", 135, 135, 4, 4, 6, 0, false, 0, 20, 20, 80, 10, 40, 0, 100, 30, 60, 50,
  40, {1, 4, 6, 11}, 2, 5, 1.2f, 0.4f, 0.6f,
  R(95, 127), R(3, 24), R(40, 80), R(10, 45), R(0, 20), // Glide 0, Humanize 0
  std::array<std::array<ChordDef, 16>, 16>{{
    PAT_16(CH_4(5,Min9,0), CH_4(3,Maj9,0), CH_4(0,Maj9,0), CH_4(4,Dom13,0)),
    PAT_16(CH_4(3,Maj9,0), CH_4(4,Dom13,0), CH_4(5,Min9,0), CH_4(0,Dom13,0)),
    PAT_16(CH_4(0,Min9,0), CH_4(6,Maj9,0), CH_4(5,Maj9,0), CH_4(4,Dom7alt,0)),
    PAT_16(CH_4(1,Min9,0), CH_4(4,Dom13,0), CH_4(0,Maj9,0), CH_4(5,Dom7alt,0)),
    PAT_16(CH_4(3,Maj9,0), CH_4(2,Min7,0), CH_4(1,Min9,0), CH_4(4,Dom7alt,0)),
    PAT_16(CH_4(5,Min9,0), CH_4(1,Min9,0), CH_4(4,Dom13,0), CH_4(0,Maj9,0)),
    PAT_16(CH_4(0,Maj9,0), CH_4(2,Dom13,0), CH_4(5,Maj9,0), CH_4(6,Dom7alt,0)),
    PAT_16(CH_4(0,Min9,0), CH_4(1,Maj9,0), CH_4(0,Min9,0), CH_4(1,Maj9,0)),
    PAT_16(CH_4(3,Maj9,0), CH_4(3,Min9,0), CH_4(0,Maj9,0), CH_4(0,Maj9,0)),
    PAT_16(CH_4(1,HalfDim,0), CH_4(4,Dom7b9,0), CH_4(0,Min9,0), CH_4(5,Dom7b9,0)),
    PAT_16(CH_4(0,Maj9,0), CH_4(6,Dom13,0), CH_4(1,Min9,0), CH_4(4,Dom13,0)),
    PAT_16(CH_4(5,Min9,0), CH_4(4,Min9,0), CH_4(3,Maj9,0), CH_4(2,Dom7alt,0)),
    PAT_16(CH_4(3,Maj9,0), CH_4(4,Dom13,0), CH_4(0,Min9,0), CH_4(0,Min9,0)),
    PAT_16(CH_4(0,Min9,0), CH_4(4,Dom13,0), CH_4(0,Min9,0), CH_4(4,Dom7alt,0)),
    PAT_16(CH_4(1,Min9,0), CH_4(1,Min9,0), CH_4(0,Maj9,0), CH_4(0,Maj9,0)),
    PAT_16(CH_4(5,Min9,0), CH_4(6,Maj9,0), CH_4(0,Maj9,0), CH_4(2,Dom7alt,0))
  }}
},

// ==============================================================================
// Source/GenreAlgorithms.cpp (抜粋: ID 9 のDNA定義差し替え)
// ==============================================================================

// 9. Afrobeat (Global Beats / Fela Kuti & Modern Afro-Pop)
{ 9, "9. Afrobeat", 105, 105, 4, 4, 6, 0, false, 0, 50, 40, 70, 10, 30, 20, 90, 60, 80, 40,
  15, {1, 4, 8, 12}, 1, 3, 1.1f, 0.4f, 0.7f,
  R(85, 120), R(2, 12), R(0, 80), R(10, 45), R(0, 0), // Glide 0, Humanize 0
  std::array<std::array<ChordDef, 16>, 16>{{
    PAT_16(CH_4(0,Min9,0), CH_4(3,Dom13,0), CH_4(0,Min9,0), CH_4(3,Dom13,0)),   // Fela's Dorian Vamp
    PAT_16(CH_4(0,Maj9,0), CH_4(3,Maj9,0), CH_4(4,Dom13,0), CH_4(0,Maj9,0)),    // Highlife Major
    PAT_16(CH_4(5,Min9,0), CH_4(3,Maj9,0), CH_4(0,Maj9,0), CH_4(4,Dom13,0)),    // Modern Afro-Pop
    PAT_16(CH_4(0,Min9,0), CH_4(1,Maj9,0), CH_4(0,Min9,0), CH_4(4,Dom7alt,0)),  // Afro-Cuban Shift
PAT_16(CH_4(0,Min7,0), CH_4(0,Min7,0), CH_4(3,Dom13,0), CH_4(4,Dom13,0)),   // Classic Afrobeat
PAT_16(CH_4(5,Min9,0), CH_4(4,Min9,0), CH_4(3,Maj9,0), CH_4(2,Dom7alt,0)),  // Emotional Walkdown
    PAT_16(CH_4(0,Min9,0), CH_4(6,Maj9,0), CH_4(5,Maj9,0), CH_4(4,Dom7alt,0)),  // Desert Blues
    PAT_16(CH_4(3,Maj9,0), CH_4(4,Dom13,0), CH_4(5,Min9,0), CH_4(0,Dom13,0)),   // Amapiano Lift
    PAT_16(CH_4(1,Min9,0), CH_4(4,Dom13,0), CH_4(0,Maj9,0), CH_4(5,Dom7b9,0)),  // Jazz Infusion
    PAT_16(CH_4(0,Maj9,0), CH_4(2,Min7,0), CH_4(3,Maj9,0), CH_4(4,Dom13,0)),    // Bright Savanna
    PAT_16(CH_4(0,Min9,0), CH_4(0,Min9,0), CH_4(0,Min9,0), CH_4(0,Min9,0)),     // One Chord Hypnosis
    PAT_16(CH_4(3,Maj9,0), CH_4(2,Min7,0), CH_4(1,Min9,0), CH_4(0,Maj9,0)),     // Soukous Run
    PAT_16(CH_4(0,Min9,0), CH_4(4,Min9,0), CH_4(0,Min9,0), CH_4(4,Min9,0)),     // Deep Tribal
    PAT_16(CH_4(5,Min9,0), CH_4(1,Dom13,0), CH_4(4,Min9,0), CH_4(0,Dom13,0)),   // Syncopated Roots
    PAT_16(CH_4(0,Maj9,0), CH_4(5,Min9,0), CH_4(1,Min9,0), CH_4(4,Dom13,0)),    // Sun City
    PAT_16(CH_4(0,Min9,0), CH_4(3,Min9,0), CH_4(4,Min9,0), CH_4(0,Min9,0))      // Minor Pentatonic
  }}
},

// ==============================================================================
// Source/GenreAlgorithms.cpp (抜粋: ID 10 のDNA定義)
// ==============================================================================

// 10. Gqom (Durban Dark / Broken & Hypnotic)
{ 10, "10. Gqom", 124, 128, 4, 4, 6, 0, false, 0, 20, 20, 80, 10, 30, 10, 90, 40, 70, 40,
  10, {1, 4, 8, 12}, 1, 4, 1.2f, 0.2f, 0.8f,
  R(90, 127), R(2, 12), R(10, 80), R(10, 30), R(0, 0), // Glide 0, Humanize 0
  std::array<std::array<ChordDef, 16>, 16>{{
    PAT_16(CH_4(0,Min9,0), CH_4(0,Min9,0), CH_4(0,Min9,0), CH_4(0,Min9,0)),     // Durban Dark Mono (単一反復)
    PAT_16(CH_4(0,Min9,0), CH_4(1,Maj9,0), CH_4(0,Min9,0), CH_4(1,Maj9,0)),     // Phrygian Menace (半音の恐怖)
    PAT_16(CH_4(0,Min9,0), CH_4(3,Dom7b9,0), CH_4(0,Min9,0), CH_4(3,Dom7b9,0)), // Tritone Tension (インダストリアル)
    PAT_16(CH_4(0,Min9,0), CH_4(4,Min9,0), CH_4(1,Maj9,0), CH_4(0,Min9,0)),     // Broken Minor Shift
    PAT_16(CH_4(0,Min7,0), CH_4(0,Min7,0), CH_4(0,Min7,0), CH_4(1,Dom7alt,0)),  // Hook at the End
    PAT_16(CH_4(5,Min9,0), CH_4(5,Min9,0), CH_4(5,Min9,0), CH_4(5,Min9,0)),     // Subdominant Drone
    PAT_16(CH_4(0,Min9,0), CH_4(2,Maj9,0), CH_4(0,Min9,0), CH_4(2,Maj9,0)),     // Minor Third Shift
    PAT_16(CH_4(0,Min9,0), CH_4(0,Min9,0), CH_4(3,Dom7alt,0), CH_4(0,Min9,0)),  // Sudden Dissonance
    PAT_16(CH_4(0,Min9,0), CH_4(4,Dom7alt,0), CH_4(0,Min9,0), CH_4(1,Maj9,0)),  // Dark Ritual
    PAT_16(CH_4(1,Maj9,0), CH_4(1,Maj9,0), CH_4(0,Min9,0), CH_4(0,Min9,0)),     // Descending Phrygian
    PAT_16(CH_4(0,Min9,0), CH_4(0,Min9,0), CH_4(0,Min9,0), CH_4(0,Min9,0)),     // Pure Minimal
    PAT_16(CH_4(0,Min9,0), CH_4(0,Min9,0), CH_4(6,Maj9,0), CH_4(6,Maj9,0)),     // Heavy 7th
    PAT_16(CH_4(0,Min9,0), CH_4(3,Min9,0), CH_4(0,Min9,0), CH_4(1,Maj9,0)),     // Tribal Walk
    PAT_16(CH_4(0,Min9,0), CH_4(0,Min9,0), CH_4(4,Dom7alt,0), CH_4(4,Dom7alt,0)),// Dominant Threat
    PAT_16(CH_4(5,Min9,0), CH_4(1,Maj9,0), CH_4(0,Min9,0), CH_4(0,Min9,0)),     // Dark Turnaround
    PAT_16(CH_4(0,Min9,0), CH_4(1,Maj9,0), CH_4(3,Dom7b9,0), CH_4(0,Min9,0))    // Ultimate Dissonance
  }}
},


// ==============================================================================
// Source/GenreAlgorithms.cpp (抜粋: ID 11 のDNA定義)
// ==============================================================================

// 11. Amapiano (Log Drum / Jazzy Lounge & Deep Bounce)
{ 11, "11. Amapiano", 115, 115, 4, 4, 6, 0, false, 0, 30, 40, 70, 0, 50, 0, 90, 60, 80, 50,
  30, {1, 4, 7, 10}, 0, 3, 1.2f, 0.3f, 0.8f,
  R(85, 127), R(2, 16), R(5, 75), R(15, 70), R(0, 0), // Glide 0, Humanize 0
  std::array<std::array<ChordDef, 16>, 16>{{
    PAT_16(CH_4(1,Min9,0), CH_4(4,Dom13,0), CH_4(0,Maj9,0), CH_4(5,Min9,0)),    // Classic 2-5-1-6
    PAT_16(CH_4(0,Min9,0), CH_4(3,Dom13,0), CH_4(6,Maj9,0), CH_4(2,Maj9,0)),    // Log Drum Bounce (4ths)
    PAT_16(CH_4(5,Min9,0), CH_4(1,Dom13,0), CH_4(4,Min9,0), CH_4(0,Dom13,0)),   // Private School
    PAT_16(CH_4(3,Maj9,0), CH_4(2,Min7,0), CH_4(1,Min9,0), CH_4(0,Maj9,0)),     // Emotional Amapiano
    PAT_16(CH_4(0,Min9,0), CH_4(0,Min9,0), CH_4(1,Min9,0), CH_4(4,Dom13,0)),    // Deep Lounge
    PAT_16(CH_4(0,Maj9,0), CH_4(5,Min9,0), CH_4(1,Min9,0), CH_4(4,Dom7alt,0)),  // Jazzy Turnaround
    PAT_16(CH_4(3,Maj9,0), CH_4(4,Dom13,0), CH_4(5,Min9,0), CH_4(0,Dom13,0)),   // Uplifting Sgija
    PAT_16(CH_4(1,HalfDim,0), CH_4(4,Dom7alt,0), CH_4(0,Min9,0), CH_4(5,Dom7b9,0)), // Minor 2-5-1
    PAT_16(CH_4(0,Min9,0), CH_4(1,Maj9,0), CH_4(0,Min9,0), CH_4(1,Maj9,0)),     // Phrygian House
    PAT_16(CH_4(0,Maj9,0), CH_4(2,Min7,0), CH_4(3,Maj9,0), CH_4(4,Dom13,0)),    // Bright Chords
    PAT_16(CH_4(0,Min9,0), CH_4(6,Maj9,0), CH_4(5,Maj9,0), CH_4(4,Dom7alt,0)),  // Descending Modal
    PAT_16(CH_4(3,Maj9,0), CH_4(5,Min9,0), CH_4(0,Maj9,0), CH_4(4,Dom13,0)),    // Kwaito Roots
    PAT_16(CH_4(0,Min9,0), CH_4(4,Min9,0), CH_4(0,Min9,0), CH_4(4,Dom7alt,0)),  // Tribal Minimal
    PAT_16(CH_4(1,Min9,0), CH_4(4,Dom13,0), CH_4(1,Min9,0), CH_4(4,Dom13,0)),   // Groove Vamp
    PAT_16(CH_4(5,Min9,0), CH_4(4,Dom13,0), CH_4(3,Maj9,0), CH_4(2,Dom7alt,0)), // Soulful Walkdown
    PAT_16(CH_4(0,Min9,0), CH_4(0,Dim7,0), CH_4(1,Min9,0), CH_4(4,Dom13,0))     // Passing Diminished
  }}
},

// ==============================================================================
// Source/GenreAlgorithms.cpp (抜粋: ID 12 のDNA定義)
// ==============================================================================

// 12. Indian / Bollywood (Cinematic Desi & Tabla Bounce)
{ 12, "12. Indian / Bollywood", 95, 110, 4, 4, 6, 0, false, 0, 40, 30, 80, 10, 40, 10, 90, 50, 70, 40,
  20, {1, 2, 6, 11}, 2, 4, 1.15f, 0.35f, 0.75f,
  R(80, 127), R(2, 16), R(35, 80), R(15, 35), R(0, 0), // Glide 0, Humanize 0
  std::array<std::array<ChordDef, 16>, 16>{{
    PAT_16(CH_4(0,Maj9,0), CH_4(1,Maj9,0), CH_4(0,Maj9,0), CH_4(1,Maj9,0)),     // Bhairav Bounce (I - bII)
    PAT_16(CH_4(0,Min9,0), CH_4(1,Maj9,0), CH_4(0,Min9,0), CH_4(4,Dom7b9,0)),   // Phrygian Dominant (Im - bII - V7b9)
    PAT_16(CH_4(0,Min9,0), CH_4(3,Min9,0), CH_4(4,Dom7b9,0), CH_4(0,Min9,0)),   // Classic Bollywood Minor
    PAT_16(CH_4(5,Min9,0), CH_4(4,Dom13,0), CH_4(3,Maj9,0), CH_4(2,Dom7alt,0)), // Cinematic Descent (A.R. Rahman Style)
    PAT_16(CH_4(0,Maj9,0), CH_4(4,Maj9,0), CH_4(1,Maj9,0), CH_4(0,Maj9,0)),     // Lydian / Kalyan Feel
    PAT_16(CH_4(0,Min9,0), CH_4(0,Min9,0), CH_4(6,Maj9,0), CH_4(5,Maj9,0)),     // Epic Minor Lift
    PAT_16(CH_4(0,Min9,0), CH_4(1,Maj9,0), CH_4(3,Min9,0), CH_4(4,Dom7b9,0)),   // Desi Suspense
    PAT_16(CH_4(0,Maj9,0), CH_4(2,Maj9,0), CH_4(0,Maj9,0), CH_4(2,Maj9,0)),     // Chromatic Mediant
    PAT_16(CH_4(0,Min9,0), CH_4(1,Min9,0), CH_4(0,Min9,0), CH_4(4,Dom7alt,0)),  // Dark Item Song
    PAT_16(CH_4(3,Maj9,0), CH_4(4,Dom13,0), CH_4(0,Min9,0), CH_4(0,Min9,0)),    // Modern Pop Ascent
    PAT_16(CH_4(0,Min9,0), CH_4(3,Dom13,0), CH_4(4,Dom7b9,0), CH_4(0,Min9,0)),  // Folk Dance Vamp
    PAT_16(CH_4(0,Maj9,0), CH_4(3,Min9,0), CH_4(0,Maj9,0), CH_4(4,Dom7b9,0)),   // Major-Minor Mixture
    PAT_16(CH_4(1,Maj9,0), CH_4(1,Maj9,0), CH_4(0,Min9,0), CH_4(0,Min9,0)),     // Resolution from bII
    PAT_16(CH_4(0,Min9,0), CH_4(4,Min9,0), CH_4(0,Min9,0), CH_4(1,Maj9,0)),     // Tribal Groove
    PAT_16(CH_4(3,Maj9,0), CH_4(2,Min7,0), CH_4(1,Maj9,0), CH_4(0,Maj9,0)),     // Emotional Raga Walk
    PAT_16(CH_4(0,Min9,0), CH_4(0,Dim7,0), CH_4(1,Maj9,0), CH_4(4,Dom7b9,0))    // Tension & Release
  }}
},


// ==============================================================================
            // ★ 13. Latin / Reggaeton (16 Patterns)
            // ==============================================================================
        { 13, "13. Latin", 88, 105, 4, 4, 6, 0, true, 70, 20, 10, 40, 20, 10, 10, 100, 127, 6, 18, 0.4f,
          {1, 4, 6, 11}, 1, 4, 0.5f, 0.0f, 0.0f, // Humanize: 0
          R(100, 127), R(6, 18), R(0, 11), R(20, 80), R(10, 30),
          std::array<std::array<ChordDef, 16>, 16>{{
            PAT_16(CH_4(0,Minor,0), CH_4(3,Minor,0), CH_4(6,Major,0), CH_4(2,Major,0)), // RG01
            PAT_16(CH_4(3,Minor,0), CH_4(0,Minor,0), CH_4(5,Major,0), CH_4(4,Major,0)), // RG02
            PAT_16(CH_4(0,Minor,0), CH_4(5,Major,0), CH_4(2,Major,0), CH_4(6,Major,0)), // RG03
            PAT_16(CH_4(0,Minor,0), CH_4(5,Major,0), CH_4(6,Major,0), CH_4(4,Minor,0)), // RG04
            PAT_16(CH_4(5,Minor,0), CH_4(3,Major,0), CH_4(0,Major,0), CH_4(4,Major,0)), // RG05
            PAT_16(CH_4(0,Minor,0), CH_4(0,Minor,0), CH_4(6,Major,0), CH_4(6,Major,0)), // RG06
            PAT_16(CH_4(0,Minor,0), CH_4(5,Major,0), CH_4(0,Minor,0), CH_4(5,Major,0)), // RG07
            PAT_16(CH_4(0,Minor,0), CH_4(3,Minor,0), CH_4(4,Minor,0), CH_4(0,Minor,0)), // RG08
            PAT_16(CH_4(0,Minor,0), CH_4(1,Major,0), CH_4(0,Minor,0), CH_4(1,Major,0)), // RG09
            PAT_16(CH_4(0,Major,0), CH_4(3,Major,0), CH_4(5,Minor,0), CH_4(4,Major,0)), // RG10
            PAT_16(CH_4(0,Minor,0), CH_4(6,Major,0), CH_4(5,Major,0), CH_4(4,Dom7,0)),  // RG11
            PAT_16(CH_4(0,Minor,0), CH_4(2,Major,0), CH_4(6,Major,0), CH_4(3,Minor,0)), // RG12
            PAT_16(CH_4(5,Minor,0), CH_4(1,Minor,0), CH_4(2,Minor,0), CH_4(5,Minor,0)), // RG13
            PAT_16(CH_4(0,Minor,0), CH_4(4,Major,0), CH_4(0,Minor,0), CH_4(4,Major,0)), // RG14
            PAT_16(CH_4(5,Major,0), CH_4(6,Major,0), CH_4(0,Minor,0), CH_4(0,Minor,0)), // RG15
            PAT_16(CH_4(0,Minor,0), CH_4(0,Minor,0), CH_4(3,Minor,0), CH_4(4,Major,0))  // RG16
          }}
        },

// 14. Trance (Epic & Rolling)
            // 推奨スケール: Natural Minor, Harmonic Minor, Phrygian
        { 14, "14. Trance", 138, 138, 4, 4, 6, 0, true, 80, 20, 10, 80, 10, 40, 0, 100, 20, 60, 40,
          10, {1, 4, 6, 11}, 1, 3, 1.2f, 0.2f, 0.85f,
          R(100, 127), R(2, 6), R(10, 50), R(10, 20), R(0, 0),
          std::array<std::array<ChordDef, 16>, 16>{{
            PAT_16(CH_4(0,Minor,0), CH_4(5,Major,0), CH_4(6,Major,0), CH_4(4,Minor,0)), // 01. Epic i-VI-VII-v
            PAT_16(CH_4(5,Major,0), CH_4(3,Major,0), CH_4(0,Minor,0), CH_4(4,Major,0)), // 02. VI-IV-i-V
            PAT_16(CH_4(0,Minor,0), CH_4(0,Minor,0), CH_4(5,Major,0), CH_4(6,Major,0)), // 03. Driving 1-6-7
            PAT_16(CH_4(0,Minor,0), CH_4(2,Major,0), CH_4(5,Major,0), CH_4(6,Major,0)), // 04. Uplifting Build
            PAT_16(CH_4(0,Minor,0), CH_4(0,Minor,0), CH_4(0,Minor,0), CH_4(0,Minor,0)), // 05. Static Roll
            PAT_16(CH_4(0,Minor,0), CH_4(3,Minor,0), CH_4(5,Major,0), CH_4(6,Major,0)), // 06. Dark Progression
            PAT_16(CH_4(0,Minor,0), CH_4(1,Major,0), CH_4(0,Minor,0), CH_4(1,Major,0)), // 07. Phrygian Pulse
            PAT_16(CH_4(0,Minor,0), CH_4(4,Minor,0), CH_4(5,Major,0), CH_4(6,Major,0)), // 08. Classic Anthem
            PAT_16(CH_4(0,Minor,0), CH_4(6,Major,0), CH_4(5,Major,0), CH_4(4,Major,0)), // 09. Descending
            PAT_16(CH_4(5,Major,0), CH_4(6,Major,0), CH_4(5,Major,0), CH_4(6,Major,0)), // 10. Tension Hold
            PAT_16(CH_4(0,Minor,0), CH_4(0,Minor,0), CH_4(1,Dim,0), CH_4(1,Dim,0)),     // 11. Dark Psy Diminished
            PAT_16(CH_4(0,Minor,0), CH_4(7,Major,0), CH_4(6,Major,0), CH_4(5,Major,0)), // 12. Melodic Walk
            PAT_16(CH_4(0,Minor,0), CH_4(0,Minor,0), CH_4(0,Minor,0), CH_4(4,Dom7,0)),  // 13. Turnaround
            PAT_16(CH_4(3,Major,0), CH_4(4,Major,0), CH_4(0,Minor,0), CH_4(0,Minor,0)), // 14. Resolution
            PAT_16(CH_4(0,Minor,0), CH_4(2,Minor,0), CH_4(4,Minor,0), CH_4(6,Major,0)), // 15. Steady Climb
            PAT_16(CH_4(0,Minor,0), CH_4(0,Minor,0), CH_4(5,Major,1), CH_4(6,Major,1))  // 16. Octave Epic
          }}
        },
        // 15. Synthwave (Running Bass & Nostalgic Drive)
{ 15, "15. Synthwave", 125, 125, 4, 4, 6, 0, false, 0, 30, 40, 80, 20, 60, 50, 100, 60, 90, 50,
  20, {1, 3, 5, 11}, 1, 4, 0.5f, 0.1f, 0.6f, // Pluck感のある短いスタッカートがデフォルト
  R(100, 115), R(5, 6), R(0, 20), R(20, 75), R(0, 0),
  std::array<std::array<ChordDef, 16>, 16>{{
    PAT_16(CH_4(0,Maj9,0), CH_4(5,Min9,0), CH_4(3,Maj9,0), CH_4(4,Dom13,0)),    // 1-6-4-5 超王道 [cite: 18]
    PAT_16(CH_4(5,Min9,0), CH_4(3,Maj9,0), CH_4(0,Maj9,0), CH_4(4,Dom13,0)),    // vi-IV-I-V 哀愁 [cite: 18]
    PAT_16(CH_4(3,Maj9,0), CH_4(4,Dom13,0), CH_4(5,Min9,0), CH_4(0,Maj9,0)),    // 4-5-6-1 疾走 [cite: 18]
    PAT_16(CH_4(0,Maj9,0), CH_4(4,Dom13,0), CH_4(5,Min9,0), CH_4(3,Maj9,0)),    // I-V-vi-IV ポップ [cite: 18]
    PAT_16(CH_4(1,Min9,0), CH_4(4,Dom13,0), CH_4(0,Maj9,0), CH_4(5,Min9,0)),    // ii-V-I-vi アーバン [cite: 18]
    PAT_16(CH_4(0,Maj9,0), CH_4(3,Maj9,0), CH_4(4,Dom13,0), CH_4(3,Maj9,0)),    // I-IV-V-IV ロック的 [cite: 18]
    PAT_16(CH_4(5,Min9,0), CH_4(4,Dom13,0), CH_4(3,Maj9,0), CH_4(4,Dom13,0)),    // vi-V-IV-V 下降上昇 [cite: 18]
    PAT_16(CH_4(0,Maj9,0), CH_4(6,Maj9,0), CH_4(3,Maj9,0), CH_4(0,Maj9,0)),    // I-bVII-IV-I ミクソリディアン [cite: 18]
    PAT_16(CH_4(5,Min9,0), CH_4(1,Min9,0), CH_4(4,Dom13,0), CH_4(0,Maj9,0)),    // vi-ii-V-I 強進行 [cite: 18]
    PAT_16(CH_4(0,Maj9,0), CH_4(5,Min9,0), CH_4(1,Min9,0), CH_4(4,Dom13,0)),    // I-vi-ii-V ドゥーワップ [cite: 18]
    PAT_16(CH_4(3,Maj9,0), CH_4(0,Maj9,0), CH_4(4,Dom13,0), CH_4(5,Min9,0)),    // IV-I-V-vi エモーショナル [cite: 18]
    PAT_16(CH_4(5,Min9,0), CH_4(2,Dom13,0), CH_4(3,Maj9,0), CH_4(0,Maj9,0)),    // vi-III-IV-I クラシカル悲哀 [cite: 18]
    PAT_16(CH_4(0,Maj9,0), CH_4(4,Dom13,0), CH_4(1,Min9,0), CH_4(3,Maj9,0)),    // I-V-ii-IV 洗練 [cite: 18]
    PAT_16(CH_4(3,Maj9,0), CH_4(5,Min9,0), CH_4(4,Dom13,0), CH_4(0,Maj9,0)),    // IV-vi-V-I 視界が開ける [cite: 18]
    PAT_16(CH_4(5,Min9,0), CH_4(3,Maj9,0), CH_4(4,Dom13,0), CH_4(2,Min9,0)),    // vi-IV-V-iii リーディング [cite: 18]
    PAT_16(CH_4(0,Maj9,0), CH_4(3,Maj9,0), CH_4(5,Min9,0), CH_4(4,Dom13,0))     // I-IV-vi-V 大団円 [cite: 18]
  }}
},

            // ==============================================================================
            // Source/GenreAlgorithms.cpp (16. Funk / Disco 定義の更新)
            // ==============================================================================

                    // 16. Funk / Disco (Contour Engine Ver.)
                    // 推奨スケール: Mixolydian, Dorian, Minor Blues
        { 16, "16. Funk / Disco", 95, 95, 4, 4, 6, 0, false, 0, 75, 40, 80, 0, 40, 20, 95, 40, 70, 50,
          30, {1, 3, 6, 10}, 1, 3, 1.1f, 0.4f, 0.75f,
          R(90, 125), R(4, 12), R(60, 90), R(30, 70), R(0, 0), // Glide 0
          std::array<std::array<ChordDef, 16>, 16>{{
            PAT_16(CH_4(0,Dom7,0), CH_4(3,Maj7,0), CH_4(4,Dom7,0), CH_4(0,Dom7,0)),
            PAT_16(CH_4(0,Min7,0), CH_4(3,Min7,0), CH_4(0,Min7,0), CH_4(3,Min7,0)),
            PAT_16(CH_4(0,Dom7,0), CH_4(0,Dom7,0), CH_4(0,Dom7,0), CH_4(0,Dom7,0)),
            PAT_16(CH_4(0,Dom7,0), CH_4(5,Min7,0), CH_4(1,Min7,0), CH_4(4,Dom7,0)),
            PAT_16(CH_4(0,Min7,0), CH_4(1,Maj7,0), CH_4(0,Min7,0), CH_4(1,Maj7,0)),
            PAT_16(CH_4(3,Maj7,0), CH_4(4,Dom7,0), CH_4(0,Min7,0), CH_4(0,Min7,0)),
            PAT_16(CH_4(0,Dom7,0), CH_4(3,Maj7,0), CH_4(1,HalfDim,0), CH_4(4,Dom7,0)),
            PAT_16(CH_4(0,Dom7,0), CH_4(5,Major,0), CH_4(3,Major,0), CH_4(1,Major,0)),
            PAT_16(CH_4(0,Min7,0), CH_4(4,Min7,0), CH_4(5,Maj7,0), CH_4(4,Min7,0)),
            PAT_16(CH_4(1,Min7,0), CH_4(4,Dom7,0), CH_4(0,Maj7,0), CH_4(5,Min7,0)),
            PAT_16(CH_4(0,Dom7,0), CH_4(6,Major,0), CH_4(5,Major,0), CH_4(4,Dom7,0)),
            PAT_16(CH_4(0,Min7,1), CH_4(0,Min7,0), CH_4(4,Dom7,1), CH_4(0,Min7,0)),
            PAT_16(CH_4(0,Dom7,0), CH_4(0,Dom7,0), CH_4(0,Dom7,0), CH_4(4,Dom7,0)),
            PAT_16(CH_4(3,Major,0), CH_4(2,Minor,0), CH_4(1,Major,0), CH_4(0,Major,0)),
            PAT_16(CH_4(0,Min7,0), CH_4(0,Min7,0), CH_4(6,Dim,0), CH_4(0,Min7,0)),
            PAT_16(CH_4(5,Min7,0), CH_4(3,Maj7,0), CH_4(4,Dom7,0), CH_4(0,Dom7,0))
          }}
        },

        // ==============================================================================
        // Source/GenreAlgorithms.cpp (17. New Jack Swing 定義の更新)
        // ==============================================================================

                // 17. New Jack Swing (Snap & Digital Rhythm Ver.)
                // 推奨スケール: Dorian (Essential), Melodic Minor, Minor Blues
        { 17, "17. New Jack Swing", 110, 110, 4, 4, 6, 0, false, 0, 60, 40, 60, 15, 30, 0, 95, 40, 70, 50,
          40, {1, 3, 5, 10}, 1, 4, 1.1f, 0.45f, 0.65f, // Staccato 65% (DX7的なタイトさ)
          R(90, 125), R(4, 12), R(40, 70), R(20, 50), R(0, 0), // Glide 0
          std::array<std::array<ChordDef, 16>, 16>{{
            PAT_16(CH_4(0,Min7,0), CH_4(3,Min7,0), CH_4(4,Dom7,0), CH_4(0,Min7,0)),
            PAT_16(CH_4(0,Min7,0), CH_4(5,Maj7,0), CH_4(3,Min7,0), CH_4(4,Dom7,0)),
            PAT_16(CH_4(1,Min7,0), CH_4(4,Dom7,0), CH_4(0,Maj7,0), CH_4(5,Min7,0)),
            PAT_16(CH_4(0,Min7,0), CH_4(0,Min7,0), CH_4(1,Maj7,0), CH_4(1,Maj7,0)),
            PAT_16(CH_4(0,Min7,0), CH_4(6,Major,0), CH_4(5,Major,0), CH_4(4,Dom7,0)),
            PAT_16(CH_4(3,Maj7,0), CH_4(4,Dom7,0), CH_4(0,Min7,0), CH_4(5,Min7,0)),
            PAT_16(CH_4(0,Min7,0), CH_4(1,HalfDim,0), CH_4(4,Dom7,0), CH_4(0,Min7,0)),
            PAT_16(CH_4(0,Dom7,0), CH_4(0,Dom7,0), CH_4(0,Dom7,0), CH_4(0,Dom7,0)),
            PAT_16(CH_4(3,Min7,0), CH_4(4,Min7,0), CH_4(0,Min7,0), CH_4(0,Min7,0)),
            PAT_16(CH_4(0,Min7,0), CH_4(3,Maj7,0), CH_4(6,Maj7,0), CH_4(4,Dom7,0)),
            PAT_16(CH_4(1,Min7,0), CH_4(2,Min7,0), CH_4(3,Min7,0), CH_4(4,Min7,0)),
            PAT_16(CH_4(0,Min7,1), CH_4(5,Maj7,1), CH_4(3,Min7,0), CH_4(4,Dom7,0)),
            PAT_16(CH_4(0,Min7,0), CH_4(0,Min7,1), CH_4(0,Min7,2), CH_4(0,Min7,1)),
            PAT_16(CH_4(0,Min7,0), CH_4(4,Dom7,0), CH_4(5,Maj7,0), CH_4(3,Min7,0)),
            PAT_16(CH_4(0,Min7,0), CH_4(1,Major,0), CH_4(0,Min7,0), CH_4(4,Dom7,0)),
            PAT_16(CH_4(5,Maj7,0), CH_4(4,Dom7,0), CH_4(0,Min7,0), CH_4(0,Min7,0))
          }}
        },

        // ==============================================================================
        // Source/GenreAlgorithms.cpp (18. Neo Soul 定義の更新)
        // ==============================================================================

                // 18. Neo Soul (Soulful Space & Extension Ver.)
                // 推奨スケール: Dorian, Melodic Minor, Pentatonic Major
        { 18, "18. Neo Soul", 80, 80, 4, 4, 6, 0, false, 0, 40, 50, 40, 15, 60, 20, 80, 40, 70, 40,
          25, {1, 3, 5, 10}, 1, 4, 0.9f, 0.4f, 0.85f, // Staccato 85% (基本は長め)
          R(70, 115), R(6, 24), R(40, 80), R(20, 50), R(0, 0), // Glide 0, Humanize 0
          std::array<std::array<ChordDef, 16>, 16>{{
            PAT_16(CH_4(1,Min7,0), CH_4(4,Dom7,0), CH_4(0,Maj7,0), CH_4(5,Min7,0)),
            PAT_16(CH_4(3,Maj7,0), CH_4(2,Min7,0), CH_4(1,Min7,0), CH_4(0,Maj7,0)),
            PAT_16(CH_4(0,Maj7,0), CH_4(5,Min7,0), CH_4(3,Maj7,0), CH_4(4,Dom7,0)),
            PAT_16(CH_4(0,Min7,0), CH_4(1,Maj7,0), CH_4(2,Maj7,0), CH_4(1,Maj7,0)),
            PAT_16(CH_4(3,Maj7,0), CH_4(4,Dom7,0), CH_4(0,Min7,0), CH_4(5,Maj7,0)),
            PAT_16(CH_4(1,Min7,0), CH_4(4,Dom7,1), CH_4(0,Maj7,1), CH_4(0,Maj7,0)),
            PAT_16(CH_4(0,Min7,0), CH_4(5,Maj7,0), CH_4(1,Min7,0), CH_4(4,Dom7,0)),
            PAT_16(CH_4(5,Min7,0), CH_4(4,Dom7,0), CH_4(3,Maj7,0), CH_4(4,Dom7,0)),
            PAT_16(CH_4(0,Maj7,0), CH_4(0,Maj7,0), CH_4(3,Maj7,0), CH_4(4,Dom7,0)),
            PAT_16(CH_4(1,Min7,0), CH_4(1,Min7,0), CH_4(0,Maj7,0), CH_4(0,Maj7,0)),
            PAT_16(CH_4(0,Min7,1), CH_4(4,Dom7,1), CH_4(5,Min7,1), CH_4(1,Min7,0)),
            PAT_16(CH_4(0,Maj7,0), CH_4(6,Maj7,0), CH_4(5,Maj7,0), CH_4(4,Dom7,0)),
            PAT_16(CH_4(3,Maj7,0), CH_4(1,HalfDim,0), CH_4(4,Dom7,0), CH_4(0,Min7,0)),
            PAT_16(CH_4(0,Maj7,0), CH_4(1,Maj7,0), CH_4(3,Min7,0), CH_4(4,Dom7,0)),
            PAT_16(CH_4(1,Min7,0), CH_4(5,Min7,0), CH_4(3,Maj7,0), CH_4(4,Dom7,0)),
            PAT_16(CH_4(0,Maj7,0), CH_4(1,Maj7,1), CH_4(0,Maj7,2), CH_4(1,Maj7,1))
          }}
        },


        // 19. Boom Bap / Lo-Fi (MPC Swing & Jazzy Loops)
                // 8番目の引数（humanizeMax）を 0 に。ハネ（Swing）とヨレ（Drag）は内部演算で生成。
        { 19, "19. Boom Bap / Lo-Fi", 95, 95, 4, 4, 6, 0, true, 60, 50, 20, 60, 10, 40, 20, 90, 40, 70, 50,
          30, {1, 3, 5, 10}, 1, 4, 1.0f, 0.4f, 0.75f,
          R(90, 120), R(4, 12), R(30, 70), R(15, 35), R(10, 30),
          std::array<std::array<ChordDef, 16>, 16>{{
            PAT_16(CH_4(0,Min7,0), CH_4(3,Min7,0), CH_4(4,Dom7,0), CH_4(0,Min7,0)), // 01. Jazzy i-iv-V-i
            PAT_16(CH_4(0,Min7,0), CH_4(5,Maj7,0), CH_4(3,Min7,0), CH_4(4,Dom7,0)), // 02. Smooth 1-6-4-5
            PAT_16(CH_4(1,Min7,0), CH_4(4,Dom7,0), CH_4(0,Maj7,0), CH_4(0,Maj7,0)), // 03. 2-5-1 Standard
            PAT_16(CH_4(0,Min7,0), CH_4(1,Maj7,0), CH_4(0,Min7,0), CH_4(1,Maj7,0)), // 04. Phrygian Chill
            PAT_16(CH_4(0,Min7,0), CH_4(3,Min7,0), CH_4(1,HalfDim,0), CH_4(4,Dom7,0)), // 05. Dark Jazzy
            PAT_16(CH_4(0,Min7,0), CH_4(0,Min7,0), CH_4(0,Min7,0), CH_4(0,Min7,0)), // 06. One Chord Loop
            PAT_16(CH_4(3,Maj7,0), CH_4(4,Dom7,0), CH_4(0,Min7,0), CH_4(5,Min7,0)), // 07. 4-5-1-6
            PAT_16(CH_4(0,Min7,0), CH_4(5,Maj7,0), CH_4(1,Min7,0), CH_4(4,Dom7,0)), // 08. Deep 1-6-2-5
            PAT_16(CH_4(5,Min7,0), CH_4(4,Min7,0), CH_4(0,Min7,0), CH_4(0,Min7,0)), // 09. Soulful 6-5-1
            PAT_16(CH_4(0,Min7,0), CH_4(6,Maj7,0), CH_4(5,Maj7,0), CH_4(4,Dom7,0)), // 10. Walkdown
            PAT_16(CH_4(0,Min7,0), CH_4(3,Maj7,0), CH_4(6,Maj7,0), CH_4(4,Dom7,0)), // 11. Modal Shift
            PAT_16(CH_4(1,Min7,0), CH_4(2,Min7,0), CH_4(3,Min7,0), CH_4(4,Min7,0)), // 12. Chromatic Bridge
            PAT_16(CH_4(0,Min7,1), CH_4(4,Dom7,0), CH_4(0,Min7,0), CH_4(0,Min7,0)), // 13. Minimal V-I
            PAT_16(CH_4(0,Maj7,0), CH_4(1,Maj7,0), CH_4(3,Min7,0), CH_4(4,Dom7,0)), // 14. Sweet & Airy
            PAT_16(CH_4(3,Maj7,0), CH_4(2,Min7,0), CH_4(1,Min7,0), CH_4(0,Maj7,0)), // 15. Falling Down
            PAT_16(CH_4(0,Min7,0), CH_4(0,Min7,0), CH_4(1,Major,0), CH_4(1,Major,0))  // 16. Phrygian End
          }}
        },


        // 20. Urban Jazz (Acid Jazz / Incognito Ver.)
        { 20, "20. Urban Jazz", 95, 95, 4, 4, 6, 0, false, 0, 75, 40, 80, 0, 50, 30, 95, 40, 70, 50,
          30, {1, 3, 5, 10}, 1, 4, 1.1f, 0.45f, 0.85f,
          R(80, 127), R(4, 10), R(50, 90), R(30, 60), R(0, 0),
          std::array<std::array<ChordDef, 16>, 16>{{
                  // 1: Dorian Vamp (Im9 - IV13) -> 0=I, 3=IV
                  PAT_16(CH_4(0,Min9,0), CH_4(3,Dom13,0), CH_4(0,Min9,0), CH_4(3,Dom13,0)),
                      // 2: Neo 2-5-1 (IIm9 - V13 - IMaj9 - VI7alt) -> 1=II, 4=V, 0=I, 5=VI
                      PAT_16(CH_4(1,Min9,0), CH_4(4,Dom13,0), CH_4(0,Maj9,0), CH_4(5,Dom7alt,0)),
                      // 3: Parallel Step Down (IVMaj9 - IIIm7 - IIm9 - V7alt)
                      PAT_16(CH_4(3,Maj9,0), CH_4(2,Min7,0), CH_4(1,Min9,0), CH_4(4,Dom7alt,0)),
                      // 4: Classic Acid (Im9 - bVIIMaj9 - bVIMaj9 - V7alt) -> 6=bVII, 5=bVI
                      PAT_16(CH_4(0,Min9,0), CH_4(6,Maj9,0), CH_4(5,Maj9,0), CH_4(4,Dom7alt,0)),
                      // 5: Funky Minor Turnaround (Im9 - IIm9 - bIIIMaj9 - V7b9)
                      PAT_16(CH_4(0,Min9,0), CH_4(1,Min9,0), CH_4(2,Maj9,0), CH_4(4,Dom7b9,0)),
                      // 6: Minor Jazz (Im9 - VIm7b5 - II7alt - V7alt) -> VIm7b5 = HalfDim
                      PAT_16(CH_4(0,Min9,0), CH_4(5,HalfDim,0), CH_4(1,Dom7alt,0), CH_4(4,Dom7alt,0)),
                      // 7: Smooth R&B (IVMaj9 - III7alt - VIm9 - I13)
                      PAT_16(CH_4(3,Maj9,0), CH_4(2,Dom7alt,0), CH_4(5,Min9,0), CH_4(0,Dom13,0)),
                      // 8: Chromatic Walk (Im9 - bVII13 - bVI13 - V7alt)
                      PAT_16(CH_4(0,Min9,0), CH_4(6,Dom13,0), CH_4(5,Dom13,0), CH_4(4,Dom7alt,0)),
                      // 9: Modern Flow (IIm9 - bIIMaj9 - IMaj9 - VI7alt)
                      PAT_16(CH_4(1,Min9,0), CH_4(1,Maj9,0), CH_4(0,Maj9,0), CH_4(5,Dom7alt,0)),
                      // 10: Smooth Jazz Walk (IMaj9 - VIm9 - IIm9 - V13)
                      PAT_16(CH_4(0,Maj9,0), CH_4(5,Min9,0), CH_4(1,Min9,0), CH_4(4,Dom13,0)),
                      // 11: Incognito Vibe (Im9 - IVm9 - bVII13 - bIIIMaj9)
                      PAT_16(CH_4(0,Min9,0), CH_4(3,Min9,0), CH_4(6,Dom13,0), CH_4(2,Maj9,0)),
                      // 12: Passing Dominant (IMaj9 - #I7alt - IIm9 - V13)
                      PAT_16(CH_4(0,Maj9,0), CH_4(0,Dom7alt,0), CH_4(1,Min9,0), CH_4(4,Dom13,0)),
                      // 13: Soulful Drops (IVMaj9 - IVm9 - IIIm7 - VI7alt)
                      PAT_16(CH_4(3,Maj9,0), CH_4(3,Min9,0), CH_4(2,Min7,0), CH_4(5,Dom7alt,0)),
                      // 14: Phrygian Funk (Im9 - bIIMaj9 - Im9 - bIIMaj9)
                      PAT_16(CH_4(0,Min9,0), CH_4(1,Maj9,0), CH_4(0,Min9,0), CH_4(1,Maj9,0)),
                      // 15: Extended Turnaround (IIIm7 - VI7alt - IIm9 - V13)
                      PAT_16(CH_4(2,Min7,0), CH_4(5,Dom7alt,0), CH_4(1,Min9,0), CH_4(4,Dom13,0)),
                      // 16: Outro Fade (IVMaj9 - bVIIMaj9 - IMaj9 - I13)
                      PAT_16(CH_4(3,Maj9,0), CH_4(6,Maj9,0), CH_4(0,Maj9,0), CH_4(0,Dom13,0))
                  }}
        },

// 21. Melodic Techno (Cinematic & Driving)
            // 推奨スケール: Natural Minor, Harmonic Minor, Dorian
        { 21, "21. Melodic Techno", 126, 126, 4, 4, 6, 0, true, 80, 40, 30, 80, 10, 40, 0, 100, 20, 60, 50,
          40, {1, 4, 6, 11}, 1, 4, 1.2f, 0.25f, 0.85f,
          R(95, 127), R(3, 12), R(0, 80), R(10, 30), R(0, 0),
          std::array<std::array<ChordDef, 16>, 16>{{
            PAT_16(CH_4(0,Minor,0), CH_4(4,Minor,0), CH_4(5,Major,0), CH_4(3,Major,0)), // 01. Tale Of Us i-v-VI-IV
            PAT_16(CH_4(0,Minor,0), CH_4(2,Major,0), CH_4(4,Minor,0), CH_4(5,Major,0)), // 02. Arp Ascent
            PAT_16(CH_4(0,Minor,0), CH_4(5,Major,0), CH_4(0,Minor,0), CH_4(6,Major,0)), // 03. Dark 1-6-1-7
            PAT_16(CH_4(0,Minor,0), CH_4(1,Major,0), CH_4(0,Minor,0), CH_4(6,Major,0)), // 04. Phrygian Cinematic
            PAT_16(CH_4(0,Minor,0), CH_4(0,Minor,0), CH_4(0,Minor,0), CH_4(0,Minor,0)), // 05. Driving Moog Drone
            PAT_16(CH_4(0,Minor,0), CH_4(6,Major,0), CH_4(5,Major,0), CH_4(4,Major,0)), // 06. Descending Drama
            PAT_16(CH_4(0,Minor,0), CH_4(0,Minor,0), CH_4(4,Minor,0), CH_4(4,Minor,0)), // 07. 5th Shift
            PAT_16(CH_4(0,Minor,0), CH_4(3,Minor,0), CH_4(5,Major,0), CH_4(6,Major,0)), // 08. Emotional Build
            PAT_16(CH_4(5,Major,0), CH_4(3,Major,0), CH_4(4,Minor,0), CH_4(0,Minor,0)), // 09. Resolution
            PAT_16(CH_4(0,Minor,0), CH_4(0,Minor,1), CH_4(0,Minor,2), CH_4(0,Minor,1)), // 10. Octave Cascade
            PAT_16(CH_4(0,Minor,0), CH_4(1,Dim,0), CH_4(0,Minor,0), CH_4(4,Dom7,0)),    // 11. Tension & Turn
            PAT_16(CH_4(0,Minor,0), CH_4(5,Major,1), CH_4(6,Major,1), CH_4(4,Minor,0)), // 12. Wide Sweep
            PAT_16(CH_4(3,Major,0), CH_4(4,Minor,0), CH_4(0,Minor,0), CH_4(0,Minor,0)), // 13. Deep Resolve
            PAT_16(CH_4(0,Minor,0), CH_4(1,Major,0), CH_4(2,Major,0), CH_4(4,Minor,0)), // 14. Chromatic Up
            PAT_16(CH_4(0,Minor,0), CH_4(0,Minor,0), CH_4(3,Major,0), CH_4(4,Dom7,0)),  // 15. Groove Push
            PAT_16(CH_4(1,Major,0), CH_4(1,Major,0), CH_4(0,Minor,0), CH_4(0,Minor,0))  // 16. Phrygian Release
          }}
        },


        // ==============================================================================
        // 12. Walking Bass (Jazz / Swing / Acoustic Bass)
        // ==============================================================================
{ 22, "22. Walking Bass", 120, 120, 4, 4, 8, 0, false, 0, 30, 40, 80, 20, 60, 50, 100, 40, 80, 50,
  20, {1, 3, 5, 11}, 1, 4, 1.0f, 0.5f, 0.8f,
  R(85, 127), R(8, 24), R(30, 80), R(10, 50), R(0, 0), // ★ 7番目のパラメータ "8" が div=8 (3連符) を意味します
  std::array<std::array<ChordDef, 16>, 16>{{
    PAT_16(CH_4(1,Min9,0), CH_4(4,Dom13,0), CH_4(0,Maj9,0), CH_4(5,Dom7alt,0)), // Classic ii-V-I-VI (Autumn Leaves)
    PAT_16(CH_4(0,Dom13,0), CH_4(3,Dom13,0), CH_4(0,Dom13,0), CH_4(0,Dom7alt,0)),// Jazz Blues 1
    PAT_16(CH_4(3,Dom13,0), CH_4(4,Dim7,0), CH_4(0,Dom13,0), CH_4(5,Dom7alt,0)), // Jazz Blues 2
    PAT_16(CH_4(0,Maj9,0), CH_4(5,Dom7alt,0), CH_4(1,Min9,0), CH_4(4,Dom13,0)),  // Rhythm Changes
    PAT_16(CH_4(0,Maj9,0), CH_4(4,Dom13,0), CH_4(3,Maj9,0), CH_4(2,Dom13,0)),    // Coltrane Changes
    PAT_16(CH_4(0,Min9,0), CH_4(1,HalfDim,0), CH_4(4,Dom7alt,0), CH_4(0,Min9,0)),// Minor ii-V-i
    PAT_16(CH_4(0,Min9,0), CH_4(3,Min9,0), CH_4(4,Dom7alt,0), CH_4(0,Min9,0)),   // Minor Swing
    PAT_16(CH_4(0,Maj9,0), CH_4(0,Maj9,0), CH_4(1,Min9,0), CH_4(4,Dom13,0)),     // Take The A Train Vibe
    PAT_16(CH_4(3,Maj9,0), CH_4(6,HalfDim,0), CH_4(2,Min7,0), CH_4(5,Dom7alt,0)),// Cycle of 4ths
    PAT_16(CH_4(1,Min9,0), CH_4(1,Min9,0), CH_4(4,Dom13,0), CH_4(4,Dom13,0)),    // Dorian Modal
    PAT_16(CH_4(0,Maj9,0), CH_4(3,Maj9,0), CH_4(1,Min9,0), CH_4(4,Dom13,0)),     // Standard Ballad
    PAT_16(CH_4(0,Dom13,0), CH_4(5,Dom13,0), CH_4(1,Dom13,0), CH_4(4,Dom13,0)),  // All Dominant Cycle
    PAT_16(CH_4(0,Min9,0), CH_4(0,Dim7,0), CH_4(1,Min9,0), CH_4(4,Dom7b9,0)),    // Passing Diminished
    PAT_16(CH_4(0,Maj9,0), CH_4(1,Dom13,0), CH_4(0,Maj9,0), CH_4(1,Dom13,0)),    // Lydian Dominant
    PAT_16(CH_4(2,Min7,0), CH_4(5,Dom7alt,0), CH_4(1,Min9,0), CH_4(4,Dom13,0)),  // iii-VI-ii-V
    PAT_16(CH_4(1,Min9,0), CH_4(4,Dom13,0), CH_4(1,Min9,0), CH_4(4,Dom13,0))     // ii-V Vamp
  }}
},
// 23. Electronic Generic (Universal Electronic Lab)
{ 23, "23. Electronic Generic", 110, 110, 4, 4, 6, 0, false, 0, 30, 40, 80, 20, 60, 50, 100, 50, 80, 50,
  20, {1, 3, 5, 11}, 1, 4, 0.8f, 0.2f, 0.5f,
  R(90, 120), R(6, 24), R(20, 90), R(10, 60), R(0, 0),
  std::array<std::array<ChordDef, 16>, 16>{{
    PAT_16(CH_4(0,Maj9,0), CH_4(4,Dom13,0), CH_4(5,Min9,0), CH_4(3,Maj9,0)),    // I-V-vi-IV (Pop Anthem)
    PAT_16(CH_4(5,Min9,0), CH_4(3,Maj9,0), CH_4(0,Maj9,0), CH_4(4,Dom13,0)),    // vi-IV-I-V (Standard EDM)
    PAT_16(CH_4(0,Maj9,0), CH_4(3,Maj9,0), CH_4(5,Min9,0), CH_4(4,Dom13,0)),    // I-IV-vi-V (Epic)
    PAT_16(CH_4(5,Min9,0), CH_4(4,Dom13,0), CH_4(3,Maj9,0), CH_4(2,Min9,0)),    // vi-V-IV-iii (Dark Descent)
    PAT_16(CH_4(1,Min9,0), CH_4(4,Dom13,0), CH_4(0,Maj9,0), CH_4(5,Min9,0)),    // ii-V-I-vi (Cycle)
    PAT_16(CH_4(0,Maj9,0), CH_4(6,Maj9,0), CH_4(3,Maj9,0), CH_4(0,Maj9,0)),    // I-bVII-IV-I (Mixolydian)
    PAT_16(CH_4(5,Min9,0), CH_4(1,Min9,0), CH_4(4,Dom13,0), CH_4(0,Maj9,0)),    // vi-ii-V-I
    PAT_16(CH_4(0,Maj9,0), CH_4(3,Maj9,0), CH_4(4,Dom13,0), CH_4(3,Maj9,0)),    // I-IV-V-IV
    PAT_16(CH_4(0,Min9,0), CH_4(4,Min9,0), CH_4(5,Maj9,0), CH_4(6,Maj9,0)),    // i-v-VI-VII (Trance Drive)
    PAT_16(CH_4(0,Maj9,0), CH_4(5,Min9,0), CH_4(3,Maj9,0), CH_4(4,Dom13,0)),    // I-vi-IV-V (50s electronic)
    PAT_16(CH_4(3,Maj9,0), CH_4(4,Dom13,0), CH_4(5,Min9,0), CH_4(0,Maj9,0)),    // IV-V-vi-I
    PAT_16(CH_4(0,Min9,0), CH_4(2,Maj9,0), CH_4(3,Maj9,0), CH_4(5,Maj9,0)),    // i-bIII-IV-bVI (Bluesy)
    PAT_16(CH_4(0,Min9,0), CH_4(6,Maj9,0), CH_4(5,Maj9,0), CH_4(4,Dom7alt,0)),  // Andalusian (Phrygian)
    PAT_16(CH_4(0,Maj9,0), CH_4(4,Dom13,0), CH_4(1,Min9,0), CH_4(3,Maj9,0)),    // I-V-ii-IV
    PAT_16(CH_4(5,Min9,0), CH_4(0,Maj9,0), CH_4(4,Dom13,0), CH_4(3,Maj9,0)),    // vi-I-V-IV
    PAT_16(CH_4(0,Min9,0), CH_4(5,Maj9,0), CH_4(2,Maj9,0), CH_4(6,Maj9,0))     // i-VI-III-VII (Epic Future)
  }}
},
} // ← genreRegistry 全体の初期化リストを閉じる
}; // ← std::array 変数の宣言を終了