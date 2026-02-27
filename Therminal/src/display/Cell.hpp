#pragma once

#include "encode/Char.hpp"

namespace Thr
{

struct CellData
{
   CellData() = default;
   CellData(char_t c);

   char_t codepoint;
};

class Cell 
{
public:
   Cell() = default;
   Cell(char_t codepoint);

   THR_INLINE char_t getCodepoint() const;
   THR_INLINE void   setCodepoint(char_t codepoint);
private:
   CellData _s;
};

} // namespace Thr
