#pragma once
#include "types.h"
#include "board.h"
#include "bitboard.h"

namespace Pawn {

// Pawn evaluation score type
enum Score : int {
    DOUBLED_PENALTY = -10,
    ISOLATED_PENALTY = -20,
    BACKWARD_PENALTY = -15,
    PASSED_BONUS = 50,
    CANDIDATE_BONUS = 30,
    SHIELD_BONUS = 15
};

// Detailed pawn structure information
struct PawnInfo {
    Score score;
    Bitboard passed_pawns[COLOR_NB];
    Bitboard pawn_attacks[COLOR_NB];
    Bitboard weak_pawns[COLOR_NB];
    Bitboard candidate_pawns[COLOR_NB];
    
    // Evaluation components
    Score king_safety[COLOR_NB];
    Score mobility[COLOR_NB];
    Score threats[COLOR_NB];
};

// Initialize pawn hash and tables
void init();

// Evaluate pawn structure (cached)
const PawnInfo& evaluate(const Board& board);

// Detailed evaluation components
Score evaluate_passed_pawns(const Board& board, Color color);
Score evaluate_king_shield(const Board& board, Color color);
Score evaluate_weak_pawns(const Board& board, Color color);
Score evaluate_candidates(const Board& board, Color color);

// Utility functions
Bitboard get_passed_pawn_mask(Color color, Square pawn_sq);
bool is_doubled(const Board& board, Color color, Square pawn_sq);
bool is_isolated(const Board& board, Color color, Square pawn_sq);
bool is_backward(const Board& board, Color color, Square pawn_sq);

} // namespace Pawn