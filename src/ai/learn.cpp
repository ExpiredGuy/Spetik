#include "learn.h"
#include "movegen.h"
#include "zobrist.h"
#include <fstream>
#include <iostream>

namespace {
    // Learning data storage
    std::unordered_map<uint64_t, std::vector<Learn::Entry>> learning_data;
    std::string learn_filename;
    constexpr size_t MAX_ENTRIES = 10'000'000;
}

namespace Learn {
    void init(const std::string& filename) {
        learn_filename = filename;
        learning_data.reserve(500'000);

        // Load existing data
        if (std::filesystem::exists(learn_filename)) {
            std::ifstream file(learn_filename, std::ios::binary);
            if (file) {
                Entry entry;
                while (file.read(reinterpret_cast<char*>(&entry), sizeof(Entry))) {
                    learning_data[entry.key].push_back(entry);
                }
                std::cout << "Loaded " << learning_data.size() << " learning entries\n";
            }
        }
    }

    void save() {
        std::ofstream file(learn_filename, std::ios::binary);
        if (!file) return;

        for (const auto& [key, entries] : learning_data) {
            for (const auto& entry : entries) {
                file.write(reinterpret_cast<const char*>(&entry), sizeof(Entry));
            }
        }
    }

    void add_position(const Board& board, Move move, int result) {
        if (learning_data.size() >= MAX_ENTRIES) return;

        const uint64_t key = board.key();
        auto& entries = learning_data[key];

        // Check if move already exists
        for (auto& entry : entries) {
            if (entry.move == move) {
                entry.count++;
                entry.result = (entry.result * entry.count + result) / (entry.count + 1);
                return;
            }
        }

        // Add new entry
        entries.push_back({
            key,
            move,
            static_cast<int16_t>(board.eval()),
            static_cast<uint16_t>(1),
            static_cast<uint8_t>(result)
        });
    }

    int adjust_eval(const Board& board, Move move, int currentEval) {
        const uint64_t key = board.key();
        const auto it = learning_data.find(key);
        if (it == learning_data.end()) return currentEval;

        for (const auto& entry : it->second) {
            if (entry.move == move) {
                // Weighted adjustment based on learned results
                const float weight = std::min(1.0f, logf(entry.count + 1) / 10.0f);
                return currentEval * (1 - weight) + entry.score * weight;
            }
        }
        return currentEval;
    }

    Move suggest_move(const Board& board) {
        const uint64_t key = board.key();
        const auto it = learning_data.find(key);
        if (it == learning_data.end()) return Move::none();

        // Find move with highest success rate
        Move best_move = Move::none();
        float best_score = -1.0f;

        for (const auto& entry : it->second) {
            const float score = entry.result / static_cast<float>(entry.count);
            if (score > best_score) {
                best_score = score;
                best_move = entry.move;
            }
        }

        return best_move;
    }
}