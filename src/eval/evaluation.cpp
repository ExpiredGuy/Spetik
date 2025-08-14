#include "evaluation.h"
#include "material.h"
#include "pawn.h"
#include "mobility.h"
#include "king_safety.h"
#include "nnue/nnue.h"
#include <algorithm>

namespace Eval {

// Aligned piece-square tables
const PST pst = {
    // Middle-game PST (values in centipawns)
    { /* WHITE */
        {}, // EMPTY
        { /* PAWN */
             0,   0,   0,   0,   0,   0,   0,   0,
             5,  10,  10, -20, -20,  10,  10,   5,
             5,  -5, -10,   0,   0, -10,  -5,   5,
             0,   0,   0,  20,  20,   0,   0,   0,
             5,   5,  10,  25,  25,  10,   5,   5,
            10,  10,  20,  30,  30,  20,  10,  10,
            50,  50,  50,  50,  50,  50,  50,  50,
             0,   0,   0,   0,   0,   0,   0,   0
        },
        { /* KNIGHT */ /* ... */ },
        { /* BISHOP */ /* ... */ },
        { /* ROOK */   /* ... */ },
        { /* QUEEN */  /* ... */ },
        { /* KING */   /* ... */ }
    },
    // Black tables would mirror white tables
    { /* BLACK */ /* ... */ }
};

// Initialize evaluation components
void init() {
    Material::init();
    Pawn::init();
    Mobility::init();
    KingSafety::init();
    
    // Optional NNUE initialization
    #ifdef USE_NNUE
    NNUE::init("nnue.bin");
    #endif
}

// Detailed game phase calculation
int game_phase(const Board& board) {
    // Piece weights for phase calculation
    static constexpr int PhaseWeights[PIECE_TYPE_NB] = {
        0, 0, 1, 1, 2, 4, 0
    };

    int phase = 0;
    for (PieceType pt = KNIGHT; pt <= QUEEN; ++pt) {
        phase += board.count(WHITE, pt) * PhaseWeights[pt];
        phase += board.count(BLACK, pt) * PhaseWeights[pt];
    }
    return std::clamp(phase, 0, PHASE_SCALE);
}

Score evaluate(const Board& board, EvalInfo* info) {
    // Initialize eval info if not provided
    EvalInfo localInfo;
    if (!info) info = &localInfo;

    // Calculate game phase
    const int phase = game_phase(board);
    const int nnueWeight = NNUE::is_enabled() ? phase / 6 : 0; // More NNUE in endgame

    // Material evaluation
    Score mg = Material::evaluate_mg(board);
    Score eg = Material::evaluate_eg(board);
    
    // Piece evaluation (including mobility)
    mg += evaluate_pieces(board, *info, MG);
    eg += evaluate_pieces(board, *info, EG);

    // Pawn structure evaluation
    mg += weights.pawn_structure.mg_value * Pawn::evaluate(board, *info, MG);
    eg += weights.pawn_structure.eg_value * Pawn::evaluate(board, *info, EG);

    // King safety (only in middlegame)
    if (phase > PHASE_SCALE / 4) {
        mg += weights.king_safety.mg_value * KingSafety::evaluate(board, *info);
    }

    // Threats and space
    mg += weights.threats.mg_value * evaluate_threats(board, *info, MG);
    eg += weights.threats.eg_value * evaluate_threats(board, *info, EG);
    
    // Blend middle-game and end-game scores
    Score classical = blend(mg, eg, phase);

    // Optional NNUE blending
    #ifdef USE_NNUE
    if (NNUE::is_enabled()) {
        Score nnue = NNUE::evaluate(board);
        // Dynamic weighting based on phase
        classical = (classical * (24 - nnueWeight) + nnue * nnueWeight) / 24;
    }
    #endif

    // Add tempo bonus
    classical += (board.side_to_move() == WHITE) ? 10 : -10;

    return board.side_to_move() == WHITE ? classical : -classical;
}

Score evaluate_pieces(const Board& board, EvalInfo& info, Phase ph) {
    Score score = SCORE_ZERO;
    
    // Evaluate each piece type
    for (PieceType pt = KNIGHT; pt <= QUEEN; ++pt) {
        Bitboard pieces = board.pieces(pt);
        while (pieces) {
            Square s = pop_lsb(&pieces);
            score += pst(board.color_on(s), pt, s, ph);
            
            // Mobility calculation
            if (pt != QUEEN || ph == MG) { // Queen mobility only in MG
                int mob = Mobility::count(board, s, pt);
                score += weights.mobility[pt].value(ph) * mob;
                info.mobility[board.color_on(s)][pt] += mob;
            }
        }
    }
    
    return score;
}

} // namespace Eval