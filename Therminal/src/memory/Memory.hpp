#pragma once

#if defined(THR_PLATFORM_WINDOWS)
#  include <windows.h>
#else
#  include <unistd.h>
#endif

#include "Simd.hpp"
#include "logger/Log.hpp"

#include <memory>
#include <cstring>

namespace Thr
{

/* We assume cache-line is fixed and it's equal to 64 bytes */
static constexpr size_t CachelineSize = 64;

THR_INLINE size_t getPageSize() 
{
#if defined(THR_PLATFORM_WINDOWS)
   SYSTEM_INFO si;
	GetSystemInfo(&si);
   return static_cast<size_t>(si.dwPageSize);
#else
   return static_cast<size_t>(sysconf(_SC_PAGESIZE));
#endif
}

THR_INTERNAL THR_FORCEINLINE void memSet(void* s, int c, size_t n)
{
	std::memset(s, c, n);
}

THR_INTERNAL THR_FORCEINLINE void memCpy(void* d, const void* s, size_t n)
{
	std::memcpy(d, s, n);
}

THR_INTERNAL THR_FORCEINLINE void prefetch(const void* addr) 
{
#if defined(_MSC_VER) || defined(_INTEL_COMPILER)
	_mm_prefetch(reinterpret_cast<const char*>(addr), _MM_HINT_T2);
#else
	__builtin_prefetch(addr, 1, 2);
#endif
}

THR_INLINE void* alignedMemset(void* dst, int ch, size_t cnt) 
{
	byte* d = reinterpret_cast<byte*>(dst);

#if defined(THR_SIMD_AVX512)
	ASSERT(cnt % AlignmentBound == 0, "Size must be a multiple of 64");
	__m512i pack8i_ch = _mm512_set1_epi8(ch);

	for (size_t i = 0; i < cnt; i += 64) {
		_mm512_store_si512(reinterpret_cast<__m512*>(d + i), pack8i_ch);
	}

#elif defined(THR_SIMD_AVX2)
	ASSERT(cnt % AlignmentBound == 0, "Size must be a multiple of 32");
	__m256i pack4i_ch = _mm256_set1_epi8(ch);

	for (size_t i = 0; i < cnt; i += 32) {
		_mm256_store_si256(reinterpret_cast<__m256i*>(d + i), pack4i_ch);
	}

#elif defined(THR_SIMD_SSE2)
	ASSERT(cnt % AlignmentBound == 0, "Size must be a multiple of 16");
	__m128i pack2i_ch = _mm_set1_epi8(ch);

	for (size_t i = 0; i < cnt; i += 16) {
		_mm_store_si128(reinterpret_cast<__m128i*>(d + i), pack2i_ch);
	}

#else
	markUnused(d);
	std::memset(dst, ch, cnt);
#endif

	return dst;
}

THR_INLINE void* alignedMalloc(size_t size, size_t alignment) 
{
#if defined (_MSC_VER)
	void* m = _aligned_malloc(size, alignment);
#else
   THR_ASSERT_LOG(size % alignment == 0, "Size must be multiple of alignment for some platforms");
	void* m = std::aligned_alloc(alignment, size);
#endif
	THR_ASSERT_LOG(m, "Failed to allocate memory");
	return m;
}

THR_INLINE void* pageAlignedMalloc(size_t size) 
{
   static const size_t page_size = getPageSize();
   const size_t page_aligned_size = (size + page_size - 1) & ~(page_size - 1);

   void* m = alignedMalloc(page_aligned_size, page_size);

   if (m == nullptr)
   {
      THR_LOG_ERROR("Failed to page-alloc a new memory page");
   }

   return m;
}

THR_INLINE void alignedFree(void* block) 
{
#if defined(_MSC_VER)
	_aligned_free(block);
#else
	std::free(block);
#endif
}

} // namespace Thr
