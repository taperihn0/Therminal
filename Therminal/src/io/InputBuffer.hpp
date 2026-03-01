#pragma once

#include "InputTranslator.hpp"
#include "memory/CircBuff.hpp"
#include <mutex>

namespace Thr
{

/* Storage for incoming input of bytes
*  to the pseudo-terminal master.
*/
class InputRingBuffer 
{
public:
   InputRingBuffer() = delete;
   InputRingBuffer(size_t size);

   THR_INLINE char get();
   THR_INLINE void put(char data);

   THR_INLINE bool isFull() const;
   THR_INLINE bool isReady() const;
private:
   const size_t       _size;
   CircularBuff<char> _circ_buff;
   mutable std::mutex _mutex;
};

THR_INTERNAL InputRingBuffer::InputRingBuffer(size_t size)
   : _size(size)
   , _circ_buff(_size)
{}

THR_INLINE char InputRingBuffer::get()
{
   std::lock_guard<std::mutex> lock(_mutex);
   return _circ_buff.get();
}

THR_INLINE void InputRingBuffer::put(char data)
{
   std::lock_guard<std::mutex> lock(_mutex);
   _circ_buff.put(data);
}

THR_INLINE bool InputRingBuffer::isFull() const
{
   std::lock_guard<std::mutex> lock(_mutex);
   return _circ_buff.isFull();
}

THR_INLINE bool InputRingBuffer::isReady() const
{
   std::lock_guard<std::mutex> lock(_mutex);
   return !_circ_buff.isEmpty();
}

} // namespace Thr
