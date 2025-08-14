#pragma once
#include <cstdint>
#include <cstring>

// Platform detection with fallbacks
#if defined(__AVX512F__)
    #include <immintrin.h>
    #define SIMD_AVX512 1
#elif defined(__AVX2__)
    #include <immintrin.h>
    #define SIMD_AVX2 1
#elif defined(__SSE4_1__)
    #include <smmintrin.h>
    #define SIMD_SSE4 1
#elif defined(__SSE2__)
    #include <emmintrin.h>
    #define SIMD_SSE2 1
#elif defined(__ARM_NEON)
    #include <arm_neon.h>
    #define SIMD_NEON 1
#else
    #define SIMD_SCALAR 1
    #warning "No SIMD support detected - falling back to scalar code"
#endif

namespace simd {

// Aligned allocation (64-byte for AVX512, 32-byte for AVX2)
inline void* aligned_alloc(size_t size, size_t alignment = 64) {
#if defined(_WIN32)
    return _aligned_malloc(size, alignment);
#else
    return aligned_alloc(alignment, size);
#endif
}

inline void aligned_free(void* ptr) {
#if defined(_WIN32)
    _aligned_free(ptr);
#else
    free(ptr);
#endif
}

// 128-bit operations --------------------------------------------------

inline __m128i loadu128(const void* ptr) {
#if SIMD_SSE2 || SIMD_SSE4 || SIMD_AVX2 || SIMD_AVX512
    return _mm_loadu_si128(static_cast<const __m128i*>(ptr));
#elif SIMD_NEON
    return vld1q_s8(static_cast<const int8_t*>(ptr));
#else
    // Scalar fallback
    __m128i result;
    std::memcpy(&result, ptr, sizeof(__m128i));
    return result;
#endif
}

inline void storeu128(void* ptr, __m128i v) {
#if SIMD_SSE2 || SIMD_SSE4 || SIMD_AVX2 || SIMD_AVX512
    _mm_storeu_si128(static_cast<__m128i*>(ptr), v);
#elif SIMD_NEON
    vst1q_s8(static_cast<int8_t*>(ptr), v);
#else
    std::memcpy(ptr, &v, sizeof(__m128i));
#endif
}

inline __m128i add_epi16(__m128i a, __m128i b) {
#if SIMD_SSE2 || SIMD_SSE4 || SIMD_AVX2 || SIMD_AVX512
    return _mm_add_epi16(a, b);
#elif SIMD_NEON
    return vaddq_s16(a, b);
#else
    // Scalar fallback
    alignas(16) int16_t result[8];
    alignas(16) int16_t va[8], vb[8];
    storeu128(va, a);
    storeu128(vb, b);
    for (int i = 0; i < 8; ++i) result[i] = va[i] + vb[i];
    return loadu128(result);
#endif
}

inline __m128i madd_epi16(__m128i a, __m128i b) {
#if SIMD_SSE4 || SIMD_AVX2 || SIMD_AVX512
    return _mm_madd_epi16(a, b);
#elif SIMD_NEON
    return vpaddq_s32(vmull_s16(vget_low_s16(a), vget_low_s16(b)),
                     vmull_s16(vget_high_s16(a), vget_high_s16(b)));
#else
    // Scalar fallback
    alignas(16) int32_t result[4];
    alignas(16) int16_t va[8], vb[8];
    storeu128(va, a);
    storeu128(vb, b);
    for (int i = 0; i < 4; ++i)
        result[i] = (int32_t)va[2*i] * vb[2*i] + (int32_t)va[2*i+1] * vb[2*i+1];
    return loadu128(result);
#endif
}

inline int hsum_epi32(__m128i v) {
#if SIMD_SSE3 || SIMD_SSE4 || SIMD_AVX2 || SIMD_AVX512
    __m128i s = _mm_hadd_epi32(v, v);
    s = _mm_hadd_epi32(s, s);
    return _mm_cvtsi128_si32(s);
#elif SIMD_NEON
    return vaddvq_s32(v);
#else
    // Scalar fallback
    alignas(16) int32_t tmp[4];
    storeu128(tmp, v);
    return tmp[0] + tmp[1] + tmp[2] + tmp[3];
#endif
}

// 256-bit operations (AVX2+) -----------------------------------------

#if SIMD_AVX2 || SIMD_AVX512
inline __m256i loadu256(const void* ptr) {
    return _mm256_loadu_si256(static_cast<const __m256i*>(ptr));
}

inline void storeu256(void* ptr, __m256i v) {
    _mm256_storeu_si256(static_cast<__m256i*>(ptr), v);
}

inline __m256i madd_epi16_avx2(__m256i a, __m256i b) {
    return _mm256_madd_epi16(a, b);
}
#endif

} // namespace simd