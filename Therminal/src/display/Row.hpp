#pragma once

#include "memory/Memory.hpp"
#include "Cell.hpp"
#include <vector>

namespace Thr
{

class Row 
{
public:
   Row();
   Row(size_t cnt);

   static void setScreenWidth(int scr_width);

   /* Writes a given string 'line' to the data vector until
   *  new line or EOF is found.
   *  If successfully wrote all of the line contents,
   *  returns std::string::npos.
   *  Else, returns position to the first NL character.
   */
   size_t writeLine(const std::string& line);
private:
   void adjustVecSize(size_t ncnt);

   static int        _scr_width;

   size_t            _cnt;
   std::vector<Cell> _v;
   bool              _nl_term;
};

} // namespace Thr
