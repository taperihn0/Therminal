#include "Row.hpp"

namespace Thr
{

int Row::_scr_width = -1;

Row::Row()
   : _cnt(0)
   , _v()
   , _nl_term(false)
{}

Row::Row(size_t cnt)
   : _cnt(cnt)
   , _v()
   , _nl_term(false)
{
   adjustVecSize(cnt);
}

void Row::setScreenWidth(int scr_width)
{
   _scr_width = scr_width;
}

/* Writes a given string 'line' to the data vector until
*  new line or EOF is found.
*  If successfully wrote all of the line contents,
*  returns std::string::npos.
*  Else, returns position to the first NL character.
*/
THR_INLINE size_t Row::writeLine(const std::string& line)
{
   const size_t nl = line.find('\n');
   const size_t sz = line.size();
   const size_t nwrite = std::min(nl + 1, sz);

   THR_HARD_ASSERT(_v.capacity() >= nwrite);

   _v.resize(nwrite);

   for (uint i = 0; i < nwrite; i++) {
      const char c = line[i];
      _v.at(i) = Cell(c);
   }

   return nl;
}

void Row::adjustVecSize(size_t ncnt)
{
   _v.reserve(ncnt);
}

} // namespace Thr
