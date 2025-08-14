#include "material.h"

// Piece values from a tuned evaluation (example Stockfish-inspired)
static constexpr Material::Score PieceValueMidgame[PIECE_TYPE_NB] = {
    0,   // NONE
    100, // PAWN
    320, // KNIGHT
    330, // BISHOP
    500, // ROOK
    900, // QUEEN
    0    // KING (handled separately)
};

static constexpr Material::Score PieceValueEndgame[PIECE_TYPE_NB] = {
    0,   // NONE
    120, // PAWN
    310, // KNIGHT
    330, // BISHOP
    500, // ROOK
    900, // QUEEN
    0
};

// Phase weights for each piece type (used for midgame/endgame interpolation)
static constexpr int PhaseWeight[PIECE_TYPE_NB] = {
    0, // NONE
    0, // PAWN
    1, // KNIGHT
    1, // BISHOP
    2, // ROOK
    4, // QUEEN
    0  // KING
};

namespace Material {

    MaterialInfo evaluate(const Board& board) {
        MaterialInfo info{};
        info.midgame = 0;
        info.endgame = 0;
        info.phase = 0;

        for (Color c = WHITE; c <= BLACK; ++c) {
            for (PieceType pt = PAWN; pt <= QUEEN; ++pt) {
                int count = popcount(board.pieces(c, pt));
                if (!count) continue;

                Score mgVal = PieceValueMidgame[pt] * count;
                Score egVal = PieceValueEndgame[pt] * count;

                info.midgame += (c == WHITE ? mgVal : -mgVal);
                info.endgame += (c == WHITE ? egVal : -egVal);

                info.phase += PhaseWeight[pt] * count;
            }
        }

        // Phase is capped at maximum (24 = starting position)
        if (info.phase > 24) info.phase = 24;

        return info;
    }

    Score piece_value(PieceType pt, Phase phase) {
        return (phase == Phase::MIDGAME) ? PieceValueMidgame[pt] : PieceValueEndgame[pt];
    }

    int game_phase(const Board& board) {
        int phase = 0;
        for (Color c = WHITE; c <= BLACK; ++c)
            for (PieceType pt = PAWN; pt <= QUEEN; ++pt)
                phase += PhaseWeight[pt] * popcount(board.pieces(c, pt));

        return std::min(phase, 24);
    }

} // namespace Material
