#pragma once
#include "types.h"
#include "board.h"
#include "attack.h"

namespace KingSafety {

// Safety score type
enum Score : int {
    SAFE = 0,
    DANGER = 1000,
    MATE_THREAT = 32000
};

// Attack weights by piece type
struct Weights {
    int queen = 5;
    int rook = 3;
    int bishop = 2;
    int knight = 2;
    int pawn = 1;
};

// Initialize safety tables
void init();

// Evaluate king safety for given color
Score evaluate(const Board& board, Color kingColor);

// Detailed evaluation components
Score evaluate_shelter(const Board& board, Color kingColor);
Score evaluate_storm(const Board& board, Color kingColor);
Score evaluate_attacks(const Board& board, Color kingColor);
Score evaluate_weak_squares(const Board& board, Color kingColor);

// Attack pattern detection
int count_attackers(const Board& board, Square kingSquare, Color attackerColor);
int attack_weight(const Board& board, Square kingSquare, Color attackerColor);

// Safety thresholds
bool is_in_danger(Score safetyScore);
bool is_mate_threat(Score safetyScore);

} // namespace KingSafety