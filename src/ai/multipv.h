#pragma once
#include "types.h"
#include "move.h"
#include <vector>
#include <algorithm>

namespace MultiPV {
    // Single PV line with score and moves
    struct PVLine {
        int score;
        std::vector<Move> moves;
        
        bool operator<(const PVLine& other) const {
            return score > other.score; // Sort descending
        }
    };

    // Configuration
    void set_max_pv(int max);
    int get_max_pv();
    void set_show_all(bool show);
    
    // Management
    void clear();
    void new_pv(int score, const std::vector<Move>& moves);
    
    // Access
    const std::vector<PVLine>& get_pvs();
    PVLine get_best_pv();
    
    // Utility
    void sort_pvs();
    bool is_main_pv(const Move& move);
}