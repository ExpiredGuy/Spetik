#pragma once
#include "board.h"
#include "move.h"
#include "evaluation.h"  // For static evaluation
#include "moveorder.h"  // For capture ordering

class QuiescenceSearch {
public:
    // Configuration parameters (tunable)
    struct Config {
        int deltaPruningMargin = 200;  // Queen value + margin
        int maxDepth = 64;             // Absolute safety limit
        bool useSee = true;            // SEE pruning for bad captures
        bool useDelta = true;          // Delta pruning
        bool checkExtensions = true;   // Extend checks
    };

    QuiescenceSearch(const Config& config = {}) : config(config) {}

    // Main search interface
    template <NodeType node>
    int search(Board& board, int alpha, int beta, int depth = 0);

    // Statistics tracking
    struct Stats {
        uint64_t nodes = 0;
        uint64_t pruned = 0;
        uint64_t reductions = 0;
    };
    const Stats& get_stats() const { return stats; }
    void clear_stats() { stats = {}; }

private:
    // Internal helpers
    int search_captures(Board& board, int alpha, int beta, int depth);
    int search_checks(Board& board, int alpha, int beta, int depth);
    
    // Generation specialized for qsearch
    template <GenType>
    void generate_qmoves(Board& board, std::vector<Move>& moves);

    // Advanced pruning
    bool delta_pruning(const Board& board, int beta, int margin) const;
    bool see_pruning(Move move, int threshold) const;

    // Move ordering
    void order_captures(Board& board, std::vector<Move>& moves);
    void order_checks(Board& board, std::vector<Move>& moves);

    // Configuration
    Config config;
    
    // Search state
    MoveOrder moveOrder;
    Stats stats;

    // Thread-local data (if parallel)
    struct ThreadData {
        std::vector<Move> captures;
        std::vector<Move> checks;
    };
    ThreadData threadData;
};