#ifndef TT_H
#define TT_H

#include "types.h"
#include "move.h"
#include <cstdint>
#include <memory>

enum Bound : uint8_t {
    BOUND_NONE = 0,
    BOUND_EXACT = 1,
    BOUND_LOWER = 2,
    BOUND_UPPER = 4
};

struct TTEntry {
    uint16_t key16;     // Part of Zobrist key for validation
    Move move;          // Best move
    int16_t value;      // Evaluation value
    int8_t depth;       // Search depth
    uint8_t bound;      // Bound type
    uint8_t generation; // Age counter
};

class TranspositionTable {
public:
    TranspositionTable();
    ~TranspositionTable();

    void resize(size_t mbSize);
    void clear();
    void newGeneration();

    TTEntry* probe(uint64_t key, bool& found) const;
    void store(uint64_t key, Move move, int value, Bound bound, int depth);

    size_t size() const { return numEntries; }
    int hashfull() const;

private:
    struct Cluster {
        TTEntry entries[3]; // Three entries per cluster
        uint8_t padding[2]; // Pad to 32 bytes
    };
    static_assert(sizeof(Cluster) == 32, "Incorrect cluster size");

    Cluster* table;
    size_t numEntries;
    uint8_t generation;

    size_t index(uint64_t key) const { return (key >> 32) % numEntries; }
    uint16_t key16(uint64_t key) const { return static_cast<uint16_t>(key >> 16); }
};

extern TranspositionTable TT;

#endif // TT_H