#pragma once

#include "Memory.hpp"
#include "Misc.hpp"

namespace Thr
{

template <typename T, typename Invocable = std::function<void(T&)>,
		  std::enable_if_t<std::is_invocable_v<Invocable, T&>, bool> = true>
class CircularBuff 
{
public:
	CircularBuff() = delete;
	CircularBuff(size_t cnt);
	CircularBuff(size_t cnt, Invocable on_delete);
	CircularBuff(const CircularBuff&) = delete;
	CircularBuff(CircularBuff&&) = delete;
	~CircularBuff();

	CircularBuff& operator=(const CircularBuff&) = delete;
	CircularBuff& operator=(CircularBuff&&) = delete;

	THR_INLINE size_t getReadIdx() const;
	THR_INLINE size_t getWriteIdx() const;

	THR_INLINE void put(T&& el);
	THR_INLINE void put(const T& el);
	THR_INLINE T&& get();
	THR_INLINE const T& front() const;
	THR_INLINE T& front();
	THR_INLINE const T& back() const;
	THR_INLINE T& back();
	THR_INLINE const T& getIthElement(size_t i) const;
	THR_INLINE T& getIthElement(size_t i);

	THR_INLINE bool isFull() const;
	THR_INLINE bool isEmpty() const;

	THR_INLINE size_t getSize() const;
private:
	static constexpr size_t _CntLimit = 0x10000;

	void allocBuffer();
	void deleteFrontOnFull();

	size_t 	  _cnt;
	size_t 	  _read_idx;
	size_t 	  _write_idx;
	size_t 	  _el_cnt;
	bool   	  _full;
	T*     	  _buff;
	Invocable _on_delete;
};

} // namespace Thr

#include "CircBuff.inl"
