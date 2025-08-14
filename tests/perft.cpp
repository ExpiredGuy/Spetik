#include <iostream>
#include <string>
#include <vector>
#include <chrono>
#include <iomanip>
#include "../src/board.h"
#include "../src/movegen.h"
#include "../src/move.h"

using namespace std;
using namespace chrono;

// Global counters for detailed statistics
struct PerftStats {
    uint64_t nodes = 0;
    uint64_t captures = 0;
    uint64_t enpassants = 0;
    uint64_t castles = 0;
    uint64_t promotions = 0;
    uint64_t checks = 0;
    uint64_t checkmates = 0;
};

// Recursive perft with full move statistics
void perft(Board& pos, int depth, PerftStats& stats, bool bulk_counting = true) {
    if (depth == 0) {
        stats.nodes++;
        if (pos.last_move().is_capture()) stats.captures++;
        if (pos.last_move().is_enpassant()) stats.enpassants++;
        if (pos.last_move().is_castle()) stats.castles++;
        if (pos.last_move().is_promotion()) stats.promotions++;
        if (pos.in_check()) stats.checks++;
        if (pos.is_checkmate()) stats.checkmates++;
        return;
    }

    vector<Move> moves;
    generate_moves(pos, moves);

    // Bulk counting optimization (3x faster at depth 6+)
    if (bulk_counting && depth == 1) {
        stats.nodes += moves.size();
        return;
    }

    for (const Move& m : moves) {
        if (!pos.is_legal(m)) continue;
        pos.make_move(m);
        perft(pos, depth - 1, stats, bulk_counting);
        pos.unmake_move();
    }
}

// Detailed move breakdown with verification
void perft_divide(Board& pos, int depth, bool verify) {
    vector<Move> moves;
    generate_moves(pos, moves);

    PerftStats total;
    vector<pair<Move, PerftStats>> move_stats;

    for (const Move& m : moves) {
        if (!pos.is_legal(m)) continue;
        
        PerftStats current;
        pos.make_move(m);
        perft(pos, depth - 1, current);
        pos.unmake_move();

        move_stats.emplace_back(m, current);
        total.nodes += current.nodes;
        total.captures += current.captures;
        total.enpassants += current.enpassants;
        total.castles += current.castles;
        total.promotions += current.promotions;
        total.checks += current.checks;
        total.checkmates += current.checkmates;
    }

    // Print detailed breakdown
    cout << left << setw(8) << "Move" << right 
         << setw(12) << "Nodes" 
         << setw(12) << "Captures"
         << setw(12) << "EP" 
         << setw(12) << "Castles" 
         << setw(12) << "Promos" 
         << setw(12) << "Checks" 
         << setw(12) << "Mates\n";
    cout << string(96, '-') << "\n";

    for (const auto& [move, stats] : move_stats) {
        cout << left << setw(8) << move.to_string() << right 
             << setw(12) << stats.nodes
             << setw(12) << stats.captures
             << setw(12) << stats.enpassants
             << setw(12) << stats.castles
             << setw(12) << stats.promotions
             << setw(12) << stats.checks
             << setw(12) << stats.checkmates << "\n";
    }

    cout << "\nTotal:     " << right
         << setw(12) << total.nodes 
         << setw(12) << total.captures
         << setw(12) << total.enpassants
         << setw(12) << total.castles
         << setw(12) << total.promotions
         << setw(12) << total.checks
         << setw(12) << total.checkmates << "\n";

    // Verification against known positions
    if (verify) {
        const string fen = pos.get_fen();
        if (fen == "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1") {
            const uint64_t expected[] = {1, 20, 400, 8902, 197281, 4865609};
            if (depth <= 6 && total.nodes != expected[depth]) {
                cerr << "ERROR: Expected " << expected[depth] << " nodes!\n";
            }
        }
    }
}

int main(int argc, char* argv[]) {
    if (argc < 3) {
        cerr << "Usage: " << argv[0] << " <FEN> <depth> [divide] [bulk] [verify]\n"
             << "Options:\n"
             << "  divide - show move-by-move breakdown\n"
             << "  bulk   - enable bulk counting (default: on)\n"
             << "  verify - check against standard positions\n";
        return 1;
    }

    // Parse arguments
    const string fen = argv[1];
    const int depth = stoi(argv[2]);
    bool divide = false, bulk = true, verify = false;
    
    for (int i = 3; i < argc; i++) {
        const string opt = argv[i];
        if (opt == "divide") divide = true;
        else if (opt == "nobulk") bulk = false;
        else if (opt == "verify") verify = true;
    }

    // Initialize board
    Board pos;
    if (!pos.set_fen(fen)) {
        cerr << "Invalid FEN: " << fen << "\n";
        return 1;
    }

    // Run perft
    auto start = high_resolution_clock::now();
    PerftStats stats;
    
    if (divide) {
        perft_divide(pos, depth, verify);
    } else {
        perft(pos, depth, stats, bulk);
        cout << "Perft(" << depth << ") = " << stats.nodes << "\n"
             << "Captures:  " << stats.captures << "\n"
             << "En Passant: " << stats.enpassants << "\n"
             << "Castles:   " << stats.castles << "\n"
             << "Promotions: " << stats.promotions << "\n"
             << "Checks:    " << stats.checks << "\n"
             << "Checkmates: " << stats.checkmates << "\n";
    }

    // Print timing
    auto end = high_resolution_clock::now();
    double elapsed = duration_cast<duration<double>>(end - start).count();
    cout << fixed << setprecision(3)
         << "Time: " << elapsed << "s (" 
         << static_cast<uint64_t>(stats.nodes / elapsed) << " nps)\n";

    return 0;
}