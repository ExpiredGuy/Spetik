#pragma once

#include "types.h"
#include "board.h"

namespace Material {

    typedef int Score;

    struct MaterialInfo {
        Score midgame;   // Midgame material score
        Score endgame;   // Endgame material score
        int phase;       // Game phase (0..24)
    };

    // Evaluate material balance
    MaterialInfo evaluate(const Board& board);

    // Get base piece value in midgame/endgame
    Score piece_value(PieceType pt, Phase phase);

    // Compute game phase
    int game_phase(const Board& board);

} // namespace Material
