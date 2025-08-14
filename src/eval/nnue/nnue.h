#pragma once
#include <string>
#include "board.h"  // Your existing board API

namespace NNUE {

struct Config {
    int inputSize  = 768;  // 12*64 one-hot (simple feature set)
    int l1Size     = 256;
    int l2Size     = 32;
    int outputSize = 1;
};

bool load(const std::string& path);       // Load weights from .nnue file
bool save(const std::string& path);       // Save the currently loaded weights
bool initialized();                       // True if a network is in memory

// Run evaluation (centipawns from White's perspective)
int evaluate(const Board& pos);

// Optional: initialize with random weights (useful for testing)
void init_random(unsigned seed = 42, const Config& cfg = Config());

// Optional: override default config (must be called before load/init_random)
void set_config(const Config& cfg);

} // namespace NNUE
