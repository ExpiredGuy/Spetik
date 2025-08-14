#include <iostream>
#include <filesystem>
#include "nnue.h"
#include "board.h"

int main(int argc, char** argv) {
    std::string out = "data/nnue/nnue_001.nnue";
    if (argc >= 2) out = argv[1];

    // Ensure directory exists
    try {
        std::filesystem::create_directories(std::filesystem::path(out).parent_path());
    } catch (...) {}

    // Configure (you can tweak sizes here — keep in sync with engine)
    NNUE::Config cfg;
    cfg.inputSize  = 12 * 64; // one-hot simple features
    cfg.l1Size     = 256;
    cfg.l2Size     = 32;
    cfg.outputSize = 1;

    NNUE::set_config(cfg);
    NNUE::init_random(42, cfg);        // replace later with your trainer
    if (!NNUE::save(out)) return 1;

    // Optional: sanity check load
    if (!NNUE::load(out)) return 2;

    // Optional: quick eval to prove it runs
    Board b;
    b.set_from_fen("rn1qkbnr/ppp1pppp/8/3p4/3P4/5N2/PPP1PPPP/RNBQKB1R w KQkq - 1 3");
    int cp = NNUE::evaluate(b);
    std::cout << "Eval(sample) = " << cp << " cp\n";
    return 0;
}
