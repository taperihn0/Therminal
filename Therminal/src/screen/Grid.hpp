#pragma once

#include "Line.hpp"
#include "memory/CircBuff.hpp"
#include "io/OutputTranslator.hpp"
#include "Line.hpp"
#include <array>

namespace Thr
{

class Grid
{
public:
    Grid(size_t ln_width);
    void putChar(char32_t c, const EscapeState* state);
private:
    static constexpr size_t    _BufSize = 0x10000;
    const size_t               _ln_width;
    OutputStreamTransl         _utf8_utf32;
    size_t                     _cursor_pos;
    std::array<Line, _BufSize> _ln_buf;
};

} // namespace Thr
