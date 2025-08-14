#pragma once

namespace AI {
    // Initialize contempt with default value (recommended: 24 for human play)
    void init_contempt(int default_contempt = 24);
    
    // Set contempt value (-100 to 100, where positive avoids draws)
    void set_contempt(int value);
    
    // Get current contempt value
    int get_contempt();
    
    // Get contempt-adjusted score (call during evaluation)
    int apply_contempt(int score, bool we_are_winning);
}