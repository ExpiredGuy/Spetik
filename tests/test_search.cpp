#include <iostream>
#include "../src/board.h"
#include "../src/search.h"
#include "../src/move.h"

int main() {
    Board b;
    Search search;

    std::cout << "=== Test: Search Function ===" << std::endl;
    b.set_from_fen("rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1");

    SearchResult result = search.iterative_deepening(b, 3);
    std::cout << "Best move: " << result.bestMove.to_string() 
              << " | Score: " << result.score << std::endl;

    if (!result.bestMove.is_ok()) {
        std::cerr << "Search failed to return a move." << std::endl;
        return 1;
    }

    std::cout << "Search test passed." << std::endl;
    return 0;
}
