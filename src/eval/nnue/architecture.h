#pragma once
#include <cstdint>
#include <vector>
#include "simd.h"

namespace NNUE {

// Network architecture constants
constexpr int kInputDimensions = 256 * 64;  // HalfKP feature set
constexpr int kHiddenDimensions = 256;
constexpr int kOutputDimensions = 1;

// Quantization parameters
constexpr int kWeightScaleBits = 6;
constexpr int kActivationScaleBits = 10;

// Network structure
struct Network {
    alignas(64) int16_t feature_weights[kInputDimensions * kHiddenDimensions];
    alignas(64) int16_t feature_bias[kHiddenDimensions];
    alignas(64) int32_t hidden_weights[kHiddenDimensions * kHiddenDimensions];
    alignas(64) int32_t hidden_bias[kHiddenDimensions];
    alignas(64) int32_t output_weights[kHiddenDimensions];
    int32_t output_bias;
    
    // Initialize with random weights
    void initialize();
};

// Accumulator for efficient updates
struct Accumulator {
    alignas(64) int16_t hidden[kHiddenDimensions];
    bool dirty = true;
    
    void refresh(const Network& network, const std::vector<int>& active_features);
    void update(const Network& network, const std::vector<int>& added, const std::vector<int>& removed);
};

// Forward propagation
int32_t forward(const Network& network, const Accumulator& accumulator);

} // namespace NNUE