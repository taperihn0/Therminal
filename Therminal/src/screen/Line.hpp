#pragma once

#include "Common.hpp"
#include "col/Color.hpp"
#include "char/Char.hpp"

namespace Thr
{
    
struct Cell
{
    char32_t ch;
    Color3u8 fg;
    Color3u8 bg;
};

struct EscapeState;

/* Represent single line of cells.
*/
class Line
{
public:
    Line() = default;
    Line(size_t width);
    
    void clear();

    /* Returns number of visible cells in 
    *  current line.
    */
    size_t getPrintableCount() const;

    void reserve(size_t width);
    void putChar(Char32 ch, const EscapeState* state);

    const Vec<Cell> getVec() const;

    void trimToNewLine();
private:
    static constexpr size_t _BufSizeLimit = 0x800;
    size_t                  _buf_size = 0;
    size_t                  _printable_cnt = 0;
    Vec<Cell>               _ln;
    size_t                  _ln_num;
};

} // namespace Thr
