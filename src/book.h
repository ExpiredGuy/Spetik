#pragma once
#include <cstdint>
#include <vector>
#include <string>
#include <unordered_map>
#include <shared_mutex>
#include "move.h"

enum class BookPolicy {
    UNIFORM,
    WEIGHTED,
    BEST_ONLY
};

enum class BookFormat {
    BIN,
    POLYGLOT
};

#pragma pack(push, 1)
struct BookEntry {
    Move move;
    uint16_t weight;
    uint16_t learn;
};
#pragma pack(pop)

class Book {
public:
    Book();
    ~Book();

    bool load(const std::string& filename, BookFormat format = BookFormat::POLYGLOT);

    std::vector<BookEntry> get_moves(uint64_t zobristKey) const;

    bool get_move(uint64_t zobristKey, Move& outMove, BookPolicy policy = BookPolicy::WEIGHTED) const;

    bool is_opening_phase(uint64_t zobristKey) const;

    size_t size() const { return bookMap.size(); }
    bool empty() const { return bookMap.empty(); }

private:
    using BookMapType = std::unordered_map<uint64_t, std::vector<BookEntry>>;

    BookMapType bookMap;

    mutable std::shared_mutex bookMutex;

    bool load_polyglot(const std::string& filename);
    bool load_bin(const std::string& filename);

    // Memory mapped file handle (for POSIX)
    int fd = -1;
    void* mappedData = nullptr;
    size_t mappedSize = 0;

    void unload_mmap();

    // Helpers
    static Move polyglot_to_move(uint16_t poly_move);
};
