#pragma once

#include "Common.hpp"

namespace Thr
{
 
template <typename T, size_t>
struct Color;

template <typename T>
struct Color<T, 3> 
{
	union
	{
		T x, r;
	};
	union
	{
		T y, g;
	};
	union
	{
		T z, b;
	};
};

template <typename T>
struct Color<T, 4> 
{
	union
	{
		T x, r;
	};
	union
	{
		T y, g;
	};
	union
	{
		T z, b;
	};
	union
	{
		T w, a;
	};
};

using Color3f    = Color<float,    3>;
using Color3d    = Color<double,   3>;
using Color3i    = Color<int, 	   3>;
using Color3s    = Color<short,    3>;
using Color3l    = Color<long,     3>;
using Color3ll   = Color<long,     3>;
using Color3i8   = Color<int8_t,   3>;
using Color3i16  = Color<int16_t,  3>;
using Color3i32  = Color<int32_t,  3>;
using Color3i64  = Color<int64_t,  3>;
using Color3u8   = Color<uint8_t,  3>;
using Color3u16  = Color<uint16_t, 3>;
using Color3u32  = Color<uint32_t, 3>;
using Color3u64  = Color<uint64_t, 3>;

using Color4f    = Color<float,    4>;
using Color4d    = Color<double,   4>;
using Color4i    = Color<int, 	   4>;
using Color4s    = Color<short,    4>;
using Color4l    = Color<long,     4>;
using Color4ll   = Color<long,     4>;
using Color4i8   = Color<int8_t,   4>;
using Color4i16  = Color<int16_t,  4>;
using Color4i32  = Color<int32_t,  4>;
using Color4i64  = Color<int64_t,  4>;
using Color4u8   = Color<uint8_t,  4>;
using Color4u16  = Color<uint16_t, 4>;
using Color4u32  = Color<uint32_t, 4>;
using Color4u64  = Color<uint64_t, 4>;

} // namespace Thr
