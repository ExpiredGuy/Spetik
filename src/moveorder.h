#pragma once
#include <vector>
#include <array>
#include "move.h"
#include "board.h"
#include "types.h"  // For Value, Piece types

class MoveOrder {
public:
    // Initialize with current position and search state
    void init(const Board& board, const std::vector<Move>& moves, 
              Move pvMove = Move::none(), int ply = 0, 
              const Move* countermove = nullptr);

    // Get next move in order (returns false when done)
    bool next(Move& outMove);

    // Reset iterator without re-scoring
    void reset() { current = ordered.begin(); }

    // Update history heuristics after a good move is found
    void update_history(Move move, int depth, int ply);

    // Clear all history tables (between games)
    static void clear_history();

private:
    // Move scoring categories (adjust weights as needed)
    enum Score {
        SCORE_PV        = 20000,
        SCORE_CAPTURE   = 15000,
        SCORE_PROMO     = 10000,
        SCORE_CASTLE    = 5000,
        SCORE_KILLER1   = 9000,
        SCORE_KILLER2   = 8000,
        SCORE_COUNTER   = 7000,
        SCORE_GOOD_SEE  = 6000,
        SCORE_HISTORY   = 4000,
        SCORE_BAD_CAPTURE = -1000
    };

    // Internal move entry with score
    struct ScoredMove {
        Move move;
        int score;
        bool operator<(const ScoredMove& other) const { return score > other.score; }
    };

    std::vector<ScoredMove> ordered;
    std::vector<ScoredMove>::iterator current;

    // History heuristics
    static constexpr int MAX_SQUARES = 64;
    static int history[2][MAX_SQUARES][MAX_SQUARES];  // [color][from][to]
    static int counterMoves[16][MAX_SQUARES];         // [piece][to_square]
    
    // Killer moves (indexed by ply)
    static constexpr int MAX_KILLERS = 2;
    static std::vector<std::array<Move, MAX_KILLERS>> killers;

    // Current search state
    const Board* pos;
    Move pvMove;
    Move countermove;
    int currentPly;

    // Scoring functions
    int score_move(Move move) const;
    int score_capture(Move move) const;
    int score_quiet(Move move) const;
    int see_sign(Move move) const;
};