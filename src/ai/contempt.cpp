#include "contempt.h"
#include <atomic>
#include <algorithm>

namespace {
    // Thread-safe storage with atomic operations
    std::atomic<int> contempt_value{24}; // Default avoids draws slightly
    
    // Clamp contempt to reasonable range
    constexpr int MIN_CONTEMPT = -100;
    constexpr int MAX_CONTEMPT = 100;
}

namespace AI {
    void init_contempt(int default_contempt) {
        set_contempt(default_contempt);
    }
    
    void set_contempt(int value) {
        contempt_value.store(
            std::clamp(value, MIN_CONTEMPT, MAX_CONTEMPT),
            std::memory_order_relaxed
        );
    }
    
    int get_contempt() {
        return contempt_value.load(std::memory_order_relaxed);
    }
    
    int apply_contempt(int score, bool we_are_winning) {
        const int contempt = get_contempt();
        
        // Only adjust near draws (|score| < 2 pawns)
        if (abs(score) < 200) {
            // Winning side avoids draws, losing side seeks them
            return score + (we_are_winning ? contempt : -contempt);
        }
        return score;
    }
}