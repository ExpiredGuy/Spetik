#include "board.h"
#include <iostream>

// Helper intrinsics - GCC/Clang built-in; replace if needed
inline int bit_scan_forward(uint64_t bb) {
    return __builtin_ctzll(bb);
}

// File masks for pawn attacks to avoid wrap-around
constexpr uint64_t FILE_A = 0x0101010101010101ULL;
constexpr uint64_t FILE_H = 0x8080808080808080ULL;

// Precomputed attack arrays (You need to implement/fill these)
extern uint64_t knight_attacks[64];
extern uint64_t king_attacks[64];
extern uint64_t between_bb[64][64];

// Stub sliding attack functions (you must implement with magic bitboards or similar)
uint64_t get_bishop_attacks(int sq, uint64_t occ) {
    // TODO: Implement bishop attacks using occupancy 'occ'
    return 0ULL;
}

uint64_t get_rook_attacks(int sq, uint64_t occ) {
    // TODO: Implement rook attacks using occupancy 'occ'
    return 0ULL;
}

// === Board class implementation ===

Board::Board() {
    reset_board();
}

void Board::reset_board() {
    // Starting position bitboards

    pieces[WHITE][PAWN]   = 0x000000000000FF00ULL;
    pieces[WHITE][KNIGHT] = 0x0000000000000042ULL;
    pieces[WHITE][BISHOP] = 0x0000000000000024ULL;
    pieces[WHITE][ROOK]   = 0x0000000000000081ULL;
    pieces[WHITE][QUEEN]  = 0x0000000000000008ULL;
    pieces[WHITE][KING]   = 0x0000000000000010ULL;

    pieces[BLACK][PAWN]   = 0x00FF000000000000ULL;
    pieces[BLACK][KNIGHT] = 0x4200000000000000ULL;
    pieces[BLACK][BISHOP] = 0x2400000000000000ULL;
    pieces[BLACK][ROOK]   = 0x8100000000000000ULL;
    pieces[BLACK][QUEEN]  = 0x0800000000000000ULL;
    pieces[BLACK][KING]   = 0x1000000000000000ULL;

    castlingRights = 0b1111;  // KQkq allowed
    enPassantSquare = -1;
    halfmoveClock = 0;
    fullmoveNumber = 1;
    sideToMove = WHITE;

    update_occupancy();

    // Initialize Zobrist key
    zobristKey = 0ULL;
    for (Color c : {WHITE, BLACK}) {
        for (PieceType pt = PAWN; pt <= KING; pt++) {
            uint64_t bb = pieces[c][pt];
            while (bb) {
                int sq = bit_scan_forward(bb);
                zobristKey ^= ZOBRIST_PIECE_KEYS[make_piece(c, pt)][sq];
                bb &= bb - 1;
            }
        }
    }
    if (sideToMove == BLACK) zobristKey ^= ZOBRIST_SIDE_KEY;
    zobristKey ^= ZOBRIST_CASTLING_KEYS[castlingRights];
    if (enPassantSquare != -1)
        zobristKey ^= ZOBRIST_EP_KEYS[enPassantSquare % 8];
}

void Board::update_occupancy() {
    occupancy[WHITE] = pieces[WHITE][PAWN] | pieces[WHITE][KNIGHT] | 
                      pieces[WHITE][BISHOP] | pieces[WHITE][ROOK] | 
                      pieces[WHITE][QUEEN] | pieces[WHITE][KING];
    occupancy[BLACK] = pieces[BLACK][PAWN] | pieces[BLACK][KNIGHT] | 
                      pieces[BLACK][BISHOP] | pieces[BLACK][ROOK] | 
                      pieces[BLACK][QUEEN] | pieces[BLACK][KING];
    occupancy[2] = occupancy[WHITE] | occupancy[BLACK];
}

uint64_t Board::get_piece_bb(Color c, PieceType pt) const {
    return pieces[c][pt];
}

uint64_t Board::get_occupancy(Color c) const {
    return occupancy[c];
}

uint64_t Board::get_all_occupancy() const {
    return occupancy[2];
}

int Board::square_index(int rank, int file) const {
    return rank * 8 + file;
}

uint64_t Board::get_attacks_to(int square, Color attacker) const {
    uint64_t attackers = 0ULL;
    uint64_t target = 1ULL << square;

    // Pawns
    uint64_t pawns = pieces[attacker][PAWN];
    if (attacker == WHITE) {
        attackers |= ((target >> 7) & ~FILE_H) | ((target >> 9) & ~FILE_A);
    } else {
        attackers |= ((target << 7) & ~FILE_A) | ((target << 9) & ~FILE_H);
    }
    attackers &= pawns;

    // Knights
    attackers |= knight_attacks[square] & pieces[attacker][KNIGHT];

    // Bishops and Queens (diagonals)
    attackers |= get_bishop_attacks(square, occupancy[2]) & 
                 (pieces[attacker][BISHOP] | pieces[attacker][QUEEN]);

    // Rooks and Queens (straight)
    attackers |= get_rook_attacks(square, occupancy[2]) & 
                 (pieces[attacker][ROOK] | pieces[attacker][QUEEN]);

    // King
    attackers |= king_attacks[square] & pieces[attacker][KING];

    return attackers;
}

uint64_t Board::get_checkers() const {
    // TODO: Implement: find pieces checking the king of sideToMove
    // Stub for now:
    return 0ULL;
}

bool Board::is_square_attacked(int square, Color by_color) const {
    return get_attacks_to(square, by_color) != 0ULL;
}

uint64_t Board::pinned_pieces(Color c) const {
    uint64_t pinned = 0ULL;
    int king_sq = bit_scan_forward(pieces[c][KING]);
    uint64_t sliders = pieces[!c][BISHOP] | pieces[!c][ROOK] | pieces[!c][QUEEN];

    while (sliders) {
        int sq = bit_scan_forward(sliders);
        uint64_t between = between_bb[sq][king_sq] & occupancy[2];
        if (__builtin_popcountll(between) == 1) pinned |= between;
        sliders &= sliders - 1;
    }
    return pinned & occupancy[c];
}

bool Board::is_legal(Move move) const {
    // TODO: Implement legality check including king safety, en passant, pins
    // Stub always returns true for now:
    return true;
}

bool Board::is_valid() const {
    // Basic sanity check (one king per side, no pawns on ranks 1 or 8, etc.)
    int white_kings = __builtin_popcountll(pieces[WHITE][KING]);
    int black_kings = __builtin_popcountll(pieces[BLACK][KING]);

    if (white_kings != 1 || black_kings != 1) return false;

    uint64_t pawns = pieces[WHITE][PAWN] | pieces[BLACK][PAWN];
    if (pawns & (0xFFULL | (0xFFULL << 56))) return false;  // Pawns on first or last rank

    return true;
}

void Board::print() const {
    const char piece_chars[12] = { 'P','N','B','R','Q','K','p','n','b','r','q','k' };

    for (int rank = 7; rank >= 0; --rank) {
        for (int file = 0; file < 8; ++file) {
            int sq = rank * 8 + file;
            bool found = false;
            for (Color c : {WHITE, BLACK}) {
                for (int pt = PAWN; pt <= KING; ++pt) {
                    if (pieces[c][pt] & (1ULL << sq)) {
                        int idx = pt + (c == BLACK ? 6 : 0);
                        std::cout << piece_chars[idx];
                        found = true;
                        break;
                    }
                }
                if (found) break;
            }
            if (!found) std::cout << '.';
        }
        std::cout << "\n";
    }
}
