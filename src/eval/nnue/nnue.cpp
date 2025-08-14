#include "nnue.h"
#include <vector>
#include <fstream>
#include <random>
#include <cstring>
#include <cstdint>
#include <algorithm>
#include <iostream>

// ---- Simple, original NNUE-like format (FLOAT weights) ----
// File layout (little-endian):
// [ Magic (32 bytes): "MYNNUE v1\0" padded with zeros ]
// [ int32: inputSize ]
// [ int32: l1Size    ]
// [ int32: l2Size    ]
// [ int32: outputSize]
// [ float array: l1_weights (l1Size * inputSize) ]
// [ float array: l1_bias    (l1Size)            ]
// [ float array: l2_weights (l2Size * l1Size)   ]
// [ float array: l2_bias    (l2Size)            ]
// [ float array: l3_weights (outputSize * l2Size) == l2Size ]
// [ float array: l3_bias    (outputSize) == 1    ]

namespace {

struct Net {
    int inputSize=768, l1Size=256, l2Size=32, outputSize=1;
    std::vector<float> l1w, l1b;
    std::vector<float> l2w, l2b;
    std::vector<float> l3w, l3b;
};

NNUE::Config g_cfg;
Net g_net;
bool g_ready = false;

constexpr int MAGIC_BYTES = 32;
const char MAGIC[MAGIC_BYTES] = "MYNNUE v1";

inline float relu(float x) { return x > 0.f ? x : 0.f; }

// Minimal 12x64 one-hot feature extractor
// channels: [P,N,B,R,Q,K, p,n,b,r,q,k] each 64 squares (a1..h8 = 0..63)
void extract_features(const Board& b, std::vector<float>& input) {
    input.assign(g_net.inputSize, 0.0f);

    for (int sq = 0; sq < 64; ++sq) {
        Piece p = b.piece_at(sq);
        if (p == NO_PIECE) continue;

        // Your board.h should provide helpers; adapt if names differ
        bool white = (piece_color(p) == WHITE);
        int pt     = piece_type(p); // PAWN=1..KING=6 expected; adjust if needed
        if (pt < 1 || pt > 6) continue;

        int channel = (white ? (pt - 1) : (pt - 1 + 6)); // 0..11
        int idx = channel * 64 + sq;
        if (idx >= 0 && idx < g_net.inputSize) input[idx] = 1.0f;
    }
}

int forward_centipawns(const Board& b) {
    if (!g_ready) return 0;

    std::vector<float> x; x.reserve(g_net.inputSize);
    extract_features(b, x);

    // L1
    std::vector<float> h1(g_net.l1Size, 0.f);
    for (int i = 0; i < g_net.l1Size; ++i) {
        float s = g_net.l1b[i];
        const float* wrow = &g_net.l1w[i * g_net.inputSize];
        for (int j = 0; j < g_net.inputSize; ++j) s += wrow[j] * x[j];
        h1[i] = relu(s);
    }

    // L2
    std::vector<float> h2(g_net.l2Size, 0.f);
    for (int i = 0; i < g_net.l2Size; ++i) {
        float s = g_net.l2b[i];
        const float* wrow = &g_net.l2w[i * g_net.l1Size];
        for (int j = 0; j < g_net.l1Size; ++j) s += wrow[j] * h1[j];
        h2[i] = relu(s);
    }

    // Output
    float s = g_net.l3b[0];
    for (int j = 0; j < g_net.l2Size; ++j) s += g_net.l3w[j] * h2[j];

    // scale to centipawns (simple)
    int cp = static_cast<int>(std::clamp(s * 100.0f, -30000.0f, 30000.0f));
    return (b.side_to_move() == WHITE) ? cp : -cp; // normalize w.r.t. STM if you prefer
}

// I/O helpers
template <typename T>
bool read_block(std::ifstream& in, std::vector<T>& v, size_t count) {
    v.resize(count);
    return static_cast<bool>(in.read(reinterpret_cast<char*>(v.data()), sizeof(T)*count));
}
template <typename T>
bool write_block(std::ofstream& out, const std::vector<T>& v) {
    return static_cast<bool>(out.write(reinterpret_cast<const char*>(v.data()), sizeof(T)*v.size()));
}

bool write_header(std::ofstream& out) {
    char magic[MAGIC_BYTES]{}; std::memset(magic, 0, sizeof(magic));
    std::memcpy(magic, MAGIC, std::min<size_t>(std::strlen(MAGIC), MAGIC_BYTES-1));
    if (!out.write(magic, MAGIC_BYTES)) return false;

    int32_t sizes[4] = { g_net.inputSize, g_net.l1Size, g_net.l2Size, g_net.outputSize };
    return static_cast<bool>(out.write(reinterpret_cast<const char*>(sizes), sizeof(sizes)));
}

bool read_header(std::ifstream& in, int32_t sizes[4]) {
    char magic[MAGIC_BYTES];
    if (!in.read(magic, MAGIC_BYTES)) return false;
    if (std::strncmp(magic, MAGIC, std::strlen(MAGIC)) != 0) return false;
    return static_cast<bool>(in.read(reinterpret_cast<char*>(sizes), sizeof(int32_t)*4));
}

} // anonymous

