#ifndef _CORE_COMMON_HEADER
#define _CORE_COMMON_HEADER

#if defined(PHS_PLATFORM_WINDOWS)
#  error "Windows platform not supported"
#endif

#ifndef _XOPEN_SOURCE
#  define _XOPEN_SOURCE
#endif

#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <sys/stat.h>
#include <string.h>
#include <termios.h>
#include <poll.h>

#ifdef __cplusplus
extern "C" {
#endif

static int writen(int fd, const void* buff, int n)
{
   ssize_t written = 0;
   ssize_t left = n;

   const char* ptr = (const char*)buff;

   while (left > 0) {
      if ((written = write(fd, ptr, left)) < 0) {
         if (left == n)
            return -1;
         else 
            break;
      } 
      else if (written == 0) {
         break;
      }

      ptr += written;
      left -= written;
   }

   return n - left;
}

#ifdef __cplusplus
}
#endif

#endif // _CORE_COMMON_HEADER
