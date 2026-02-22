#pragma once

#if  __cplusplus >= 202002L
#  define THR_STANDARD_20
#  define THR_STANDARD_20
#elif __cplusplus >= 201702L
#  define THR_STANDARD_17
#  define THR_STANDARD_17
#else
#  error "C++ standard not defined"
#endif

#if defined(_MSC_VER)
#  define THR_INLINE              inline 
#  define THR_FORCEINLINE		    __forceinline
#  define THR_LAMBDA_FORCEINLINE  [[msvc::forceinline]] 
#  define THR_NODISCARD           [[nodiscard]]
#  define THR_RESTRICT            __restrict
#  define THR_DECLARE             extern
#  define THR_INTERNAL            inline
#elif defined(__GNUG__)
#  define THR_INLINE			       inline
#  define THR_FORCEINLINE		    __attribute__((always_inline)) inline
#  define THR_LAMBDA_FORCEINLINE  __attribute__((always_inline))
#  define THR_RESTRICT            __restrict__
#  define THR_NODISCARD           [[nodiscard]]
#  define THR_DECLARE             
#  define THR_INTERNAL            inline
#endif

#define THR_NOEXCEPT              noexcept
#define THR_NORETURN              [[noreturn]]
#define THR_UNUSED                [[maybe_unused]]
#define THR_FALLTHROUGH           [[fallthrough]]

#if defined(THR_STANDARD_20)
#  define THR_UNLIKELY            [[unlikely]]
#  define THR_LIKELY              [[likely]]
#else
#  define THR_UNLIKELY
#  define THR_LIKELY
#endif
