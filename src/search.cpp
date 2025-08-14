#include "search.h"
#include "movegen.h"
#include "evaluation.h"
#include "transposition.h"
#include "pruning.h"
#include "quiescence.h"
#include "timer.h"
#include "history.h"
#include <algorithm>
#include <iostream>
#include <vector>

namespace Search {

// Configuration constants
constexpr int MAX_PLY = 64;
constexpr int ASPIRATION_WINDOW = 50;
constexpr int NULL_MOVE_R = 2; // Reduction for null move

// Search state
SearchLimits limits;
SearchResult result;
std::atomic<bool> stopFlag;
HistoryStats history;
CounterMoveStats counterMoves;
KillerMoves killers[MAX_PLY];

void think(Position& pos, const SearchLimits& lim) {
    // Initialize search
    limits = lim;
    result = SearchResult();
    stopFlag = false;
    Timer::reset();
    nodes = 0;

    // Aspiration window search
    int bestScore = 0;
    int alpha = -INFINITE;
    int beta = INFINITE;
    int depth = 1;

    // Iterative deepening loop
    while (depth <= limits.maxDepth && !stopFlag) {
        // Adjust window after first iteration
        if (depth >= 5) {
            alpha = std::max(-INFINITE, bestScore - ASPIRATION_WINDOW);
            beta = std::min(INFINITE, bestScore + ASPIRATION_WINDOW);
        }

        // Search with current depth
        int score = alphaBeta<Root>(pos, depth, alpha, beta);

        // Handle aspiration window failure
        if (score <= alpha || score >= beta) {
            alpha = -INFINITE;
            beta = INFINITE;
            continue;
        }

        // Update best score and PV
        bestScore = score;
        updatePV(depth);

        // Send info to GUI
        sendInfo(depth, bestScore, Timer::elapsed());

        // Check time and depth limits
        if (Timer::isTimeUp() || depth == MAX_PLY) {
            stopFlag = true;
        }

        depth++;
    }

    result.bestMove = rootMoves[0].move;
}

template <NodeType node>
int alphaBeta(Position& pos, int depth, int alpha, int beta, bool cutNode) {
    // Check time and depth limits
    if ((nodes & 1023) == 0 && checkTime()) {
        stopFlag = true;
    }
    nodes++;

    // Leaf nodes
    if (depth <= 0) {
        return quiescence<node>(pos, alpha, beta);
    }

    // Draw detection
    if (pos.isDraw()) {
        return 0;
    }

    // TT lookup
    TTEntry tt;
    bool ttHit = TT.probe(pos.key(), tt);
    if (ttHit && !node == Root && tt.depth >= depth) {
        if (tt.bound == BOUND_EXACT)
            return tt.score;
        if (tt.bound == BOUND_LOWER)
            alpha = std::max(alpha, tt.score);
        else if (tt.bound == BOUND_UPPER)
            beta = std::min(beta, tt.score);
        if (alpha >= beta)
            return tt.score;
    }

    // Null move pruning
    if (node != PV && depth >= 3 && !pos.inCheck() && !ttHit) {
        pos.doNullMove();
        int reduction = depth >= 6 ? 4 : 3;
        int score = -alphaBeta<NonPV>(pos, depth - 1 - reduction, -beta, -beta + 1);
        pos.undoNullMove();
        if (score >= beta)
            return score;
    }

    // Generate and order moves
    MoveList moves;
    MovePicker mp(pos, moves, history, counterMoves, killers, depth, tt.move);
    mp.scoreMoves();

    // Search variables
    int bestScore = -INFINITE;
    Move bestMove = MOVE_NONE;
    int legalMoves = 0;
    bool raisedAlpha = false;

    // Main move loop
    for (int i = 0; i < moves.size(); i++) {
        Move move = mp.nextMove();
        if (!pos.makeMove(move)) {
            continue;
        }

        legalMoves++;
        int score;

        // Late move reduction
        if (legalMoves >= 4 && depth >= 3 && !pos.inCheck() && !move.isCapture()) {
            int reduction = log(legalMoves) / log(2) - 1;
            score = -alphaBeta<NonPV>(pos, depth - 1 - reduction, -alpha - 1, -alpha);
        } else {
            score = -alphaBeta<NonPV>(pos, depth - 1, -alpha - 1, -alpha);
        }

        // Full search if LMR failed high
        if (score > alpha && (node == PV || score < beta)) {
            score = -alphaBeta<node>(pos, depth - 1, -beta, -alpha);
        }

        pos.undoMove(move);

        if (score > bestScore) {
            bestScore = score;
            bestMove = move;

            if (score > alpha) {
                alpha = score;
                raisedAlpha = true;

                if (node == PV) {
                    updatePV(move);
                }

                if (score >= beta) {
                    // Update history and killers
                    if (!move.isCapture()) {
                        history.update(pos.pieceOn(move.from()), move.to(), depth);
                        killers[depth].update(move);
                    }
                    break;
                }
            }
        }
    }

    // TT store
    Bound bound = raisedAlpha ? BOUND_LOWER : BOUND_UPPER;
    if (alpha >= beta) bound = BOUND_LOWER;
    TT.store(pos.key(), bestScore, depth, bound, bestMove);

    return bestScore;
}

} // namespace Search