#include "simd.h"

namespace simd {

#if SIMD_AVX2 || SIMD_SSE2
    inline __m128i loadu128(const void* ptr) {
        return _mm_loadu_si128(static_cast<const __m128i*>(ptr));
    }

    inline void storeu128(void* ptr, __m128i v) {
        _mm_storeu_si128(static_cast<__m128i*>(ptr), v);
    }

    inline __m128i add_epi16(__m128i a, __m128i b) {
        return _mm_add_epi16(a, b);
    }

    inline __m128i madd_epi16(__m128i a, __m128i b) {
        return _mm_madd_epi16(a, b);
    }

    inline int hsum_epi32(__m128i v) {
        // More efficient horizontal sum using SSE3
        __m128i shuf = _mm_shuffle_epi32(v, _MM_SHUFFLE(2,3,0,1));
        __m128i sums = _mm_add_epi32(v, shuf);
        shuf = _mm_movehdup_epi32(sums);
        sums = _mm_add_epi32(sums, shuf);
        return _mm_cvtsi128_si32(sums);
    }

#elif SIMD_NEON
    inline int16x8_t loadu128(const void* ptr) {
        return vld1q_s16(static_cast<const int16_t*>(ptr));
    }

    inline void storeu128(void* ptr, int16x8_t v) {
        vst1q_s16(static_cast<int16_t*>(ptr), v);
    }

    inline int16x8_t add_epi16(int16x8_t a, int16x8_t b) {
        return vaddq_s16(a, b);
    }

    inline int32x4_t madd_epi16(int16x8_t a, int16x8_t b) {
        // Full 8x16-bit → 4x32-bit multiply-add
        return vmlal_s16(vmull_s16(vget_low_s16(a), vget_low_s16(b)),
                        vget_high_s16(a), vget_high_s16(b));
    }

    inline int hsum_epi32(int32x4_t v) {
        // Faster than multiple vadds
        int32x2_t sum = vadd_s32(vget_low_s32(v), vget_high_s32(v));
        return vget_lane_s32(vpadd_s32(sum, sum), 0);
    }

#else // Scalar fallback
    struct alignas(16) simd128 {
        int16_t data[8];
    };

    inline simd128 loadu128(const void* ptr) {
        simd128 val;
        std::memcpy(&val, ptr, sizeof(val));
        return val;
    }

    inline void storeu128(void* ptr, simd128 v) {
        std::memcpy(ptr, &v, sizeof(v));
    }

    inline simd128 add_epi16(simd128 a, simd128 b) {
        simd128 result;
        for (int i = 0; i < 8; ++i)
            result.data[i] = a.data[i] + b.data[i];
        return result;
    }

    inline int madd_epi16(simd128 a, simd128 b) {
        int sum = 0;
        for (int i = 0; i < 8; i += 2)
            sum += a.data[i] * b.data[i] + a.data[i+1] * b.data[i+1];
        return sum;
    }

    inline int hsum_epi32(int v) {
        return v;
    }

#endif

} // namespace simd