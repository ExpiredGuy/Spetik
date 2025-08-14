#include "architecture.h"
#include <random>
#include <immintrin.h>

namespace NNUE {

void Network::initialize() {
    std::mt19937 gen(42);  // Fixed seed for reproducibility
    std::uniform_int_distribution<int16_t> dist(-127, 127);
    
    for (int i = 0; i < kInputDimensions * kHiddenDimensions; ++i) {
        feature_weights[i] = dist(gen);
    }
    
    // Initialize other layers similarly...
}

void Accumulator::refresh(const Network& network, const std::vector<int>& active_features) {
    // Reset to bias
    for (int i = 0; i < kHiddenDimensions; ++i) {
        hidden[i] = network.feature_bias[i];
    }
    
    // Add active features
    for (int f : active_features) {
        const int16_t* weights = &network.feature_weights[f * kHiddenDimensions];
        for (int i = 0; i < kHiddenDimensions; i += 16) {
            __m256i acc = _mm256_load_si256((__m256i*)(hidden + i));
            __m256i w = _mm256_loadu_si256((__m256i*)(weights + i));
            acc = _mm256_add_epi16(acc, w);
            _mm256_store_si256((__m256i*)(hidden + i), acc);
        }
    }
    dirty = false;
}

void Accumulator::update(const Network& network, const std::vector<int>& added, const std::vector<int>& removed) {
    // Process added features
    for (int f : added) {
        const int16_t* weights = &network.feature_weights[f * kHiddenDimensions];
        for (int i = 0; i < kHiddenDimensions; i += 16) {
            __m256i acc = _mm256_load_si256((__m256i*)(hidden + i));
            __m256i w = _mm256_loadu_si256((__m256i*)(weights + i));
            acc = _mm256_add_epi16(acc, w);
            _mm256_store_si256((__m256i*)(hidden + i), acc);
        }
    }
    
    // Process removed features (similar but subtract)
    // ...
}

int32_t forward(const Network& network, const Accumulator& accumulator) {
    alignas(64) int32_t hidden_output[kHiddenDimensions];
    
    // First layer (clipped ReLU)
    for (int i = 0; i < kHiddenDimensions; i += 8) {
        __m256i h = _mm256_load_si256((__m256i*)(accumulator.hidden + i));
        h = _mm256_max_epi16(h, _mm256_setzero_si256());
        h = _mm256_min_epi16(h, _mm256_set1_epi16(127));
        _mm256_store_si256((__m256i*)(hidden_output + i), h);
    }
    
    // Second layer (int8 -> int32)
    int32_t output = network.output_bias;
    for (int i = 0; i < kHiddenDimensions; ++i) {
        output += hidden_output[i] * network.output_weights[i];
    }
    
    return output / (1 << kActivationScaleBits);
}

} // namespace NNUE