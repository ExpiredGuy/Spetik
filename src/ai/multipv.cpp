#include "multipv.h"
#include <algorithm>

namespace {
    // Configuration
    int max_pv = 1; // Default to single PV
    bool show_all = false;
    
    // Storage
    std::vector<PVLine> pv_lines;
}

namespace MultiPV {
    void set_max_pv(int max) {
        max_pv = std::clamp(max, 1, 10); // Limit to 10 PVs max
    }
    
    int get_max_pv() {
        return max_pv;
    }
    
    void set_show_all(bool show) {
        show_all = show;
    }
    
    void clear() {
        pv_lines.clear();
    }
    
    void new_pv(int score, const std::vector<Move>& moves) {
        // Don't store duplicates
        for (const auto& line : pv_lines) {
            if (!line.moves.empty() && !moves.empty() && 
                line.moves[0] == moves[0]) {
                return;
            }
        }
        
        // Add new PV if within limit or showing all
        if (pv_lines.size() < static_cast<size_t>(max_pv) || show_all) {
            pv_lines.push_back({score, moves});
        }
        
        // Replace worst PV if better than existing
        else {
            auto worst = std::min_element(pv_lines.begin(), pv_lines.end());
            if (score > worst->score) {
                *worst = {score, moves};
            }
        }
    }
    
    const std::vector<PVLine>& get_pvs() {
        sort_pvs();
        return pv_lines;
    }
    
    PVLine get_best_pv() {
        if (pv_lines.empty()) return {0, {}};
        sort_pvs();
        return pv_lines[0];
    }
    
    void sort_pvs() {
        std::sort(pv_lines.begin(), pv_lines.end());
    }
    
    bool is_main_pv(const Move& move) {
        if (pv_lines.empty()) return false;
        return !pv_lines[0].moves.empty() && pv_lines[0].moves[0] == move;
    }
}