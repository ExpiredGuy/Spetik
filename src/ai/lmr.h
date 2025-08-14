#pragma once
#include "types.h"

namespace LMR {
    // Initialize LMR table (call once at startup)
    void init();
    
    // Get reduction for given depth and move number
    inline int reduction(int depth, int move_number, bool improving) {
        // Base reduction from table
        int red = LMRTable[std::min(depth, MAX_DEPTH-1)][std::min(move_number, MAX_MOVES-1)];
        
        // Smaller reductions when position is improving
        if (improving) {
            red -= 1;
        }
        
        // Never reduce more than current depth
        return std::max(0, std::min(red, depth - 1));
    }
}

// LMR lookup table [depth][move number]
extern int LMRTable[MAX_DEPTH][MAX_MOVES];