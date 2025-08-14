#pragma once
#include <cstdint>
#include <string>
#include <cassert>

enum PieceType : uint8_t { NONE, PAWN, KNIGHT, BISHOP, ROOK, QUEEN, KING };
enum Color : uint8_t { WHITE, BLACK, COLOR_NONE };

// Optimized Move representation (32 bits total)
// Bit layout:
// 0-5:   from square (0-63)
// 6-11:  to square (0-63)
// 12-14: promotion piece (NONE, KNIGHT, BISHOP, ROOK, QUEEN)
// 15-17: move flags (normal, promotion, en passant, castling)
// 18-31: unused (future expansion)
class Move {
public:
    // Move flags (bit 15-17)
    enum Flag : uint8_t {
        NORMAL      = 0,
        PROMOTION   = 1 << 0,
        EN_PASSANT  = 1 << 1,
        CASTLING    = 1 << 2,
        DOUBLE_PUSH = 1 << 3  // For pawn history
    };

    Move() : move_(0) {}
    
    // Main constructor
    Move(int from, int to, PieceType promo = NONE, uint8_t flags = NORMAL) {
        assert(from >= 0 && from < 64);
        assert(to >= 0 && to < 64);
        assert(promo <= QUEEN);  // Only allow promotions to Q, R, B, N
        
        move_ = (static_cast<uint32_t>(from) |
               (static_cast<uint32_t>(to) << 6) |
               (static_cast<uint32_t>(promo) << 12) |
               (static_cast<uint32_t>(flags) << 15));
    }

    // Accessors
    int from() const { return move_ & 0x3F; }
    int to() const { return (move_ >> 6) & 0x3F; }
    PieceType promotion_piece() const { return static_cast<PieceType>((move_ >> 12) & 0x7); }
    uint8_t flags() const { return (move_ >> 15) & 0xF; }

    // Predicates
    bool is_promotion() const { return flags() & PROMOTION; }
    bool is_castle() const { return flags() & CASTLING; }
    bool is_enpassant() const { return flags() & EN_PASSANT; }
    bool is_double_push() const { return flags() & DOUBLE_PUSH; }
    bool is_valid() const { return move_ != 0; }

    // Comparison
    bool operator==(const Move& other) const { return move_ == other.move_; }
    bool operator!=(const Move& other) const { return !(*this == other); }

    // String conversion (UCI format)
    std::string to_uci() const {
        if (!is_valid()) return "0000";
        
        std::string result;
        result += static_cast<char>('a' + (from() % 8));
        result += static_cast<char>('1' + (from() / 8));
        result += static_cast<char>('a' + (to() % 8));
        result += static_cast<char>('1' + (to() / 8));
        
        if (is_promotion()) {
            const char promos[] = {'n', 'b', 'r', 'q'};
            result += promos[promotion_piece() - KNIGHT];
        }
        
        return result;
    }

    // Static constructors
    static Move from_uci(const std::string& uci) {
        if (uci == "0000") return none();
        
        int from = (uci[0] - 'a') + 8 * (uci[1] - '1');
        int to = (uci[2] - 'a') + 8 * (uci[3] - '1');
        PieceType promo = NONE;
        uint8_t flags = NORMAL;
        
        if (uci.length() > 4) {
            flags |= PROMOTION;
            switch (uci[4]) {
                case 'n': promo = KNIGHT; break;
                case 'b': promo = BISHOP; break;
                case 'r': promo = ROOK; break;
                case 'q': promo = QUEEN; break;
            }
        }
        
        return Move(from, to, promo, flags);
    }

    static Move none() { return Move(); }

    // For std::unordered_map support
    struct Hash {
        size_t operator()(const Move& m) const {
            return m.move_;
        }
    };

private:
    uint32_t move_;  // Compact 32-bit representation
};