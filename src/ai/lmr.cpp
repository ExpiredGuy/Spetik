#include "lmr.h"
#include <cmath>

// LMR table [depth][move number]
int LMRTable[MAX_DEPTH][MAX_MOVES];

namespace LMR {
    void init() {
        // Initialize reduction values based on depth and move number
        for (int depth = 0; depth < MAX_DEPTH; ++depth) {
            for (int move_num = 0; move_num < MAX_MOVES; ++move_num) {
                // Base reduction formula
                double reduction = 0.5 + log(depth) * log(move_num) / 2.5;
                
                // Adjust based on empirical testing:
                // - Increase reductions at higher depths
                // - More aggressive reductions for later moves
                if (depth >= 6) {
                    reduction *= 1.15;
                }
                if (move_num >= 8) {
                    reduction *= 1.1;
                }
                
                LMRTable[depth][move_num] = static_cast<int>(reduction);
            }
        }
        
        // Clamp values for extreme cases
        for (int depth = 0; depth < 3; ++depth) {
            for (int move_num = 0; move_num < MAX_MOVES; ++move_num) {
                LMRTable[depth][move_num] = 0; // No reduction in shallow depths
            }
        }
    }
}