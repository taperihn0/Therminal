#include "Line.hpp"
#include "io/OutputParser.hpp"

namespace Thr
{

Line::Line(size_t width)
    : _width(width > _WidthLimit ? 0 : width)
{
    THR_HARD_ASSERT_LOG(width > 0, "Invalid width value");
    _ln.reserve(_width);
}

void Line::resize(size_t width)
{
    THR_HARD_ASSERT_LOG(width <= _WidthLimit, "Invalid width value");
    _width = width;
    _ln.reserve(_width);
}

void Line::putChar(char32_t ch, const EscapeState* state)
{
    markUnused(state);
    const Cell cell = { ch, {}, {} };
    _ln.push_back(cell);
}

const std::vector<Cell> Line::getCellLine() const
{
    return _ln;
}

} // namespace Thr
