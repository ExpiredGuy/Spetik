#include <iostream>
#include "../src/board.h"
#include "../src/evaluation.h"

int main() {
    Board b;
    Evaluation eval;

    std::cout << "=== Test: Evaluation Consistency ===" << std::endl;
    b.set_from_fen("8/8/8/8/8/8/8/8 w - - 0 1");

    int score1 = eval.evaluate(b);
    int score2 = eval.evaluate(b);

    if (score1 != score2) {
        std::cerr << "Eval inconsistency detected!" << std::endl;
        return 1;
    }

    b.set_from_fen("rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1");
    int openingScore = eval.evaluate(b);
    std::cout << "Opening position score: " << openingScore << std::endl;

    std::cout << "Eval test passed." << std::endl;
    return 0;
}
