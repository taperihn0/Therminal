#pragma once

#include "Common.hpp"

/* Instruction sets are iterative, that is
*  they are build on top of each other.
*/

#ifndef THR_FORCE_PURE

#if    defined(THR_SIMD_AVX512) \
    || defined(THR_SIMD_AVX2)   \
    || defined(THR_SIMD_AVX) 
#include <immintrin.h>   // AVX, AVX2, AVX-512, BMI, FMA, etc.

#elif defined(THR_SIMD_SSE4_2)
#include <nmmintrin.h>   // SSE4.2

#elif defined(THR_SIMD_SSE4_1)
#include <smmintrin.h>   // SSE4.1

#elif defined(THR_SIMD_SSSE3)
#include <tmmintrin.h>   // SSSE3

#elif defined(THR_SIMD_SSE3)
#include <pmmintrin.h>   // SSE3

#elif defined(THR_SIMD_SSE2)
#include <emmintrin.h>   // SSE2

#elif defined(THR_SIMD_SEE)
#include <xmmintrin.h>   // SSE

#endif

#endif // THR_FORCE_PURE
