#include "king.h"
#include "bitboard.h"
#include "tuner.h"
#include <array>

namespace {

// Safety table [king square][safety score]
std::array<std::array<int, 64>, 2> SafetyTable;

// Attack distance penalties
constexpr int DistancePenalty[8] = {
    0, 0, 10, 20, 30, 40, 50, 60
};

// Tuned weights
KingSafety::Weights weights;

// Initialize safety tables
void init_safety_table() {
    for (Square s = SQ_A1; s <= SQ_H8; ++s) {
        // Middle-game safety
        SafetyTable[0][s] = 
            -5 * popcount(Attack::king_zone(s, WHITE)) +
            -3 * relative_rank(WHITE, s);

        // End-game safety
        SafetyTable[1][s] = 
            -2 * popcount(Attack::king_zone(s, WHITE));
    }
}

} // namespace

namespace KingSafety {

void init() {
    init_safety_table();
    
    // Register tunable parameters
    Tuner::Tuner.add_parameter({"QueenAttackWeight", &weights.queen, 3, 8});
    Tuner::Tuner.add_parameter({"RookAttackWeight", &weights.rook, 2, 5});
    // ... other weights
}

Score evaluate(const Board& board, Color kingColor) {
    const Square kingSquare = board.king_square(kingColor);
    const Color enemy = ~kingColor;
    
    // 1. King shelter (pawn structure around king)
    Score shelter = evaluate_shelter(board, kingColor);
    
    // 2. Pawn storms (enemy pawns attacking king zone)
    Score storm = evaluate_storm(board, kingColor);
    
    // 3. Piece attacks
    Score attacks = evaluate_attacks(board, kingColor);
    
    // 4. Weak squares around king
    Score weak = evaluate_weak_squares(board, kingColor);
    
    // Combine scores with safety table lookup
    Score safety = SafetyTable[0][kingSquare] + shelter + storm + attacks + weak;
    
    // Scale by game phase (less important in endgame)
    int phase = board.game_phase();
    return (safety * phase) / 24;
}

Score evaluate_shelter(const Board& board, Color kingColor) {
    const Square kingSquare = board.king_square(kingColor);
    Bitboard shelterZone = Attack::king_zone(kingSquare) & ~board.pieces(PAWN);
    
    int holes = popcount(shelterZone);
    int pawnCover = popcount(Attack::pawn_attacks(kingColor, kingSquare) & 
                            board.pieces(kingColor, PAWN));
    
    return -20 * holes + 15 * pawnCover;
}

Score evaluate_attacks(const Board& board, Color kingColor) {
    const Square kingSquare = board.king_square(kingColor);
    const Color enemy = ~kingColor;
    
    int attackScore = 0;
    Bitboard attackers = board.attackers_to(kingSquare, enemy);
    
    while (attackers) {
        Square s = pop_lsb(&attackers);
        PieceType pt = board.piece_on(s).type();
        
        switch (pt) {
            case QUEEN:  attackScore += weights.queen; break;
            case ROOK:   attackScore += weights.rook; break;
            case BISHOP: attackScore += weights.bishop; break;
            case KNIGHT: attackScore += weights.knight; break;
            case PAWN:   attackScore += weights.pawn; break;
            default: break;
        }
        
        // Add distance penalty
        int distance = distance_between(s, kingSquare);
        attackScore += DistancePenalty[distance];
    }
    
    // Double attacker bonus
    if (popcount(attackers) > 1) {
        attackScore *= 2;
    }
    
    return attackScore;
}

bool is_in_danger(Score safetyScore) {
    return safetyScore > 150;
}

bool is_mate_threat(Score safetyScore) {
    return safetyScore > 500;
}

} // namespace KingSafety