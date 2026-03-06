#include "Grid.hpp"
#include "io/OutputParser.hpp"
#include <algorithm>

namespace Thr
{

Grid::Grid(size_t ln_width)
    : _ln_width(ln_width)
    , _cursor_pos(0)
    , _ln_buf{}
{
    std::for_each(_ln_buf.begin(), _ln_buf.end(), [this](Line& l) {
        l.resize(_ln_width);
    });
}

void Grid::putChar(char32_t c, const EscapeState* state)
{
    if (c == U'\n') {
        _cursor_pos = (_cursor_pos + 1) % _BufSize;
    }

    Line& curr_ln = _ln_buf[_cursor_pos];
    curr_ln.putChar(c, state);

    /*
    const std::string_view sv(reinterpret_cast<const char*>(buf), n);
    _utf8_utf32.setBuf(sv);

    char32_t utf32;
    size_t nwrite;

    while (_utf8_utf32.nextUTF32(&utf32)) {
        Line& l = _scroll_buf.ln[_scroll_buf.cursor_pos];

        //updateStates(utf32);
        //nwrite = l.putChar(&utf32, 1);
        
        if (nwrite != 1) {
            _scroll_buf.cursor_pos = (_scroll_buf.cursor_pos + 1) % ScrollBuffer::BufSize;
        }
    }
    */
}

} // namespace Thr
