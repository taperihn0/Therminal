#include "Line.hpp"
#include "io/OutputParser.hpp"

namespace Thr
{

Line::Line(size_t width)
    : _buf_size(width > _BufSizeLimit ? 0 : width)
{
    THR_HARD_ASSERT_LOG(width > 0, "Invalid width value");
    _ln.reserve(_buf_size);
}

size_t Line::getPrintableCount() const
{
    return _printable_cnt;
}

void Line::clear()
{
    _ln.clear();
    _printable_cnt = 0;
}

void Line::reserve(size_t width)
{
    THR_HARD_ASSERT_LOG(width <= _BufSizeLimit, "Invalid width value");
    _buf_size = width;
    _ln.reserve(_buf_size);
}

void Line::putChar(Char32 ch, const EscapeState* state)
{
    const Cell cell = { ch, {}, {} };

    if (ch.isPrintable())
        _printable_cnt += ch.getWidth();

    _ln.push_back(cell);
}

const Vec<Cell> Line::getVec() const
{
    return _ln;
}

} // namespace Thr
