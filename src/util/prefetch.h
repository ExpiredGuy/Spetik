#pragma once

#include <cstddef>
#include <immintrin.h>  // For AVX/SSE intrinsics

namespace ChessEngine {

// Prefetch hints tuned for chess engine patterns
enum class PrefetchType {
    TT_READ,       // Transposition Table read (high locality)
    TT_WRITE,      // Transposition Table write
    PAWN_READ,     // Pawn hash table access
    EVAL_CACHE,    // Evaluation cache line
    KILLER_MOVES,  // Killer move slots
    HISTORY        // History heuristic tables
};

// Main prefetch function (constexpr compatible)
template <PrefetchType P = PrefetchType::TT_READ>
inline void prefetch(const void* addr) noexcept {
    constexpr int hints[] = {
        _MM_HINT_T0,   // TT_READ: Strong temporal locality
        _MM_HINT_T1,   // TT_WRITE: Moderate temporal
        _MM_HINT_T2,   // PAWN_READ: Light temporal
        _MM_HINT_NTA,  // EVAL_CACHE: Non-temporal
        _MM_HINT_T0,   // KILLER_MOVES
        _MM_HINT_T0    // HISTORY
    };
    _mm_prefetch(static_cast<const char*>(addr), hints[static_cast<int>(P)]);
}

// Aggressive multi-line prefetch for critical sections
inline void prefetch_tt_cluster(const void* addr) noexcept {
    const char* base = static_cast<const char*>(addr);
    _mm_prefetch(base + 0 * 64, _MM_HINT_T0);  // Primary entry
    _mm_prefetch(base + 1 * 64, _MM_HINT_T1);  // Secondary entry (for bucket TT)
}

} // namespace ChessEngine