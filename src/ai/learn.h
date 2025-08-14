#pragma once
#include "board.h"
#include "types.h"
#include <vector>
#include <unordered_map>
#include <filesystem>

namespace Learn {
    // Learning entry containing game results and moves
    struct Entry {
        uint64_t key;
        Move move;
        int16_t score;
        uint16_t count;
        uint8_t result; // 0=loss, 1=draw, 2=win
    };

    // Initialize learning system
    void init(const std::string& filename = "learn.bin");

    // Save learning data to disk
    void save();

    // Add new position to learning database
    void add_position(const Board& board, Move move, int result);

    // Adjust evaluation based on learned data
    int adjust_eval(const Board& board, Move move, int currentEval);

    // Get recommended move from learned data
    Move suggest_move(const Board& board);
}