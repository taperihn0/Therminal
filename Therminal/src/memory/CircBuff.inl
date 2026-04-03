#include "CircBuff.hpp"

namespace Thr
{

template <typename T, typename Invocable, 
		  std::enable_if_t<std::is_invocable_v<Invocable, T&>, bool> En>
CircularBuff<T, Invocable, En>::CircularBuff(size_t cnt)
	: CircularBuff(cnt, doNothing<void, T&>)
{}

template <typename T, typename Invocable, 
		  std::enable_if_t<std::is_invocable_v<Invocable, T&>, bool> En>
CircularBuff<T, Invocable, En>::CircularBuff(size_t cnt, Invocable on_delete)
	: _cnt(cnt)
	, _read_idx(0)
	, _write_idx(0)
	, _el_cnt(0)
	, _on_delete(on_delete)
{
	THR_HARD_ASSERT_LOG(_cnt > 0, "Invalid buffer size");
	THR_HARD_ASSERT_LOG(_cnt <= _CntLimit, "Too much elements queried");
	allocBuffer();
}

template <typename T, typename Invocable, 
		  std::enable_if_t<std::is_invocable_v<Invocable, T&>, bool> En>
CircularBuff<T, Invocable, En>::~CircularBuff()
{
	delete[] _buff;
}

template <typename T, typename Invocable, 
		  std::enable_if_t<std::is_invocable_v<Invocable, T&>, bool> En>
THR_INLINE size_t CircularBuff<T, Invocable, En>::getReadIdx() const
{
	return _read_idx;
}

template <typename T, typename Invocable, 
		  std::enable_if_t<std::is_invocable_v<Invocable, T&>, bool> En>
THR_INLINE size_t CircularBuff<T, Invocable, En>::getWriteIdx() const
{
	return _write_idx;
}

template <typename T, typename Invocable, 
		  std::enable_if_t<std::is_invocable_v<Invocable, T&>, bool> En>
THR_INLINE bool CircularBuff<T, Invocable, En>::isFull() const
{
	return getSize() == _cnt;
}

template <typename T, typename Invocable, 
		  std::enable_if_t<std::is_invocable_v<Invocable, T&>, bool> En>
THR_INLINE bool CircularBuff<T, Invocable, En>::isEmpty() const
{
	return getSize() == 0;
}

template <typename T, typename Invocable, 
		  std::enable_if_t<std::is_invocable_v<Invocable, T&>, bool> En>
THR_INLINE void CircularBuff<T, Invocable, En>::put(T&& el)
{
	THR_HARD_ASSERT(_buff != nullptr);

	if (isFull())
		deleteFrontOnFull();

	_buff[_write_idx] = std::move(el);
	_write_idx = (_write_idx + 1) % _cnt;
	_el_cnt++;
}

template <typename T, typename Invocable, 
		  std::enable_if_t<std::is_invocable_v<Invocable, T&>, bool> En>
THR_INLINE void CircularBuff<T, Invocable, En>::put(const T& el)
{
	THR_HARD_ASSERT(_buff != nullptr);

	if (isFull())
		deleteFrontOnFull();

	_buff[_write_idx] = el;
	_write_idx = (_write_idx + 1) % _cnt;
	_el_cnt++;
}

template <typename T, typename Invocable, 
		  std::enable_if_t<std::is_invocable_v<Invocable, T&>, bool> En>
THR_INLINE T&& CircularBuff<T, Invocable, En>::get()
{
	T& first = front();
	_read_idx = (_read_idx + 1) % _cnt;
	_el_cnt--;
	return std::move(first);
}

template <typename T, typename Invocable, 
		  std::enable_if_t<std::is_invocable_v<Invocable, T&>, bool> En>
THR_INLINE const T& CircularBuff<T, Invocable, En>::front() const
{
	return getIthElement(0);
}

template <typename T, typename Invocable, 
		  std::enable_if_t<std::is_invocable_v<Invocable, T&>, bool> En>
THR_INLINE T& CircularBuff<T, Invocable, En>::front() 
{
	return getIthElement(0);
}

template <typename T, typename Invocable, 
		  std::enable_if_t<std::is_invocable_v<Invocable, T&>, bool> En>
THR_INLINE const T& CircularBuff<T, Invocable, En>::back() const
{
	return getIthElement(getSize() - 1);
}

template <typename T, typename Invocable, 
		  std::enable_if_t<std::is_invocable_v<Invocable, T&>, bool> En>
THR_INLINE T& CircularBuff<T, Invocable, En>::back()
{
	return getIthElement(getSize() - 1);
}

template <typename T, typename Invocable, 
		  std::enable_if_t<std::is_invocable_v<Invocable, T&>, bool> En>
THR_INLINE const T& CircularBuff<T, Invocable, En>::getIthElement(size_t i) const
{
	THR_HARD_ASSERT_LOG(i < getSize(), "Invalid access in CircularBuff");
	THR_HARD_ASSERT(_buff != nullptr);

	const T* elem = _buff + (_read_idx + i) % _cnt;
	return *elem;
}

template <typename T, typename Invocable, 
		  std::enable_if_t<std::is_invocable_v<Invocable, T&>, bool> En>
THR_INLINE T& CircularBuff<T, Invocable, En>::getIthElement(size_t i)
{
	THR_HARD_ASSERT_LOG(i < getSize(), "Invalid access in CircularBuff");
	THR_HARD_ASSERT(_buff != nullptr);

	T* elem = _buff + (_read_idx + i) % _cnt;
	return *elem;
}

template <typename T, typename Invocable, 
		  std::enable_if_t<std::is_invocable_v<Invocable, T&>, bool> En>
void CircularBuff<T, Invocable, En>::allocBuffer()
{
	THR_ASSERT(_cnt > 0);

	void* m;

	try {
		m = new T[_cnt];
	}
	catch (...) {
		THR_LOG_ERROR("Failed to 'new' and initialize CircularBuffer memory");
		return;
	}

	THR_HARD_ASSERT(m != nullptr);

	_buff = reinterpret_cast<T*>(m);
}
	
template <typename T, typename Invocable, 
		  std::enable_if_t<std::is_invocable_v<Invocable, T&>, bool> En>
THR_INLINE void CircularBuff<T, Invocable, En>::deleteFrontOnFull()
{
	if (_write_idx == _read_idx) { // if we're full, we need to shift beginning index
		T* del_elem = _buff + _read_idx;
		_on_delete(*del_elem);
		_read_idx = (_read_idx + 1) % _cnt;
		_el_cnt--;
	}
}

template <typename T, typename Invocable, 
		  std::enable_if_t<std::is_invocable_v<Invocable, T&>, bool> En>
THR_INLINE size_t CircularBuff<T, Invocable, En>::getSize() const
{
	return _el_cnt;
}

} // namespace Thr
