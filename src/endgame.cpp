#include "endgame.h"
#include <fathom/tbprobe.h>  // From Stockfish (or your custom implementation)
#include <filesystem>
#include <unordered_set>

// Special score constants
constexpr int TB_WIN_IN_MAX_PLY  = 30000 - MAX_PLY;
constexpr int TB_LOSS_IN_MAX_PLY = -TB_WIN_IN_MAX_PLY;

EndgameTablebase::EndgameTablebase() : 
    max_pieces(5), 
    cache_size(64) {}

EndgameTablebase::~EndgameTablebase() {
    free_tb();  // Properly unload tablebases
}

bool EndgameTablebase::load(const std::vector<std::string>& paths) {
    free_tb();
    
    for (const auto& path : paths) {
        if (!std::filesystem::exists(path)) {
            std::cerr << "TB path not found: " << path << std::endl;
            continue;
        }

        // Initialize Fathom (Stockfish's TB probing)
        if (tb_init(path.c_str()) == TB_NO_MISSING) {
            loaded_paths.insert(path);
            std::cout << "Loaded TBs from: " << path << std::endl;
        }
    }

    if (loaded_paths.empty()) {
        std::cerr << "No tablebases loaded!" << std::endl;
        return false;
    }

    // Configure probing behavior
    tb_set_max_pieces(max_pieces);
    tb_set_cache_size(cache_size);
    return true;
}

EndgameTablebase::ProbeResult EndgameTablebase::probe(const Board& pos) const {
    ProbeResult result = { WDLResult::Draw, 0, Move::none() };
    if (!is_loaded() || pos.piece_count() > max_pieces) 
        return result;

    // Convert to Fathom's expected format
    uint64_t white = pos.pieces[WHITE][ALL_PIECES];
    uint64_t black = pos.pieces[BLACK][ALL_PIECES];
    uint64_t kings = pos.pieces[WHITE][KING] | pos.pieces[BLACK][KING];
    uint64_t queens = pos.pieces[WHITE][QUEEN] | pos.pieces[BLACK][QUEEN];
    // ... similarly for other pieces

    unsigned tb_result;
    uint16_t move = tb_probe_root(
        white, black, kings, queens, /*rooks*/, /*bishops*/, /*knights*/, /*pawns*/,
        pos.sideToMove == WHITE,
        &tb_result
    );

    // Convert Fathom result to our format
    switch (tb_result) {
        case TB_WIN:  result.wdl = WDLResult::Win; break;
        case TB_LOSS: result.wdl = WDLResult::Loss; break;
        case TB_DRAW: result.wdl = WDLResult::Draw; break;
        case TB_CURSED_WIN:  result.wdl = WDLResult::CursedWin; break;
        case TB_BLESSED_LOSS: result.wdl = WDLResult::BlessedLoss; break;
    }

    if (move != TB_NOMOVE) {
        result.best_move = syzygy_to_move(move, pos);
        result.dtz = tb_probe_dtz(pos);  // Distance to zeroing move
    }

    return result;
}

bool EndgameTablebase::probe_best_move(const Board& pos, Move& bestMove) const {
    auto result = probe(pos);
    if (result.best_move.is_ok()) {
        bestMove = result.best_move;
        return true;
    }
    return false;
}

int EndgameTablebase::probe_score(const Board& pos) const {
    auto result = probe(pos);
    switch (result.wdl) {
        case WDLResult::Win:  return TB_WIN_IN_MAX_PLY - result.dtz;
        case WDLResult::Loss: return TB_LOSS_IN_MAX_PLY + result.dtz;
        default: return 0;
    }
}

// --- Private Helpers ---

void EndgameTablebase::free_tb() {
    if (is_loaded()) {
        tb_free();
        loaded_paths.clear();
    }
}

Move syzygy_to_move(uint16_t syzygy_move, const Board& pos) {
    int to = syzygy_move & 0x3F;
    int from = (syzygy_move >> 6) & 0x3F;
    
    // Handle promotions
    PieceType promo = NONE;
    switch ((syzygy_move >> 12) & 0x7) {
        case 1: promo = QUEEN; break;
        case 2: promo = ROOK; break;
        case 3: promo = BISHOP; break;
        case 4: promo = KNIGHT; break;
    }

    return Move(from, to, promo);
}