#pragma once
#include "types.h"
#include "board.h"
#include "tuner.h"  // For parameter tuning integration

namespace Eval {

// Score type with explicit units
enum Score : int32_t {
    SCORE_ZERO      = 0,
    SCORE_MATE      = 32000,
    SCORE_MATE_IN_MAX = SCORE_MATE - MAX_PLY,
    SCORE_INFINITE  = 32001,
    SCORE_UNKNOWN   = 32002
};

// Evaluation phases
enum Phase {
    PHASE_MIDGAME,
    PHASE_ENDGAME,
    PHASE_SCALE = 24  // Granularity for phase interpolation
};

// Piece-Square Tables (aligned for SIMD access)
struct alignas(64) PST {
    Score mg[COLOR_NB][PIECE_TYPE_NB][SQUARE_NB];
    Score eg[COLOR_NB][PIECE_TYPE_NB][SQUARE_NB];
    
    Score operator()(Color c, PieceType pt, Square s, Phase ph) const {
        return blend(mg[c][pt][s], eg[c][pt][s], ph);
    }
};

extern const PST pst;

// Evaluation terms (tunable parameters)
struct Weights {
    Tuner::TuneParam mobility[PIECE_TYPE_NB];
    Tuner::TuneParam pawn_structure;
    Tuner::TuneParam king_safety;
    Tuner::TuneParam threats;
    Tuner::TuneParam passed_pawns;
    Tuner::TuneParam space;
    
    // Initialize with default values and ranges
    Weights() {
        mobility[KNIGHT] = {"Mobility Knight", 80, 50, 120};
        mobility[BISHOP] = {"Mobility Bishop", 90, 60, 130};
        // ... other initializations
    }
};

extern Weights weights;

// Evaluation features
struct EvalInfo {
    int mobility[COLOR_NB][PIECE_TYPE_NB];
    int king_safety[COLOR_NB];
    int pawn_structure[COLOR_NB];
    // ... other features
};

// Initialization
void init();

// Main evaluation function
Score evaluate(const Board& board, EvalInfo* info = nullptr);

// Detailed evaluation components
Score evaluate_pieces(const Board& board, EvalInfo& info);
Score evaluate_pawns(const Board& board, EvalInfo& info);
Score evaluate_king(const Board& board, EvalInfo& info);
Score evaluate_threats(const Board& board, EvalInfo& info);

// Phase calculation
int game_phase(const Board& board);

// Score blending with phase
inline Score blend(Score mg, Score eg, int phase) {
    return static_cast<Score>((mg * phase + eg * (PHASE_SCALE - phase)) / PHASE_SCALE);
}

// Tuning interface
void register_tunable_parameters(Tuner::ParameterTuner& tuner);

} // namespace Eval