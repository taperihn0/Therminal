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
   ThreadWorker();
   ~ThreadWorker();

   void init(Ptr<InputRingBuffer> is, 
             Ptr<OutputBuffer> os,
             int ptym);
   
   void spawn();
   void stop();
private:
   void thrExecution();

   std::thread          _thr;
   Ptr<InputRingBuffer> _is_buff;
   Ptr<OutputBuffer>    _os_buff;
   bool                 _running;
   int                  _ptymfd;
};

} // namespace Thr

