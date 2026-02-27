#include "Worker.hpp"
#include "core/core_common.h"

namespace Thr
{
   
ThreadWorker::ThreadWorker(std::shared_ptr<InputRingBuffer> is, 
                           std::shared_ptr<OutputBuffer> os,
                           int ptym)
   : _is_buff(is)
   , _os_buff(os)
   , _running(false)
   , _ptymfd(ptym)
{}

void ThreadWorker::spawn()
{
   _running = true;
   _thr = std::thread(
      [this]() {
         this->thrExecution();
      });
}

void ThreadWorker::stop()
{
   if (_thr.joinable()) {
      _running = false;
      _thr.join();
   }
}

void ThreadWorker::thrExecution()
{
   int nread;

   static constexpr size_t BuffSize = 512;
   char buff[BuffSize];

   struct pollfd pfd;
   pfd.fd = _ptymfd;
   pfd.events = POLLIN;

   static constexpr int Timeout = 5;

   while (_running) {
      if (poll(&pfd, 1, Timeout) <= 0)
         break;

      if (pfd.revents & (POLLIN | POLLHUP | POLLERR)) { /* copies ptym to output */
         if ((nread = read(pfd.fd, buff, sizeof(buff))) <= 0)
            break;
         
         // TODO: avoid small allocations here.
         // Implement pre-allocated buffer inside output segment and 
         // copy the data there.
         auto alloc = std::make_unique<byte[]>(nread);
         memCpy(reinterpret_cast<void*>(alloc.get()), buff, nread);

         _os_buff->write(std::move(alloc));
      }

      /* copies input to ptym */

      if (_is_buff->isReady()) {
         const in_char_t ch = _is_buff->get();
         
         // TODO: handle EOF

         if (writen(pfd.fd, &ch, sizeof(ch)) != sizeof(ch))
            THR_LOG_ERROR("writen error to master pty");
         }
   }

   /*
   *  We should terminate.
   */
}

} // namespace Thr
