#include <iostream>
#include <fstream>
#include <vector>
#include <random>
#include <cstring>
#include "board.h"  // Your board representation
#include "trainer.h"

namespace NNUE {

static constexpr int INPUT_SIZE = 768; // 12 pieces × 64 squares
static constexpr int L1_SIZE    = 256;
static constexpr int L2_SIZE    = 32;
static constexpr int OUTPUT_SIZE = 1;

struct Weights {
    std::vector<float> l1_weights;
    std::vector<float> l1_bias;
    std::vector<float> l2_weights;
    std::vector<float> l2_bias;
    std::vector<float> l3_weights;
    std::vector<float> l3_bias;
};

static Weights model;

static std::vector<float> board_to_input(const Board& board) {
    std::vector<float> input(INPUT_SIZE, 0.0f);
    // Example piece encoding: 0-5 white pieces, 6-11 black pieces
    for (int sq = 0; sq < 64; ++sq) {
        Piece p = board.piece_at(sq);
        if (p != NO_PIECE) {
            int type_index = (is_white(p) ? 0 : 6) + piece_type(p);
            input[type_index * 64 + sq] = 1.0f;
        }
    }
    return input;
}

static float relu(float x) { return x > 0 ? x : 0; }

static float forward_single(const std::vector<float>& input) {
    std::vector<float> h1(L1_SIZE), h2(L2_SIZE);

    // Layer 1
    for (int i = 0; i < L1_SIZE; ++i) {
        float sum = model.l1_bias[i];
        for (int j = 0; j < INPUT_SIZE; ++j) {
            sum += input[j] * model.l1_weights[i * INPUT_SIZE + j];
        }
        h1[i] = relu(sum);
    }

    // Layer 2
    for (int i = 0; i < L2_SIZE; ++i) {
        float sum = model.l2_bias[i];
        for (int j = 0; j < L1_SIZE; ++j) {
            sum += h1[j] * model.l2_weights[i * L1_SIZE + j];
        }
        h2[i] = relu(sum);
    }

    // Output layer
    float sum = model.l3_bias[0];
    for (int j = 0; j < L2_SIZE; ++j) {
        sum += h2[j] * model.l3_weights[j];
    }
    return sum;
}

void Trainer::initialize_random_weights() {
    std::mt19937 rng(42);
    std::uniform_real_distribution<float> dist(-0.1f, 0.1f);

    model.l1_weights.resize(L1_SIZE * INPUT_SIZE);
    model.l1_bias.resize(L1_SIZE);
    model.l2_weights.resize(L2_SIZE * L1_SIZE);
    model.l2_bias.resize(L2_SIZE);
    model.l3_weights.resize(L2_SIZE);
    model.l3_bias.resize(1);

    for (auto& w : model.l1_weights) w = dist(rng);
    for (auto& b : model.l1_bias) b = dist(rng);
    for (auto& w : model.l2_weights) w = dist(rng);
    for (auto& b : model.l2_bias) b = dist(rng);
    for (auto& w : model.l3_weights) w = dist(rng);
    for (auto& b : model.l3_bias) b = dist(rng);
}

void Trainer::save_to_file(const std::string& path) {
    std::ofstream out(path, std::ios::binary);
    if (!out) {
        std::cerr << "Cannot open " << path << " for writing\n";
        return;
    }

    // Write a simple header
    const char header[] = "NNUE Evaluation Function\0";
    out.write(header, sizeof(header));

    auto write_vec = [&](const std::vector<float>& vec) {
        out.write(reinterpret_cast<const char*>(vec.data()), vec.size() * sizeof(float));
    };

    write_vec(model.l1_weights);
    write_vec(model.l1_bias);
    write_vec(model.l2_weights);
    write_vec(model.l2_bias);
    write_vec(model.l3_weights);
    write_vec(model.l3_bias);

    std::cout << "NNUE weights saved to " << path << "\n";
}

float Trainer::evaluate_board(const Board& board) {
    auto input = board_to_input(board);
    return forward_single(input);
}

} // namespace NNUE
