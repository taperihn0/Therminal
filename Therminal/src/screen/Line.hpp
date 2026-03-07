#pragma once

#include "Common.hpp"
#include "Color.hpp"

namespace Thr
{
    
struct Cell
{
    char32_t ch;
    Col8     fg;
    Col8     bg;
};

struct EscapeState;

class Line
{
public:
    Line() = default;
    Line(size_t width);
    
    void resize(size_t width);
    void putChar(char32_t ch, const EscapeState* state);

    const Vec<Cell> getCellLine() const;
private:
    static constexpr size_t _WidthLimit = 0x800;
    size_t                  _width = 0;
    Vec<Cell>               _ln; 
};

} // namespace Thr
