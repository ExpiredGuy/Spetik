#ifndef SEARCH_H
#define SEARCH_H

#include "position.h"
#include <move.h>
#include "moveorder.h"
#include "pruning.h"
#include "quiescence.h"
#include <vector>
#include <atomic>
#include <chrono>

// Search parameters (depth, time control, nodes, etc.)
struct SearchLimits {
    int depth;                  // Maximum search depth
    int movetime;               // Fixed time per move (ms)
    int time[2];                // Remaining time for both sides
    int inc[2];                 // Increment per move
    int movesToGo;              // Moves to next time control
    bool infinite;              // Search until stopped
};

// Search results (best move, score, PV line)
struct SearchResult {
    Move bestMove;
    int score;                  // in centipawns
    std::vector<Move> pv;       // Principal variation
};

// Main search class
class Search {
public:
    Search();

    void startSearch(Position& pos, const SearchLimits& limits);
    void stopSearch();

    SearchResult getResult() const;

private:
    // Core alpha-beta search
    int alphaBeta(Position& pos, int depth, int alpha, int beta, bool isRoot);
    int quiescenceSearch(Position& pos, int alpha, int beta);

    // Time management
    bool checkStop();
    void initTime(const SearchLimits& limits);

    // Search state
    std::atomic<bool> stopFlag;
    std::chrono::steady_clock::time_point startTime;
    int timeLimitMs;

    SearchResult result;
};

#endif // SEARCH_H
