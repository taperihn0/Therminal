#pragma once

#include "Defines.hpp"
#include <iostream>

#ifdef THR_DEBUG
#	define THR_RUNTIME_ASSERT_ENABLE
#endif

#ifdef THR_PLATFORM_WINDOWS
#	define THR_DEBUG_BREAK() __debugbreak()
#else
#	define THR_DEBUG_BREAK() __builtin_trap()
#endif

THR_INLINE bool __thrfailedAssertion(const char* file, const char* text, int line)
{
	std::cerr << "\nAssertion failed: " << text << " - file " << file << " line " << line << std::endl;
	THR_DEBUG_BREAK();
	exit(-1);
	return true;
}

#ifdef THR_RUNTIME_ASSERT_ENABLE
#	define THR_ASSERT(s)						(void)((s) or ::__thrfailedAssertion(__FILE__, "Runtime assertion failed", __LINE__))
#	define THR_ASSERT_LOG(s, msg)			(void)((s) or ::__thrfailedAssertion(__FILE__, msg, __LINE__))
#else // Assertions not specified.
#	define THR_ASSERT(s) 
#	define THR_ASSERT_LOG(s, msg) 
#endif // THR_RUNTIME_ASSERT_ENABLE

// Universal assertion that cannot be turned off for now
#define THR_HARD_ASSERT(s)					(void)((s) or ::__thrfailedAssertion(__FILE__, "Runtime assertion failed", __LINE__))
#define THR_HARD_ASSERT_LOG(s, msg) 	(void)((s) or ::__thrfailedAssertion(__FILE__, msg, __LINE__))

// Static assertions doesn't influence runtime performance, so keep them as they are.
#define THR_STATIC_ASSERT(s)				static_assert(s)
#define THR_STATIC_ASSERT_LOG(s, msg)	static_assert(s, msg)
