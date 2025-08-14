#include "book.h"
#include <iostream>
#include <fstream>
#include <random>
#include <algorithm>
#include <fcntl.h>
#include <unistd.h>
#include <sys/mman.h>
#include <cassert>

struct PolyglotEntry {
    uint64_t key;
    uint16_t move;
    uint16_t weight;
    uint32_t learn;
};

Book::Book() = default;

Book::~Book() {
    unload_mmap();
}

void Book::unload_mmap() {
    if (mappedData) {
        munmap(mappedData, mappedSize);
        mappedData = nullptr;
    }
    if (fd != -1) {
        close(fd);
        fd = -1;
    }
}

bool Book::load(const std::string& filename, BookFormat format) {
    unload_mmap();
    std::unique_lock lock(bookMutex);
    bookMap.clear();

    if (format == BookFormat::POLYGLOT) return load_polyglot(filename);
    else return load_bin(filename);
}

bool Book::load_polyglot(const std::string& filename) {
    fd = open(filename.c_str(), O_RDONLY);
    if (fd == -1) {
        std::cerr << "Cannot open book file: " << filename << "\n";
        return false;
    }

    mappedSize = lseek(fd, 0, SEEK_END);
    if (mappedSize == (size_t)-1) {
        std::cerr << "Failed to get file size: " << filename << "\n";
        close(fd);
        fd = -1;
        return false;
    }

    mappedData = mmap(nullptr, mappedSize, PROT_READ, MAP_PRIVATE, fd, 0);
    if (mappedData == MAP_FAILED) {
        std::cerr << "mmap failed for: " << filename << "\n";
        close(fd);
        fd = -1;
        return false;
    }

    size_t numEntries = mappedSize / sizeof(PolyglotEntry);
    bookMap.reserve(numEntries / 2);

    auto entries = static_cast<const PolyglotEntry*>(mappedData);

    for (size_t i = 0; i < numEntries; ++i) {
        // On little-endian systems, Polyglot entries need byte swap
        uint64_t key = __builtin_bswap64(entries[i].key);
        uint16_t moveEnc = __builtin_bswap16(entries[i].move);
        uint16_t weight = __builtin_bswap16(entries[i].weight);
        uint16_t learn = static_cast<uint16_t>( __builtin_bswap32(entries[i].learn) & 0xFFFF );

        Move move = polyglot_to_move(moveEnc);
        if (move.is_none()) continue; // skip invalid

        bookMap[key].emplace_back(BookEntry{ move, weight, learn });
    }

    // Basic sanity check
    const uint64_t TEST_KEY = 0x463b96181691fc9cULL; // Start position key (Polyglot)
    if (bookMap.find(TEST_KEY) == bookMap.end()) {
        std::cerr << "WARNING: Book may be corrupt or empty\n";
    }

    std::cout << "Loaded Polyglot book: " << bookMap.size() << " positions.\n";
    return true;
}

bool Book::load_bin(const std::string& filename) {
    // Implement your custom binary format loader here if needed
    std::cerr << "BIN format loading not implemented.\n";
    return false;
}

std::vector<BookEntry> Book::get_moves(uint64_t zobristKey) const {
    std::shared_lock lock(bookMutex);
    auto it = bookMap.find(zobristKey);
    if (it != bookMap.end()) return it->second;
    return {};
}

bool Book::get_move(uint64_t zobristKey, Move& outMove, BookPolicy policy) const {
    std::shared_lock lock(bookMutex);
    auto it = bookMap.find(zobristKey);
    if (it == bookMap.end() || it->second.empty()) return false;

    const auto& entries = it->second;

    static thread_local std::mt19937 gen(std::random_device{}());

    if (policy == BookPolicy::BEST_ONLY) {
        auto best_it = std::max_element(entries.begin(), entries.end(),
            [](const BookEntry& a, const BookEntry& b) { return a.weight < b.weight; });
        outMove = best_it->move;
        return true;
    }
    else if (policy == BookPolicy::UNIFORM) {
        std::uniform_int_distribution<size_t> dis(0, entries.size() - 1);
        outMove = entries[dis(gen)].move;
        return true;
    }
    else if (policy == BookPolicy::WEIGHTED) {
        // Normalize weights to prevent overflow
        int maxWeight = 1;
        for (auto& e : entries)
            if (e.weight > maxWeight) maxWeight = e.weight;

        std::vector<double> probs;
        double total = 0;
        for (auto& e : entries) {
            double p = (double)e.weight / maxWeight;
            probs.push_back(p);
            total += p;
        }

        std::uniform_real_distribution<> dis(0, total);
        double r = dis(gen);

        for (size_t i = 0; i < probs.size(); ++i) {
            r -= probs[i];
            if (r <= 0) {
                outMove = entries[i].move;
                return true;
            }
        }
        outMove = entries[0].move; // fallback
        return true;
    }
    return false;
}

bool Book::is_opening_phase(uint64_t zobristKey) const {
    std::shared_lock lock(bookMutex);
    auto it = bookMap.find(zobristKey);
    return it != bookMap.end() && it->second.size() > 3;
}

Move Book::polyglot_to_move(uint16_t poly_move) {
    int toFile   = (poly_move & 0x7);
    int toRank   = (poly_move >> 3) & 0x7;
    int fromFile = (poly_move >> 6) & 0x7;
    int fromRank = (poly_move >> 9) & 0x7;
    int promo   = (poly_move >> 12) & 0xF;

    int from_sq = fromRank * 8 + fro*_
