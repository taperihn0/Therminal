#pragma once

#include "Common.hpp"
#include "Types.hpp"

/* It is rather slow, but useful for DEBUG builds
*  since we would like to flush every line to the output.
*/
#define THR_FLUSH_OUTPUT

namespace Thr
{

/* Main static logger.
*  Its static methods are used rather internally - 
*  external use of this class is done via macro defined below.
*/
class Log
{
public:
   Log()  = delete;
   ~Log() = delete;

   enum messageLevel : uint8_t
   {
      MSG_DEBUG = 1,
      MSG_INFO  = 2,
      MSG_ERROR = 3,
      MSG_FATAL = 4,
      MSG_MAX   = MSG_FATAL,
   };

   /* The arguments should be formatted in C manner.
   *  For now, only types are handled.
   */
   template <typename... Ts>
   static void message(messageLevel lvl, Ts&&... args);

   /* Prints given message as MSG_FATAL with full debug info (function declaration, file and line)
   */
   template <typename... Ts>
   static void frameInfoMessage(const char* file, uint line, const char* func, Ts&&... args);
private:
   template <size_t N = 1, typename... Ts>
   static void printQueue(std::string_view format, Ts&&... queue);

   static constexpr size_t           _ArgumentLimit = 16;

   static constexpr std::string_view _LevelStr[MSG_MAX] =
   {
      "[DEBUG]",
      "[INFO] ",
      "[ERROR]",
      "[FATAL]",
   };

   // ANSI escape sequences for colored output
   static constexpr std::string_view _ANSIColorReset         = "\033[0m";
   static constexpr std::string_view _ANSIColorBlack         = "\033[30m";
   static constexpr std::string_view _ANSIColorRed           = "\033[31m";
   static constexpr std::string_view _ANSIColorGreen         = "\033[32m";
   static constexpr std::string_view _ANSIColorYellow        = "\033[33m";
   static constexpr std::string_view _ANSIColorBlue          = "\033[34m";
   static constexpr std::string_view _ANSIColorMagenta       = "\033[35m";
   static constexpr std::string_view _ANSIColorCyan          = "\033[36m";
   static constexpr std::string_view _ANSIColorWhite         = "\033[37m";
   static constexpr std::string_view _ANSIColorBrightBlack   = "\033[90m";
   static constexpr std::string_view _ANSIColorBrightRed     = "\033[91m";
   static constexpr std::string_view _ANSIColorBrightGreen   = "\033[92m";
   static constexpr std::string_view _ANSIColorBrightYellow  = "\033[93m";
   static constexpr std::string_view _ANSIColorBrightBlue    = "\033[94m";
   static constexpr std::string_view _ANSIColorBrightMagenta = "\033[95m";
   static constexpr std::string_view _ANSIColorBrightCyan    = "\033[96m";
   static constexpr std::string_view _ANSIColorBrightWhite   = "\033[97m";

   static constexpr std::string_view _ANSIColor[MSG_MAX] =
   {
        _ANSIColorBrightGreen,
        _ANSIColorBrightBlue,
        _ANSIColorBrightYellow,
        _ANSIColorRed,
   };

   static inline bool _initialized = false;
};

// External logger API for clients
#define THR_LOG_DEBUG(...)      ::Thr::Log::message(::Thr::Log::MSG_DEBUG, __VA_ARGS__)
#define THR_LOG_INFO(...)       ::Thr::Log::message(::Thr::Log::MSG_INFO,  __VA_ARGS__)
#define THR_LOG_ERROR(...)      ::Thr::Log::message(::Thr::Log::MSG_ERROR, __VA_ARGS__)
#define THR_LOG_FATAL(...)      ::Thr::Log::message(::Thr::Log::MSG_FATAL, __VA_ARGS__)

#define THR_LOG_FATAL_FRAME_INFO(...) \
                                ::Thr::Log::frameInfoMessage(__FILE__, __LINE__, __PRETTY_FUNCTION__, __VA_ARGS__)

} // namespace Thr

#include "Log.inl"
