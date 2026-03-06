#pragma once

#include "Common.hpp"

namespace Thr
{
 
template <typename T>
struct Color 
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

using Col8 = Color<uint8_t>;

} // namespace Thr
