#pragma once
#include "board.h"
#include "move.h"
#include "search.h"
#include "tt.h"

namespace Singular {

// Check if a move is singular (the only good move in position)
bool is_singular(const Board& board, Move move, int depth, int alpha, int beta, int ttValue);

// Calculate extension depth for singular moves
int extension_depth(const Board& board, Move move, int depth, int ttValue);

// Should we skip this move due to singular extension search results?
bool skip_move(const Board& board, Move move, int depth, int beta);
}