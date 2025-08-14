#include "quiescence.h"
#include "eval.h"
#include "moveorder.h"
#include "see.h"
#include <algorithm>

// Configuration constants (tunable)
constexpr int FutilityMargin = 100;  // Centipawns
constexpr int SeeMargin = -50;       // SEE threshold for bad captures

QuiescenceSearch::QuiescenceSearch() : 
    nodes(0),
    qDepth(0),
    moveOrder(std::make_unique<MoveOrder>()) {}

int QuiescenceSearch::search(Board& board, int alpha, int beta, bool inCheck) {
    nodes++;
    
    // Check recursion limit
    if (qDepth++ >= MaxQDepth) {
        qDepth--;
        return static_eval(board, inCheck);
    }

    // Stand pat evaluation
    int standPat = static_eval(board, inCheck);
    
    // Beta cutoff
    if (standPat >= beta)
        return beta;
    
    // Alpha update
    if (standPat > alpha)
        alpha = standPat;

    // Generate moves (captures + checks if not in check)
    std::vector<Move> moves;
    if (inCheck) {
        board.generate_evasions(moves);
    } else {
        board.generate_captures(moves);
        if (alpha < beta - FutilityMargin) {
            board.generate_checks(moves);
        }
    }

    // Move ordering
    moveOrder->order_moves(board, moves, Move::none(), qDepth);

    // Search moves
    for (const Move& move : moves) {
        // Skip illegal moves (already filtered in generation)
        if (!board.is_legal(move))
            continue;

        // SEE pruning for bad captures
        if (!inCheck && board.is_capture(move) && 
            see(board, move) < SeeMargin) {
            continue;
        }

        board.make_move(move);
        int score = -search(board, -beta, -alpha, board.in_check());
        board.unmake_move(move);

        if (score >= beta) {
            // Update history heuristics
            if (!board.is_capture(move))
                moveOrder->update_history(move, 1, qDepth);
            return beta;
        }
        
        if (score > alpha) {
            alpha = score;
        }
    }

    qDepth--;
    return alpha;
}

int QuiescenceSearch::static_eval(const Board& board, bool inCheck) const {
    // Use main evaluation with small optimizations
    int eval = Eval::evaluate(board);
    
    // Small bonus for side to move in qsearch
    if (!inCheck)
        eval += (board.side_to_move() == WHITE) ? 10 : -10;
    
    return eval;
}

// Optional optimizations you can add:

// 1. Delta pruning
bool QuiescenceSearch::delta_pruning(const Board& board, int beta, int margin) const {
    return static_eval(board, false) + margin <= beta;
}

// 2. Recursive SEE
int QuiescenceSearch::see(Board& board, Move move, int threshold) const {
    return SEE::see(board, move, threshold);
}

// 3. Advanced move ordering
void QuiescenceSearch::order_moves(std::vector<Move>& moves, const Board& board) {
    std::sort(moves.begin(), moves.end(), [&](const Move& a, const Move& b) {
        // MVV-LVA for captures
        if (board.is_capture(a) && board.is_capture(b)) {
            return board.see(a) > board.see(b);
        }
        // Checks first
        return board.gives_check(a) && !board.gives_check(b);
    });
}