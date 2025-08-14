#pragma once
#include <cstdint>
#include <string>
#include <vector>
#include <unordered_set>
#include <memory>
#include <algorithm>
#include "move.h"
#include "board.h"  // For Board or Position class

// Special score constants (adjust to your engine's scale)
constexpr int TB_WIN   = 30000;  // Winning position mate score
constexpr int TB_LOSS  = -TB_WIN;
constexpr int TB_DRAW  = 0;

class EndgameTablebase {
public:
    EndgameTablebase();
    ~EndgameTablebase();

    // --- Core Interface ---

    // Load tablebases from one or multiple directories
    // Returns true if at least one path loaded successfully
    bool load(const std::vector<std::string>& paths);

    // Structure for probing result with WDL/DTZ and best move
    enum class WDLResult : uint8_t { Loss = 0, BlessedLoss, Draw, CursedWin, Win };
    struct ProbeResult {
        WDLResult wdl;
        int dtz;          // Distance to zeroing move (50-move rule reset)
        Move best_move;   // Best move according to TB (if available)
    };

    // Probe the position, returns ProbeResult with WDL/DTZ/best move
    ProbeResult probe(const Board& pos) const;

    // --- Advanced Features ---

    // Preload specific material subsets to speed probing
    void preload_material(const std::string& material);

    // Set max pieces to probe (default 5; range 3-7)
    void set_max_pieces(int max_pieces_) { max_pieces = std::clamp(max_pieces_, 3, 7); }

    // Cache management
    void clear_cache();
    void set_cache_size(size_t megabytes);

    // --- Diagnostics & Status ---

    bool is_loaded() const { return !loaded_paths_.empty(); }
    const std::unordered_set<std::string>& loaded_paths() const { return loaded_paths_; }

private:
    struct Impl;
    std::unique_ptr<Impl> impl_;

    int max_pieces = 5;            // Pieces threshold for probing
    size_t cache_size_mb = 64;     // Cache size in MB
    std::unordered_set<std::string> loaded_paths_;
};

// --- Helper Utilities ---

// Return a material signature string for TB identification (e.g. "KQvK")
std::string get_material_signature(const Board& pos);

// Syzygy-style move encoder/decoder (for integration)
Move syzygy_to_move(uint16_t syzygy_move, const Board& pos);
uint16_t move_to_syzygy(const Move& move, const Board& pos);
