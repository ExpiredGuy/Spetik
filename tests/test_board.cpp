#include <iostream>
#include "../src/board.h"
#include "../src/move.h"

int main() {
    Board b;
    b.set_from_fen("rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1");

    std::cout << "=== Test: Board FEN Load ===" << std::endl;
    b.print();

    if (b.sideToMove != WHITE) {
        std::cerr << "Error: Side to move should be WHITE." << std::endl;
        return 1;
    }

    Move m = Move(12, 28); // e2 -> e4
    b.make_move(m);

    if (b.pieces[WHITE][PAWN] & (1ULL << 28)) {
        std::cout << "Move generation: PASS" << std::endl;
    } else {
        std::cerr << "Move generation: FAIL" << std::endl;
        return 1;
    }

    std::cout << "Board test passed." << std::endl;
    return 0;
}
