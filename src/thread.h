#pragma once
#include <thread>
#include <vector>
#include <atomic>
#include <condition_variable>
#include <mutex>
#include "search.h"
#include "tt.h"  // For shared hash table access

class Thread {
public:
    explicit Thread(size_t id) : 
        id(id),
        exit_flag(false),
        searching(false),
        node_count(0),
        root_depth(0) {}
    
    ~Thread() {
        exit_flag = true;
        cv.notify_one();
        if (native_thread.joinable())
            native_thread.join();
    }

    void start() {
        native_thread = std::thread(&Thread::idle_loop, this);
    }

    void idle_loop() {
        while (!exit_flag) {
            std::unique_lock<std::mutex> lock(mtx);
            cv.wait(lock, [this]{ return searching || exit_flag; });
            
            if (searching && !exit_flag) {
                lock.unlock();
                Search::think(*root_pos, limits);
                searching = false;
            }
        }
    }

    void begin_search(Position* pos, const SearchLimits& lim) {
        std::lock_guard<std::mutex> lock(mtx);
        root_pos = pos;
        limits = lim;
        node_count = 0;
        searching = true;
        cv.notify_one();
    }

    void stop() {
        searching = false;
    }

    // Thread synchronization
    void wait_for_search_finish() {
        std::unique_lock<std::mutex> lock(mtx);
        cv.wait(lock, [this]{ return !searching; });
    }

    // Thread-local data
    size_t id;
    Position* root_pos;
    SearchLimits limits;
    std::atomic<uint64_t> node_count;
    std::atomic<int> root_depth;
    HistoryStats history;
    CounterMoveStats counter_moves;
    std::array<KillerMoves, Search::MAX_PLY> killers;

private:
    std::thread native_thread;
    std::atomic<bool> exit_flag;
    std::atomic<bool> searching;
    std::condition_variable cv;
    std::mutex mtx;
};

class ThreadPool {
public:
    void init(size_t num_threads) {
        threads.clear();
        for (size_t i = 0; i < num_threads; ++i) {
            threads.emplace_back(std::make_unique<Thread>(i));
            threads.back()->start();
        }
        main_thread = threads[0].get();
    }

    void stop_all() {
        for (auto& thread : threads)
            thread->stop();
    }

    Thread& get(size_t idx) { return *threads.at(idx); }
    Thread& main() { return *main_thread; }
    size_t size() const { return threads.size(); }

    // Statistics
    uint64_t total_nodes() const {
        uint64_t nodes = 0;
        for (const auto& thread : threads)
            nodes += thread->node_count.load();
        return nodes;
    }

private:
    std::vector<std::unique_ptr<Thread>> threads;
    Thread* main_thread;
};

extern ThreadPool Threads;