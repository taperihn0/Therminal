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
    std::for_each(_ln_buf.begin(), _ln_buf.end(), 
        [&](Line& l) {
            l.resize(_ln_width);
        }
    );
}

void Grid::putChar(char32_t c, const EscapeState* state)
{
    if (c == U'\n') {
        _cursor_pos = (_cursor_pos + 1) % _BufSize;
    }

    Line& curr_ln = _ln_buf[_cursor_pos];
    curr_ln.putChar(c, state);
}

Vec<Ptr<const Line>> Grid::getVisibleLines()// const
{
    // TODO
    _ln_buf[0].clear();
    _ln_buf[0].putChar('X', nullptr);
    return Vec<Ptr<const Line>>{ _ln_buf.data() };
}

} // namespace Thr
