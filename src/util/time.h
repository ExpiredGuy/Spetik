#pragma once
#include <cstdint>
#include <string>
#include <chrono>

namespace Timer {

// High-resolution clock (ns precision)
using Clock = std::chrono::high_resolution_clock;
using TimePoint = std::chrono::time_point<Clock>;

// Returns current time in milliseconds since epoch (monotonic)
inline uint64_t now_ms() {
    return std::chrono::duration_cast<std::chrono::milliseconds>(
        Clock::now().time_since_epoch()).count();
}

// Returns current time in nanoseconds (highest precision available)
inline uint64_t now_ns() {
    return std::chrono::duration_cast<std::chrono::nanoseconds>(
        Clock::now().time_since_epoch()).count();
}

// Platform-specific accurate sleep (handles sub-millisecond sleeps)
void sleep_ns(uint64_t ns);

// Sleeps for milliseconds (uses OS-specific high-res sleep)
inline void sleep_ms(uint64_t ms) {
    if (ms > 0) sleep_ns(ms * 1'000'000);
}

// Formats milliseconds into "Xd Yh Zm As" or "X.Ys" for short durations
std::string format_time(uint64_t ms);

// RAII timer for profiling (reports duration on destruction)
class ScopedTimer {
    TimePoint start;
    std::string label;
public:
    explicit ScopedTimer(std::string_view label = "") 
        : start(Clock::now()), label(label) {}
    ~ScopedTimer();
};

} // namespace Timer