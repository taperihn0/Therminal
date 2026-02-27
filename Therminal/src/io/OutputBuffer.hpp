#pragma once

#include "InputTranslator.hpp"
#include "memory/CircBuff.hpp"
#include <mutex>

namespace Thr
{
   
class OutputBuffer 
{
public:
   OutputBuffer();
   
   THR_INLINE void swap();
   THR_INLINE void write(std::unique_ptr<byte[]>&& m);
   THR_INLINE void read(Ptr<std::unique_ptr<byte[]>>& m);
private:
   static constexpr size_t      _WriteSideBuff = 0;
   static constexpr size_t      _ReadSideBuff  = 1;
   std::unique_ptr<byte[]>      _flip_buff_0;
   std::unique_ptr<byte[]>      _flip_buff_1;
   Ptr<std::unique_ptr<byte[]>> _buff_ptr[2];
   std::mutex                   _read_mutex;
   std::mutex                   _write_mutex;
};

THR_INTERNAL OutputBuffer::OutputBuffer()
   : _flip_buff_0(nullptr)
   , _flip_buff_1(nullptr)
   , _buff_ptr{ std::addressof(_flip_buff_0), std::addressof(_flip_buff_1) }
{}

THR_INLINE void OutputBuffer::swap()
{
   std::scoped_lock lock { _read_mutex, _write_mutex };
   std::swap(*_buff_ptr[_WriteSideBuff], *_buff_ptr[_ReadSideBuff]);
   *_buff_ptr[_WriteSideBuff] = nullptr;
}
   
THR_INLINE void OutputBuffer::write(std::unique_ptr<byte[]>&& m) 
{
   std::lock_guard<std::mutex> lock(_write_mutex);
   Ptr<std::unique_ptr<byte[]>> write_buff = _buff_ptr[_WriteSideBuff];
   *write_buff = std::move(m);
}

THR_INLINE void OutputBuffer::read(Ptr<std::unique_ptr<byte[]>>& m) 
{
   std::lock_guard<std::mutex> lock(_read_mutex);
   Ptr<std::unique_ptr<byte[]>> read_buff = _buff_ptr[_ReadSideBuff];
   m = std::move(read_buff);
}

} // namespace Thr
