#pragma once
#include <cstdint>
#include <vector>
#include "board.h"  // Assumes Board class with Piece, Color, Square

namespace Bitbases {

// Bitbase for KPK (King + Pawn vs King)
class KPKBitbase {
public:
    static void init();  // Precompute the bitbase
    static bool probe(Color strongSide, Square wKing, Square wPawn, Square bKing);

private:
    static uint64_t bitbase[2][24][64][64]; // [strongSide][pawnFile][wKing][bKing]
    static int index(Square sq);
    static void compute();
};

// Add more bitbases (KRK, KBNK, etc.) as needed
} // namespace Bitbases