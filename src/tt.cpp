#include "tt.h"
#include <cstdlib>
#include <cstring>
#include <iostream>

TranspositionTable TT;

TranspositionTable::TranspositionTable() : 
    table(nullptr), numEntries(0), generation(0) {}

TranspositionTable::~TranspositionTable() {
    std::free(table);
}

void TranspositionTable::resize(size_t mbSize) {
    std::free(table);
    numEntries = (mbSize * 1024 * 1024) / sizeof(Cluster);
    table = static_cast<Cluster*>(std::calloc(numEntries, sizeof(Cluster)));
    if (!table) {
        std::cerr << "Failed to allocate " << mbSize << "MB for transposition table\n";
        numEntries = 0;
    }
}

void TranspositionTable::clear() {
    if (table) {
        std::memset(table, 0, numEntries * sizeof(Cluster));
    }
}

void TranspositionTable::newGeneration() {
    generation++;
}

TTEntry* TranspositionTable::probe(uint64_t key, bool& found) const {
    if (numEntries == 0) {
        found = false;
        return nullptr;
    }

    Cluster& cluster = table[index(key)];
    const uint16_t check = key16(key);

    // Check all entries in cluster
    for (int i = 0; i < 3; ++i) {
        if (cluster.entries[i].key16 == check) {
            found = true;
            cluster.entries[i].generation = generation;
            return &cluster.entries[i];
        }
    }

    found = false;
    // Return the first entry as default replacement slot
    return &cluster.entries[0];
}

void TranspositionTable::store(uint64_t key, Move move, int value, Bound bound, int depth) {
    if (numEntries == 0) return;

    bool found;
    TTEntry* entry = probe(key, found);
    if (!entry) return;

    // Replacement scheme:
    // 1. Always replace if empty or same key
    // 2. Replace if depth is better
    if (!found || entry->key16 != key16(key) || depth + 3 > entry->depth) {
        entry->key16 = key16(key);
        entry->move = move;
        entry->value = static_cast<int16_t>(value);
        entry->depth = static_cast<int8_t>(depth);
        entry->bound = bound;
        entry->generation = generation;
    }
    // Always preserve best move if available
    else if (move && (depth >= entry->depth || !entry->move)) {
        entry->move = move;
    }
}

int TranspositionTable::hashfull() const {
    const int samples = 1000;
    int used = 0;

    for (int i = 0; i < samples; ++i) {
        const Cluster& cluster = table[(i * 2654435761U) % numEntries];
        for (int j = 0; j < 3; ++j) {
            if (cluster.entries[j].generation == generation) {
                used++;
                break;
            }
        }
    }
    return (used * 100) / samples;
}