#pragma once
#include <cstdint>
#include <array>
#include <vector>
#include <string>

#include "move.h"       // Assume you have a Move class/struct defined elsewhere
#include "zobrist.h"    // For Zobrist hashing keys and functions

enum PieceType { PAWN, KNIGHT, BISHOP, ROOK, QUEEN, KING, NONE };
enum Color { WHITE, BLACK, COLOR_NONE };

class Board {
public:
    Board();

    // Bitboards for each piece type by color: [color][piecetype]
    std::array<std::array<uint64_t, 6>, 2> pieces;  // [color][piece]

    // Occupancy bitboards for white, black, and both sides
    uint64_t occupancy[3];  // 0 = WHITE, 1 = BLACK, 2 = BOTH

    // Current game state
    Color sideToMove;
    uint8_t castlingRights;  // Bits: K=1, Q=2, k=4, q=8 (0b1111 = all allowed)
    int enPassantSquare;     // Square index 0-63 or -1 if none
    int halfmoveClock;       // For 50-move rule
    int fullmoveNumber;      // Starts at 1

    // Zobrist hash key of current position
    uint64_t zobristKey;

    // Basic board setup and utility functions
    void reset_board();
    uint64_t get_piece_bb(Color c, PieceType pt) const;
    uint64_t get_occupancy(Color c) const;
    uint64_t get_all_occupancy() const;
    int square_index(int rank, int file) const;

    // Move generation helpers
    uint64_t get_attacks_to(int square, Color attacker) const;  // Pieces attacking given square
    uint64_t get_checkers() const;                              // Bitboard of pieces checking the king
    bool is_square_attacked(int square, Color by_color) const; // Is square attacked by side?

    // Pin and legality
    uint64_t pinned_pieces(Color c) const;                      // Pieces pinned to king
    bool is_legal(Move move) const;                             // Does move leave king safe?

    // Validation and integrity checks
    bool is_valid() const;  // Sanity checks: exactly one king, no pawns on rank 1/8, etc.

    // Debug and output
    void print() const;                  // ASCII board representation
    std::string to_fen() const;          // Export board to FEN string

private:
    void update_occupancy();             // Update occupancy arrays from piece bitboards
    void update_zobrist_key(Move move); // Incremental Zobrist hash update on move

    // Optional optimizations
    std::array<std::vector<int>, 2> pieceLists;  // Lists of squares occupied by each color's pieces
};
