#pragma once

#include "memory/Memory.hpp"

namespace Thr
{

class Row 
{
public:
   Row() = default;
   Row(size_t cnt);

private:
   size_t _cnt;
};

} // namespace Thr
