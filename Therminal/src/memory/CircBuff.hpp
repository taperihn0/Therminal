#pragma once

#include "Memory.hpp"

namespace Thr
{

template <typename T>
class CircularBuff 
{
public:
   CircularBuff() = delete;
   CircularBuff(size_t size);

   THR_INLINE size_t getBegin() const;
   THR_INLINE size_t getCurrentIdx() const;

   THR_INLINE void put(T&& el);
   THR_INLINE const T& get() const;
   THR_INLINE T& get();
private:
   void allocBuffer();

   size_t             _cnt;
   size_t             _beg;
   size_t             _end;
   bool               _full;
   std::unique_ptr<T> _buff;
};

} // namespace Thr

#include "CircBuff.inl"
