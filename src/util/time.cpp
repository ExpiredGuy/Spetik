#include "timer.h"
#include <thread>

#ifdef _WIN32
#include <windows.h>
#else
#include <time.h>
#endif

namespace Timer {

void sleep_ns(uint64_t ns) {
    if (ns == 0) return;
    
#ifdef _WIN32
    // Windows high-resolution sleep
    static bool initialized = [](){
        timeBeginPeriod(1); // Request 1ms timer resolution
        return true;
    }();
    HANDLE timer = CreateWaitableTimer(NULL, TRUE, NULL);
    LARGE_INTEGER li;
    li.QuadPart = -(int64_t)(ns / 100); // Relative time in 100ns units
    SetWaitableTimer(timer, &li, 0, NULL, NULL, FALSE);
    WaitForSingleObject(timer, INFINITE);
    CloseHandle(timer);
#else
    // Linux/OSX nanosleep
    timespec ts;
    ts.tv_sec = ns / 1'000'000'000;
    ts.tv_nsec = ns % 1'000'000'000;
    nanosleep(&ts, nullptr);
#endif
}

std::string format_time(uint64_t ms) {
    if (ms < 1'000) return std::to_string(ms) + "ms";
    
    const uint64_t seconds = ms / 1'000;
    ms %= 1'000;
    
    if (seconds < 60) return std::to_string(seconds) + "." + 
                          std::to_string(ms/100).substr(0,1) + "s";
    
    const uint64_t minutes = seconds / 60;
    const uint64_t hours = minutes / 60;
    const uint64_t days = hours / 24;
    
    std::string result;
    if (days > 0) result += std::to_string(days) + "d ";
    if (hours > 0) result += std::to_string(hours % 24) + "h ";
    if (minutes > 0) result += std::to_string(minutes % 60) + "m ";
    result += std::to_string(seconds % 60) + "s";
    
    return result;
}

ScopedTimer::~ScopedTimer() {
    auto duration = Clock::now() - start;
    auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(duration).count();
    if (!label.empty()) {
        printf("[TIMER] %s: %s\n", label.c_str(), format_time(ms).c_str());
    }
}

} // namespace Timer