#include "pruning.h"

// Constructor with default parameters, can be tuned
Pruning::Pruning() : R(2), null_move_reduction(2) {}

// Null move pruning: returns true to prune subtree if conditions met
bool Pruning::null_move_pruning(const Board& board, int depth, int beta) {
    // Don't prune in PV nodes or shallow depths
    if (depth < 3) return false;

    // Don't prune if in check or insufficient material
    if (board.in_check(board.sideToMove)) return false;

    // TODO: Add some heuristics based on static eval if desired

    return true;  // signal to perform null move search (reduced depth)
}

// Late move pruning (LMP): prune non-capture late moves in non-PV nodes
bool Pruning::late_move_pruning(int depth, int moveNumber, bool inPV) {
    if (inPV) return false;  // don't prune PV moves

    if (depth <= 3) return false;  // only prune at sufficient depth

    // Prune after trying first few moves
    if (moveNumber >= 4 + (depth / 4)) {
        return true;
    }
    return false;
}

// Futility pruning: prune if static eval + margin < beta near leaf nodes
bool Pruning::futility_pruning(const Board& board, int depth, int staticEval, int beta) {
    if (depth > 5) return false;  // only prune near leaves

    int margin = futility_margin[depth];
    if (staticEval + margin < beta) {
        return true;
    }
    return false;
}

// Static pruning for quiescence and static eval cutoff
bool Pruning::static_pruning(int staticEval, int alpha, int beta) {
    if (staticEval >= beta) return true;   // fail-hard beta cutoff
    if (staticEval <= alpha) return true;  // fail-soft alpha cutoff
    return false;
}
