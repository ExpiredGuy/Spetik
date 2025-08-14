#pragma once
#include "move.h"
#include "board.h"

class Pruning {
public:
    Pruning();

    // Null move pruning: returns true if search should prune
    bool null_move_pruning(const Board& board, int depth, int beta);

    // Late move pruning: returns true if move can be pruned at given move count
    bool late_move_pruning(int depth, int moveNumber, bool inPV);

    // Futility pruning: returns true if static eval plus margin fails low cutoff
    bool futility_pruning(const Board& board, int depth, int staticEval, int beta);

    // Verify static eval cutoff (quiescence & static pruning)
    bool static_pruning(int staticEval, int alpha, int beta);

private:
    // Parameters/tuning constants
    int R;  // Reduction for LMP
    int null_move_reduction;

    // Futility margins per depth (example tuning)
    static constexpr int futility_margin[16] = {
        0, 0, 100, 200, 300, 400, 500, 600, 700, 800, 900, 1000, 1100, 1200, 1300, 1400
    };
};