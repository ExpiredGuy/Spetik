#pragma once
#include "architecture.h"
#include <vector>

namespace NNUE {

class Trainer {
public:
    struct TrainingSample {
        std::vector<int> features;
        float score;
    };
    
    void train(Network& network, const std::vector<TrainingSample>& samples, 
              float learning_rate = 0.01f, int batch_size = 1024);
    
    void update_weights(Network& network, const std::vector<int>& features, 
                       float error, float learning_rate);
    
private:
    void backpropagate(Network& network, const TrainingSample& sample, float error);
};

} // namespace NNUE