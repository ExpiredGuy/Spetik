#include "thread.h"
#include "search.h"
#include "tt.h"  // For shared hash table access
#include <iostream>
#include <sched.h>  // For CPU affinity

ThreadPool Threads;

Thread::Thread(size_t id) : 
    id(id),
    exit_flag(false), 
    searching(false),
    nodes_searched(0),
    root_depth(0) 
{
    // Initialize thread-local data
    history.clear();
    std::fill(killers.begin(), killers.end(), Move::none());
}

Thread::~Thread() {
    stop();
}

void Thread::start() {
    native_thread = std::thread(&Thread::idle_loop, this);
    
    // Set CPU affinity (Linux/MacOS)
    #ifdef __linux__
    cpu_set_t cpuset;
    CPU_ZERO(&cpuset);
    CPU_SET(id % std::thread::hardware_concurrency(), &cpuset);
    pthread_setaffinity_np(native_thread.native_handle(), sizeof(cpu_set_t), &cpuset);
    #endif
}

void Thread::stop() {
    {
        std::lock_guard<std::mutex> lock(mtx);
        exit_flag = true;
        searching = false;
    }
    cv.notify_all();
    if (native_thread.joinable()) {
        native_thread.join();
    }
}

void Thread::idle_loop() {
    while (true) {
        std::unique_lock<std::mutex> lock(mtx);
        cv.wait(lock, [this]{ return searching || exit_flag; });
        
        if (exit_flag) break;
        
        // Release lock during search
        lock.unlock();
        
        // Perform search
        SearchResult result = Search::think(*root_pos, limits, *this);
        
        // Update global best move if needed
        if (id == 0) {
            std::lock_guard<std::mutex> result_lock(result_mutex);
            if (result.depth > best_result.depth) {
                best_result = result;
            }
        }
        
        // Mark search complete
        lock.lock();
        searching = false;
        cv.notify_one();  // Notify main thread
    }
}

void Thread::begin_search(Position* pos, const SearchLimits& lim) {
    std::lock_guard<std::mutex> lock(mtx);
    root_pos = pos;
    limits = lim;
    nodes_searched = 0;
    searching = true;
    cv.notify_one();
}

void ThreadPool::init(size_t num_threads) {
    // Clear existing threads
    stop_all();
    
    // Create new threads
    for (size_t i = 0; i < num_threads; ++i) {
        threads.emplace_back(std::make_unique<Thread>(i));
        threads.back()->start();
    }
    
    // Set main thread pointer
    if (!threads.empty()) {
        main_thread = threads[0].get();
    }
}

void ThreadPool::stop_all() {
    for (auto& thread : threads) {
        thread->stop();
    }
    threads.clear();
}

Thread& ThreadPool::get(size_t idx) {
    return *threads.at(idx);  // bounds-checked access
}

// Statistics aggregation
uint64_t ThreadPool::total_nodes() const {
    uint64_t nodes = 0;
    for (const auto& thread : threads) {
        nodes += thread->nodes_searched.load(std::memory_order_relaxed);
    }
    return nodes;
}

// Main thread helper
void ThreadPool::wait_for_search_finish() {
    std::unique_lock<std::mutex> lock(main_thread->mtx);
    main_thread->cv.wait(lock, [this]{
        return !main_thread->searching.load(std::memory_order_acquire);
    });
}