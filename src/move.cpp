#include "move.h"
#include <array>
#include <stdexcept>

// Precomputed lookup tables for faster conversions
namespace {
    constexpr std::array<char, 8> files = {'a','b','c','d','e','f','g','h'};
    constexpr std::array<char, 8> ranks = {'1','2','3','4','5','6','7','8'};
    constexpr std::array<int, 256> char_to_file = []() {
        std::array<int, 256> table{};
        table['a'] = 0; table['b'] = 1; table['c'] = 2; table['d'] = 3;
        table['e'] = 4; table['f'] = 5; table['g'] = 6; table['h'] = 7;
        return table;
    }();
    constexpr std::array<int, 256> char_to_rank = []() {
        std::array<int, 256> table{};
        table['1'] = 0; table['2'] = 1; table['3'] = 2; table['4'] = 3;
        table['5'] = 4; table['6'] = 5; table['7'] = 6; table['8'] = 7;
        return table;
    }();
    constexpr std::array<char, 5> promo_chars = {'\0', 'n', 'b', 'r', 'q'};
    constexpr std::array<PieceType, 256> char_to_promo = []() {
        std::array<PieceType, 256> table{};
        table['n'] = KNIGHT;
        table['b'] = BISHOP;
        table['r'] = ROOK;
        table['q'] = QUEEN;
        return table;
    }();
}

Move::Move(int from, int to, PieceType promo, uint8_t flags) {
    // Use bitfields for compact storage and faster access
    struct PackedMove {
        unsigned from : 6;
        unsigned to : 6;
        unsigned promo : 4;
        unsigned flags : 8;
    };
    
    static_assert(sizeof(PackedMove) == 3, "PackedMove should be 3 bytes");

    // Validate inputs in debug builds only
    assert(from >= 0 && from < 64 && "Invalid from square");
    assert(to >= 0 && to < 64 && "Invalid to square");
    assert(promo <= QUEEN && "Invalid promotion piece");

    // Pack data using bitfields
    PackedMove packed{
        static_cast<unsigned>(from),
        static_cast<unsigned>(to),
        static_cast<unsigned>(promo),
        flags
    };

    // Copy packed structure into move_
    std::memcpy(&move_, &packed, sizeof(packed));
}

int Move::from() const {
    return move_ & 0x3F;
}

int Move::to() const {
    return (move_ >> 6) & 0x3F;
}

PieceType Move::promotion_piece() const {
    return static_cast<PieceType>((move_ >> 12) & 0xF);
}

uint8_t Move::flags() const {
    return static_cast<uint8_t>((move_ >> 16) & 0xFF);
}

bool Move::is_promotion() const {
    // Check promotion flag first for branch prediction
    return (flags() & FLAG_PROMOTION) || promotion_piece() != NONE;
}

bool Move::is_castle() const {
    return flags() & FLAG_CASTLE;
}

bool Move::is_enpassant() const {
    return flags() & FLAG_ENPASSANT;
}

bool Move::is_valid() const {
    // Fast path for common case
    if (move_ == 0) return false;
    
    const int f = from();
    const int t = to();
    return (f != t) && (f >= 0) && (f < 64) && (t >= 0) && (t < 64);
}

std::string Move::to_uci() const {
    if (!is_valid()) return "0000";

    std::string uci;
    uci.reserve(5);  // Most moves are 4 chars, promotions are 5
    
    // File and rank calculations using lookup tables
    uci += files[from() % 8];
    uci += ranks[from() / 8];
    uci += files[to() % 8];
    uci += ranks[to() / 8];

    // Append promotion character if needed
    if (is_promotion()) {
        uci += promo_chars[promotion_piece()];
    }
    
    return uci;
}

Move Move::from_uci(const std::string& uci) {
    // Fast rejection of invalid formats
    if (uci.size() < 4 || uci == "0000") return none();

    try {
        // Use lookup tables for faster conversion
        const int from_file = char_to_file[static_cast<unsigned char>(uci[0])];
        const int from_rank = char_to_rank[static_cast<unsigned char>(uci[1])];
        const int to_file = char_to_file[static_cast<unsigned char>(uci[2])];
        const int to_rank = char_to_rank[static_cast<unsigned char>(uci[3])];

        // Check for promotions
        PieceType promo = NONE;
        uint8_t flags = 0;
        
        if (uci.size() == 5) {
            promo = char_to_promo[static_cast<unsigned char>(uci[4])];
            flags |= FLAG_PROMOTION;
        }

        return Move(from_rank * 8 + from_file, to_rank * 8 + to_file, promo, flags);
    } catch (...) {
        return none();
    }
}

Move Move::none() {
    static const Move null_move;
    return null_move;
}