#pragma once
#include "board.h"
#include <vector>

namespace NNUE {

// HalfKP feature set (like Stockfish NNUE)
class FeatureSet {
public:
    static constexpr int kMaxActiveFeatures = 32;
    
    // Get active features for a position
    static void get_active_features(const Board& board, std::vector<int>& features);
    
    // Get changed features after a move
    static void get_changed_features(const Board& board, Move move, 
                                   std::vector<int>& removed, std::vector<int>& added);
    
private:
    static int make_feature_index(Piece piece, Square king_sq, Square piece_sq);
};

} // namespace NNUE