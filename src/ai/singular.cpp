#include "singular.h"
#include "pruning.h"

namespace {
    // Margin for considering a move singular (in centipawns)
    constexpr int SINGULAR_MARGIN = 50;
    
    // Minimum depth to attempt singular extensions
    constexpr int MIN_SINGULAR_DEPTH = 8;
}

namespace Singular {

bool is_singular(const Board& board, Move move, int depth, int alpha, int beta, int ttValue) {
    // Only check at sufficient depth
    if (depth < MIN_SINGULAR_DEPTH)
        return false;

    // Don't extend in PV nodes
    if (alpha != beta - 1)
        return false;

    // Only consider moves that appear potentially good
    if (ttValue < beta - SINGULAR_MARGIN)
        return false;

    // Setup reduced search window
    const int reducedBeta = ttValue - SINGULAR_MARGIN;
    if (reducedBeta <= -MATE_SCORE)
        return false;

    // Verify move is indeed singular by searching alternatives
    int bestScore = -MATE_SCORE;
    MoveList moves;
    generate_moves(board, moves);

    for (const Move& m : moves) {
        if (m == move) continue;

        Board b = board;
        if (!b.make_move(m)) continue;

        int score = -search::alphaBeta<NonPV>(b, depth / 2, -reducedBeta, -reducedBeta + 1);

        if (score >= reducedBeta) {
            return false; // Found another good move
        }
        bestScore = std::max(bestScore, score);
    }

    // If we get here, the move appears singular
    return true;
}

int extension_depth(const Board& board, Move move, int depth, int ttValue) {
    // Base extension formula
    int extension = 1;
    
    // Increase extension for deeper searches
    if (depth >= 12) {
        extension += 1;
    }
    
    // Reduce extension if move is from TT
    if (move == TT.get_best_move(board.key())) {
        extension -= 1;
    }
    
    return std::clamp(extension, 0, 2);
}

bool skip_move(const Board& board, Move move, int depth, int beta) {
    // Skip verification if not at sufficient depth
    if (depth < MIN_SINGULAR_DEPTH * 2)
        return false;

    // Only skip in non-PV nodes
    if (beta != alpha + 1)
        return false;

    // Check if this move failed high in previous verification
    TTEntry* entry = TT.probe(board.key());
    if (entry && entry->move == move && entry->bound == BOUND_LOWER) {
        return entry->score >= beta;
    }
    
    return false;
}

} // namespace Singular