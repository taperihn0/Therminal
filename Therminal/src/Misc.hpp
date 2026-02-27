#pragma once

#include "Defines.hpp"
#include "Assert.hpp"
#include "Types.hpp"
#include <GLFW/glfw3.h>
#include <cmath>

namespace Thr 
{

/* Miscellaneous Features */

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

template <std::size_t N, typename... Ts>
decltype(auto) getNthArgument(Ts&&... args)
{
   THR_STATIC_ASSERT(N < sizeof...(args));
   return std::get<N>(std::forward_as_tuple(std::forward<Ts>(args)...));
}

// Credits: https://github.com/glfw/glfw/issues/2036 by @wintertime
THR_INLINE bool glfwIsInitialized() {
   markUnused(glfwGetKeyScancode(GLFW_KEY_0));
   return glfwGetError(nullptr) != GLFW_NOT_INITIALIZED;
}

template <typename T>
THR_FORCEINLINE constexpr T sq(T x)
{
   return x * x;
}

} // namespace Thr