namespace NNUE {

void set_config(const Config& cfg) {
    g_cfg = cfg;
    // If not initialized yet, configure net too:
    if (!g_ready) {
        g_net.inputSize  = g_cfg.inputSize;
        g_net.l1Size     = g_cfg.l1Size;
        g_net.l2Size     = g_cfg.l2Size;
        g_net.outputSize = g_cfg.outputSize;
    }
}

void init_random(unsigned seed, const Config& cfg) {
    set_config(cfg);
    std::mt19937 rng(seed);
    std::uniform_real_distribution<float> d(-0.1f, 0.1f);

    g_net.l1w.resize(g_net.l1Size * g_net.inputSize);
    g_net.l1b.resize(g_net.l1Size);
    g_net.l2w.resize(g_net.l2Size * g_net.l1Size);
    g_net.l2b.resize(g_net.l2Size);
    g_net.l3w.resize(g_net.outputSize * g_net.l2Size);
    g_net.l3b.resize(g_net.outputSize);

    for (auto& v: g_net.l1w) v = d(rng);
    for (auto& v: g_net.l1b) v = d(rng);
    for (auto& v: g_net.l2w) v = d(rng);
    for (auto& v: g_net.l2b) v = d(rng);
    for (auto& v: g_net.l3w) v = d(rng);
    for (auto& v: g_net.l3b) v = d(rng);

    g_ready = true;
}

bool load(const std::string& path) {
    std::ifstream in(path, std::ios::binary);
    if (!in) {
        std::cerr << "[NNUE] Failed to open: " << path << "\n";
        return false;
    }

    int32_t sizes[4];
    if (!read_header(in, sizes)) {
        std::cerr << "[NNUE] Bad header in: " << path << "\n";
        return false;
    }

    g_net.inputSize  = sizes[0];
    g_net.l1Size     = sizes[1];
    g_net.l2Size     = sizes[2];
    g_net.outputSize = sizes[3];

    // Allocate
    g_net.l1w.resize((size_t)g_net.l1Size * g_net.inputSize);
    g_net.l1b.resize(g_net.l1Size);
    g_net.l2w.resize((size_t)g_net.l2Size * g_net.l1Size);
    g_net.l2b.resize(g_net.l2Size);
    g_net.l3w.resize((size_t)g_net.outputSize * g_net.l2Size);
    g_net.l3b.resize(g_net.outputSize);

    // Read
    if (!read_block(in, g_net.l1w, g_net.l1w.size())) return false;
    if (!read_block(in, g_net.l1b, g_net.l1b.size())) return false;
    if (!read_block(in, g_net.l2w, g_net.l2w.size())) return false;
    if (!read_block(in, g_net.l2b, g_net.l2b.size())) return false;
    if (!read_block(in, g_net.l3w, g_net.l3w.size())) return false;
    if (!read_block(in, g_net.l3b, g_net.l3b.size())) return false;

    g_ready = true;
    std::cout << "[NNUE] Loaded " << path << " ("
              << g_net.inputSize << "->" << g_net.l1Size << "->" << g_net.l2Size
              << "->" << g_net.outputSize << ")\n";
    return true;
}

bool save(const std::string& path) {
    if (!g_ready) return false;
    std::ofstream out(path, std::ios::binary);
    if (!out) {
        std::cerr << "[NNUE] Failed to write: " << path << "\n";
        return false;
    }
    if (!write_header(out)) return false;
    if (!write_block(out, g_net.l1w)) return false;
    if (!write_block(out, g_net.l1b)) return false;
    if (!write_block(out, g_net.l2w)) return false;
    if (!write_block(out, g_net.l2b)) return false;
    if (!write_block(out, g_net.l3w)) return false;
    if (!write_block(out, g_net.l3b)) return false;
    std::cout << "[NNUE] Saved " << path << "\n";
    return true;
}

bool initialized() { return g_ready; }

int evaluate(const Board& pos) {
    return forward_centipawns(pos);
}

} // namespace NNUE
