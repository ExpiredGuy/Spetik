#include "moveorder.h"
#include <algorithm>

// Initialize static members
int MoveOrder::history[2][64][64] = {};
int MoveOrder::counterMoves[16][64] = {};
std::vector<std::array<Move, 2>> MoveOrder::killers;

void MoveOrder::init(const Board& board, const std::vector<Move>& moves, 
                    Move pvMove, int ply, const Move* countermove) {
    pos = &board;
    this->pvMove = pvMove;
    currentPly = ply;
    if (countermove) this->countermove = *countermove;
    
    // Ensure killers vector is large enough
    if (killers.size() <= static_cast<size_t>(ply)) {
        killers.resize(ply + 1);
    }

    // Score and order moves
    ordered.clear();
    ordered.reserve(moves.size());
    
    for (Move m : moves) {
        ordered.push_back({m, score_move(m)});
    }

    // Partial sort - first 5 moves are fully ordered
    if (ordered.size() > 5) {
        std::partial_sort(ordered.begin(), ordered.begin() + 5, 
                         ordered.end());
    } else {
        std::sort(ordered.begin(), ordered.end());
    }

    reset();
}

bool MoveOrder::next(Move& outMove) {
    if (current == ordered.end()) return false;
    outMove = current->move;
    ++current;
    return true;
}

int MoveOrder::score_move(Move move) const {
    if (move == pvMove) return SCORE_PV;
    if (pos->is_capture(move)) return score_capture(move);
    return score_quiet(move);
}

int MoveOrder::score_capture(Move move) const {
    // MVV-LVA (Most Valuable Victim - Least Valuable Attacker)
    int victim = pos->piece_on(move.to());
    int attacker = pos->piece_on(move.from());
    int score = SCORE_CAPTURE + 10 * victim - attacker;

    // SEE pruning
    if (see_sign(move) < 0) {
        return SCORE_BAD_CAPTURE + score;  // Bad captures go last
    }
    return score;
}

int MoveOrder::score_quiet(Move move) const {
    int score = 0;
    
    // Killer moves
    for (int i = 0; i < MAX_KILLERS; ++i) {
        if (move == killers[currentPly][i]) {
            score += (i == 0) ? SCORE_KILLER1 : SCORE_KILLER2;
        }
    }

    // Countermove heuristic
    if (move == countermove) {
        score += SCORE_COUNTER;
    }

    // History heuristic
    Color c = pos->side_to_move();
    score += history[c][move.from()][move.to()] / 16;  // Scale down history

    return score;
}

void MoveOrder::update_history(Move move, int depth, int ply) {
    Color c = pos->side_to_move();
    int bonus = std::min(16 * depth * depth, 1200);
    history[c][move.from()][move.to()] += bonus;

    // Decay old history to prevent overflow
    if (history[c][move.from()][move.to()] > 20000) {
        for (auto& h : history[c]) {
            for (auto& v : h) v /= 2;
        }
    }

    // Update killer moves
    if (killers[ply][0] != move) {
        killers[ply][1] = killers[ply][0];
        killers[ply][0] = move;
    }
}