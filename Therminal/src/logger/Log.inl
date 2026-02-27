#pragma once

#include "Common.hpp"

namespace Thr
{

template <typename... Ts>
void Log::message(Log::messageLevel lvl, Ts&&... args)
{
   THR_STATIC_ASSERT(sizeof...(args) < _ArgumentLimit);

   const size_t msgval = static_cast<size_t>(lvl) - 1;

   THR_ASSERT(msgval < MSG_MAX);

   std::cout << _ANSIColor[msgval] << _LevelStr[msgval] << ": ";

   const std::string_view format = getNthArgument<0>(args...);
   printQueue(format, std::forward<Ts>(args)...);

   std::cout << _ANSIColorReset << '\n';

#if defined(THR_FLUSH_OUTPUT)
   std::cout << std::flush;
#endif
}

template <typename... Ts>
void Log::frameInfoMessage(const char* file, uint line, const char* func, Ts&&... args)
{
   message(MSG_FATAL, std::forward<Ts>(args)...);
   message(MSG_FATAL, "\tfile %s", file);
   message(MSG_FATAL, "\tline %d", line);
   message(MSG_FATAL, "\tfunc %s", func);
}

template <std::size_t N, typename... Ts>
THR_INLINE void Log::printQueue(std::string_view format, Ts&&... queue)
{
   const size_t pos = format.find("{}");
   
   {
      const size_t len = std::min(format.size(), pos);

      if (len == 0)
         return;

      const std::string_view stv(format.data(), len);
      std::cout << stv;
   }

   if constexpr (N < sizeof...(queue)) {
      std::cout << getNthArgument<N>(std::forward<Ts>(queue)...);
      const std::string_view stv(format.data() + pos + 2);
      printQueue<N + 1>(stv, std::forward<Ts>(queue)...);
   }
}

} // namespace Thr
