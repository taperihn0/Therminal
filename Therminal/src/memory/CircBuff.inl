#include "CircBuff.hpp"

namespace Thr
{

template <typename T>
CircularBuff<T>::CircularBuff(size_t cnt)
   : _cnt(cnt)
   , _read_idx(0)
   , _write_idx(0)
{
   THR_HARD_ASSERT_LOG(_cnt > 0, "Invalid buffer size");
   THR_HARD_ASSERT_LOG(_cnt <= _CntLimit, "Too much elements queried");
   allocBuffer();
}

template <typename T>
CircularBuff<T>::~CircularBuff()
{
   alignedFree(_buff);
}

template <typename T>
THR_INLINE size_t CircularBuff<T>::getReadIdx() const
{
   return _read_idx;
}

template <typename T>
THR_INLINE size_t CircularBuff<T>::getWriteIdx() const
{
   return _write_idx;
}

template <typename T>
THR_INLINE bool CircularBuff<T>::isFull() const
{
   const size_t next_write_idx = (_write_idx + 1) % _cnt;
   return next_write_idx == _read_idx;
}

template <typename T>
THR_INLINE bool CircularBuff<T>::isEmpty() const
{
   return _write_idx == _read_idx;
}

template <typename T>
THR_INLINE void CircularBuff<T>::put(T&& el)
{
   advanceWriteIdx();
   
   T* const ptr = _buff;
   THR_HARD_ASSERT(ptr != nullptr);

   ptr[_write_idx] = std::move(el);
}

template <typename T>
THR_INLINE void CircularBuff<T>::put(const T& el)
{
   advanceWriteIdx();
   
   T* const ptr = _buff;
   THR_HARD_ASSERT(ptr != nullptr);

   ptr[_write_idx] = el;
}

template <typename T>
THR_INLINE T CircularBuff<T>::get()
{
   THR_HARD_ASSERT(_write_idx < _cnt);

   if (_write_idx == _read_idx) {
      THR_HARD_ASSERT_LOG(false, "Getting from empty buffer");
      return T();
   }

   T* const ptr = _buff;
   THR_HARD_ASSERT(ptr != nullptr);

   const T* const elem = ptr + _read_idx;
   _read_idx = (_read_idx + 1) % _cnt;

   return std::move(*elem);
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

   _buff = reinterpret_cast<T*>(m);
}

template <typename T>
THR_INLINE void CircularBuff<T>::advanceWriteIdx()
{
   _write_idx = (_write_idx + 1) % _cnt;

   if (_write_idx == _read_idx) { // if we're full, we need to shift beginning index
      _read_idx = (_read_idx + 1) % _cnt;
   }
}
   
} // namespace Thr
