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
   ~CircularBuff();

   THR_INLINE size_t getReadIdx() const;
   THR_INLINE size_t getWriteIdx() const;

   THR_INLINE void put(T&& el);
   THR_INLINE void put(const T& el);
   THR_INLINE T get();

   THR_INLINE bool isFull() const;
   THR_INLINE bool isEmpty() const;
private:
   static constexpr size_t _CntLimit = 0x10000;

   void allocBuffer();
   void advanceWriteIdx();

   size_t             _cnt;
   size_t             _read_idx;
   size_t             _write_idx;
   bool               _full;
   T*                 _buff;
};

} // namespace Thr

#include "CircBuff.inl"
