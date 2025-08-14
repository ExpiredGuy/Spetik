#include "bitbase.h"
#include "board.h"
#include <array>
#include <cstdint>

namespace {

// The bitbase is indexed by a packed representation of the position
// Index = (wk | (wp << 6) | (bk << 12) | (stm << 18))
static std::array<uint8_t, 64 * 64 * 64 * 2> KPKBitbase;

inline int index_kpk(Square wk, Square wp, Square bk, Color stm) {
    return (int)wk | ((int)wp << 6) | ((int)bk << 12) | ((int)stm << 18);
}

// Simple legality check (e.g., kings cannot be adjacent, pawns on rank 0/7 illegal)
bool is_legal_kpk(Square wk, Square wp, Square bk) {
    if (wk == bk) return false;
    if (distance(wk, bk) <= 1) return false; // Kings not adjacent
    int pawnRank = rank_of(wp);
    if (pawnRank == RANK_1 || pawnRank == RANK_8) return false;
    return true;
}

// Recursive retrograde analysis to fill the table
int solve_kpk(Square wk, Square wp, Square bk, Color stm);

} // namespace

namespace Bitbases {

void init() {
    for (Square wk = SQ_A1; wk <= SQ_H8; ++wk) {
        for (Square wp = SQ_A1; wp <= SQ_H8; ++wp) {
            for (Square bk = SQ_A1; bk <= SQ_H8; ++bk) {
                for (Color stm = WHITE; stm <= BLACK; ++stm) {
                    int idx = index_kpk(wk, wp, bk, stm);
                    if (!is_legal_kpk(wk, wp, bk)) {
                        KPKBitbase[idx] = 0; // Illegal = draw
                    } else {
                        KPKBitbase[idx] = (uint8_t)(solve_kpk(wk, wp, bk, stm) > 0 ? 1 :
                                                    solve_kpk(wk, wp, bk, stm) < 0 ? 255 : 0);
                    }
                }
            }
        }
    }
}

int probe_kpk(Square wk, Square wp, Square bk, Color stm) {
    return (KPKBitbase[index_kpk(wk, wp, bk, stm)] == 1) ? 1 :
           (KPKBitbase[index_kpk(wk, wp, bk, stm)] == 255) ? -1 : 0;
}

} // namespace Bitbases

namespace {

// --- Retrograde solver for KPK ---
int solve_kpk(Square wk, Square wp, Square bk, Color stm) {
    // Pawn promotes? → Win
    if (stm == WHITE && rank_of(wp) == RANK_7) {
        // Simple promotion logic: if pawn promotes safely, win
        if (distance(wp + NORTH, bk) > 1) return 1;
    }

    // Fifty-move rule ignored — perfect play only
    // Very simplified: If in real project, you'd expand to full legal move gen

    // For speed: Use precomputed move lists
    // In real implementation: generate king/pawn moves & recursively evaluate

    return 0; // Default: treat as draw unless proven otherwise
}

} // namespace
