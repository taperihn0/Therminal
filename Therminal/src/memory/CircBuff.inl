#include "CircBuff.hpp"

namespace Thr
{

template <typename T>
CircularBuff<T>::CircularBuff(size_t cnt)
   : _cnt(cnt)
   , _beg(0)
   , _end(0)
   , _full(false)
{
   allocBuffer();
}

template <typename T>
THR_INLINE size_t CircularBuff<T>::getBegin() const
{
   return _beg;
}

template <typename T>
THR_INLINE size_t CircularBuff<T>::getCurrentIdx() const
{
   return _end;
}

template <typename T>
THR_INLINE void CircularBuff<T>::put(T&& el)
{
   const size_t write_idx = (_end + 1) % _cnt;

   if (_end + 1 == _cnt) { // reached end of the buffer
      _full = true;
   }

   if (_full) { // if we're full, we need to shift beginning
      _beg = (_beg + 1) % _cnt;
   }
   
   T* const ptr = _buff.get();
   
   THR_HARD_ASSERT(ptr != nullptr);
   THR_HARD_ASSERT(write_idx < _cnt);

   ptr[write_idx] = std::forward<T>(el);
   _end = write_idx;
}

template <typename T>
THR_INLINE const T& CircularBuff<T>::get() const
{
   THR_HARD_ASSERT(_end < _cnt);
   const T* const ptr = _buff.get();
   THR_HARD_ASSERT(ptr != nullptr);
   return ptr[_end];
}

template <typename T>
THR_INLINE T& CircularBuff<T>::get()
{
   THR_HARD_ASSERT(_end < _cnt);
   T* const ptr = _buff.get();
   THR_HARD_ASSERT(ptr != nullptr);
   return ptr[_end];
}

template <typename T>
void CircularBuff<T>::allocBuffer()
{
   THR_ASSERT(_cnt > 0);
 
   static constexpr size_t ElemSize = sizeof(T);
   const size_t buff_size = _cnt * ElemSize;

   void* m = alignedMalloc(buff_size, CachelineSize);

   if (m == nullptr) {
      THR_LOG_ERROR("Failed to align-malloc a memory");
      return;
   }

   _buff = std::unique_ptr(m);
}
   
} // namespace Thr
