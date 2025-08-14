#include "pawn.h"
#include "tuner.h"
#include "magic.h"
#include <array>

namespace {

// Pawn hash table (size 2^16 entries)
constexpr size_t PAWN_HASH_SIZE = 65536;
std::array<Pawn::PawnInfo, PAWN_HASH_SIZE> pawnHashTable;

// Tuned evaluation weights
struct {
    Tuner::TuneParam passed_pawn = {"PassedPawn", 50, 30, 80};
    Tuner::TuneParam candidate = {"CandidatePawn", 30, 15, 50};
    Tuner::TuneParam doubled = {"DoubledPawn", -10, -20, -5};
    Tuner::TuneParam isolated = {"IsolatedPawn", -20, -30, -10};
    Tuner::TuneParam backward = {"BackwardPawn", -15, -25, -5};
} weights;

// Initialize pawn-related bitboards
void init_pawn_masks() {
    // Initialization of attack tables etc.
    // ... (typically handled by magic bitboards)
}

} // namespace

namespace Pawn {

void init() {
    init_pawn_masks();
    
    // Register tunable parameters
    Tuner::Tuner.add_parameter(weights.passed_pawn);
    Tuner::Tuner.add_parameter(weights.candidate);
    // ... other parameters
}

const PawnInfo& evaluate(const Board& board) {
    // Probe pawn hash table
    uint64_t key = board.pawn_key() % PAWN_HASH_SIZE;
    PawnInfo& pi = pawnHashTable[key];
    
    // Reuse if already computed
    if (pi.score != 0 || board.pawns() == 0)
        return pi;
    
    // Reset info
    pi = PawnInfo();
    
    // Evaluate for both colors
    for (Color c : {WHITE, BLACK}) {
        Bitboard pawns = board.pieces(c, PAWN);
        
        // Pawn attacks
        pi.pawn_attacks[c] = pawn_attacks_bb(c, pawns);
        
        // Passed pawns
        pi.passed_pawns[c] = 0;
        while (pawns) {
            Square s = pop_lsb(&pawns);
            
            // Passed pawn detection
            if ((passed_pawn_mask(c, s) & board.pieces(~c, PAWN)) == 0) {
                pi.passed_pawns[c] |= square_bb(s);
                pi.score += (c == WHITE ? 1 : -1) * evaluate_passed_pawn(board, s);
            }
            
            // Weak pawn detection
            if (is_isolated(board, c, s) || is_backward(board, c, s)) {
                pi.weak_pawns[c] |= square_bb(s);
                pi.score += (c == WHITE ? 1 : -1) * weights.isolated.value();
            }
            
            // Doubled pawns
            if (is_doubled(board, c, s)) {
                pi.score += (c == WHITE ? 1 : -1) * weights.doubled.value();
            }
        }
        
        // King safety
        pi.king_safety[c] = evaluate_king_shield(board, c);
        
        // Candidate pawns
        pi.candidate_pawns[c] = find_candidate_pawns(board, c);
    }
    
    return pi;
}

Score evaluate_passed_pawn(const Board& board, Square pawn_sq) {
    Color c = color_of(board.piece_on(pawn_sq));
    int rank = relative_rank(c, pawn_sq);
    
    // Bonus increases with advancement
    static constexpr int PassedRankBonus[8] = {
        0, 5, 10, 20, 35, 60, 100, 0
    };
    
    Score bonus = weights.passed_pawn.value() + PassedRankBonus[rank];
    
    // Add bonus if supported by own pawns
    if (board.pieces(c, PAWN) & adjacent_files_bb(file_of(pawn_sq)))
        bonus += bonus / 2;
    
    return bonus;
}

Score evaluate_king_shield(const Board& board, Color c) {
    Square king_sq = board.king_square(c);
    Score shield = 0;
    
    // Only evaluate in middlegame
    if (board.non_pawn_material() < 6000) {
        Bitboard shield_bb = pawn_attacks_bb(c, board.pieces(c, PAWN)) & 
                           king_zone_bb(king_sq);
        shield = popcount(shield_bb) * weights.shield;
    }
    
    return shield;
}

Bitboard find_candidate_pawns(const Board& board, Color c) {
    Bitboard candidates = 0;
    Bitboard pawns = board.pieces(c, PAWN);
    
    while (pawns) {
        Square s = pop_lsb(&pawns);
        if (is_candidate_pawn(board, c, s))
            candidates |= square_bb(s);
    }
    
    return candidates;
}

bool is_candidate_pawn(const Board& board, Color c, Square s) {
    // A pawn is candidate if:
    // 1. Not blocked by enemy pawns
    // 2. Has potential to become passed
    // 3. Supported by friendly pawns
    
    Bitboard forward = forward_bb(c, s);
    return !(forward & board.pieces(~c, PAWN)) &&
           (adjacent_files_bb(s) & board.pieces(c, PAWN));
}

} // namespace Pawn