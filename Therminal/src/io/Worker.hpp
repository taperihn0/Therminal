#pragma once

#include "InputBuffer.hpp"
#include "InputTranslator.hpp"
#include "OutputBuffer.hpp"
#include <thread>

namespace Thr
{
   
class ThreadWorker
{
public:
   ThreadWorker(std::shared_ptr<InputRingBuffer> is, 
                std::shared_ptr<OutputBuffer> os,
                int ptym);
   void spawn();
   void stop();
private:
   void thrExecution();

   std::thread                      _thr;
   std::shared_ptr<InputRingBuffer> _is_buff;
   std::shared_ptr<OutputBuffer>    _os_buff;
   bool                             _running;
   int                              _ptymfd;
};

} // namespace Thr

