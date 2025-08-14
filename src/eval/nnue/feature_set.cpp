#include "feature_set.h"

namespace NNUE {

void FeatureSet::get_active_features(const Board& board, std::vector<int>& features) {
    features.clear();
    features.reserve(kMaxActiveFeatures);
    
    Square king_sq[2] = {
        board.king_square(Color::WHITE),
        board.king_square(Color::BLACK)
    };
    
    for (Color c : {Color::WHITE, Color::BLACK}) {
        for (PieceType pt : {PAWN, KNIGHT, BISHOP, ROOK, QUEEN}) {
            Bitboard pieces = board.pieces(c, pt);
            while (pieces) {
                Square sq = pop_lsb(pieces);
                features.push_back(make_feature_index(
                    make_piece(c, pt), king_sq[c], sq));
            }
        }
    }
}

void FeatureSet::get_changed_features(const Board& board, Move move,
                                    std::vector<int>& removed, std::vector<int>& added) {
    // Implementation of incremental updates...
}

int FeatureSet::make_feature_index(Piece piece, Square king_sq, Square piece_sq) {
    // Implementation of feature hashing...
}

} // namespace NNUE