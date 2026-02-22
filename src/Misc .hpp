#pragma once

#include "Defines.hpp"
#include "Assert.hpp"

namespace Thr 
{

template <typename T>
static void markUnused(T&&) {}

template <typename T>
constexpr THR_INLINE T maxof()
{
   THR_STATIC_ASSERT(is_numeric<T>);
   return std::numeric_limits<T>::max();
}

template <typename T>
constexpr THR_INLINE T minof()
{
   THR_STATIC_ASSERT(is_numeric<T>);
   return std::numeric_limits<T>::lowest();
}

} // namespace Thr
