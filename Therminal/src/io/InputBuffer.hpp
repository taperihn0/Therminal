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

	THR_INLINE byte get();
	THR_INLINE void put(byte data);

	THR_INLINE bool isFull() const;
	THR_INLINE bool isReady() const;

	THR_INLINE size_t getIncomingCnt() const;
	THR_INLINE size_t getCapacity() const;
private:
	const size_t       _size;
	CircularBuff<byte> _circ_buff;
	mutable std::mutex _mutex;
};

THR_INTERNAL InputRingBuffer::InputRingBuffer(size_t size)
	: _size(size)
	, _circ_buff(_size)
{}

THR_INLINE byte InputRingBuffer::get()
{
	std::lock_guard<std::mutex> lock(_mutex);
	return _circ_buff.get();
}

THR_INLINE void InputRingBuffer::put(byte data)
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

THR_INLINE size_t InputRingBuffer::getIncomingCnt() const
{
	std::lock_guard<std::mutex> lock(_mutex);
	return _circ_buff.getSize();
}

THR_INLINE size_t InputRingBuffer::getCapacity() const
{
	std::lock_guard<std::mutex> lock(_mutex);
	return _size;
}

} // namespace Thr
