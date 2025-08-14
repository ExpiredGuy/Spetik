#include "prefetch.h"

#if defined(_MSC_VER)
#include <intrin.h>
#elif defined(__GNUC__)
#include <x86intrin.h>
#endif

namespace ChessEngine {

void prefetch(const void* addr, PrefetchHint hint) noexcept {
#if defined(__GNUC__) || defined(__clang__)
    // GCC/Clang intrinsic
    switch (hint) {
        case PrefetchHint::Read:
            __builtin_prefetch(addr, 0, 3); // Read, high temporal locality
            break;
        case PrefetchHint::Write:
            __builtin_prefetch(addr, 1, 3); // Write, high temporal locality
            break;
        case PrefetchHint::StrongRead:
            __builtin_prefetch(addr, 0, 0); // Read, minimal temporal locality
            break;
        case PrefetchHint::Streaming:
            __builtin_prefetch(addr, 0, 1); // Read, low temporal locality
            break;
    }
#elif defined(_MSC_VER)
    // MSVC intrinsic
    switch (hint) {
        case PrefetchHint::Read:
        case PrefetchHint::StrongRead:
            _mm_prefetch((const char*)addr, _MM_HINT_T0);
            break;
        case PrefetchHint::Write:
            _mm_prefetch((const char*)addr, _MM_HINT_T1);
            break;
        case PrefetchHint::Streaming:
            _mm_prefetch((const char*)addr, _MM_HINT_NTA);
            break;
    }
#else
    // Fallback - does nothing
    (void)addr;
    (void)hint;
#endif
}

} // namespace ChessEngine