#pragma once

#include "Defines.hpp"
#include "Assert.hpp"
#include <cstdint>
#include <limits>
#include <uchar.h>
#include <type_traits>

namespace Thr
{

template <typename T1, typename T2>
constexpr bool is_same = std::is_same_v<std::remove_volatile_t<std::remove_const_t<std::remove_reference_t<T1>>>,
                                        std::remove_volatile_t<std::remove_const_t<std::remove_reference_t<T2>>>>;

template <typename T>
constexpr bool is_integral = std::is_integral_v<T>;

template <typename T>
constexpr bool is_real = std::is_floating_point_v<T>;

template <typename T>
constexpr bool is_numeric = (is_real<T> or is_integral<T>);

// Assert IEEE Standard is implemented
THR_STATIC_ASSERT(std::numeric_limits<float>::is_iec559);

// IEEE Standard is provided
#define THR_IEEE_STANDARD 1

// As IEEE is checked, these types exactly maps their size.
using float32_t  = float;
using float64_t  = double;
using floatmax_t = long double;

// Define some handy aliases
using byte      = uint8_t;
using uchar     = unsigned char;
using ll        = long long;
using ull       = unsigned long long;
using uint      = unsigned int;
using intptr_t  = size_t;
using uintmax_t = ull;

static constexpr uint8_t operator"" _u8(uintmax_t v) noexcept
{
   return static_cast<uint8_t>(v);
}

static constexpr uint16_t operator"" _u16(uintmax_t v) noexcept
{
   return static_cast<uint16_t>(v);
}

static constexpr uint32_t operator"" _u32(uintmax_t v) noexcept
{
   return static_cast<uint32_t>(v);
}

static constexpr uint64_t operator"" _u64(uintmax_t v) noexcept
{
   return static_cast<uint64_t>(v);
}

static constexpr int8_t operator"" _i8(uintmax_t v) noexcept
{
   return static_cast<int8_t>(v);
}

static constexpr int16_t operator"" _i16(uintmax_t v) noexcept
{
   return static_cast<int16_t>(v);
}

static constexpr int32_t operator"" _i32(uintmax_t v) noexcept
{
   return static_cast<int32_t>(v);
}

static constexpr int64_t operator"" _i64(uintmax_t v) noexcept
{
   return static_cast<int64_t>(v);
}

static constexpr float32_t operator"" _f32(floatmax_t v) noexcept
{
   return static_cast<float32_t>(v);
}

static constexpr float64_t operator"" _f64(floatmax_t v) noexcept
{
   return static_cast<float64_t>(v);
}

} // namespace Thr
