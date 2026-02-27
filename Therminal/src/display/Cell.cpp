#include "Cell.hpp"

namespace Thr
{

CellData::CellData(char_t c)
   : codepoint(c)
{}

Cell::Cell(char_t codepoint)
   : _s(codepoint)
{}
   
THR_INLINE char_t Cell::getCodepoint() const
{
   return _s.codepoint;
}

THR_INLINE void Cell::setCodepoint(char_t codepoint)
{
   _s.codepoint = codepoint;
}

} // namespace Thr
