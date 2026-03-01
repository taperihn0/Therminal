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
	THR_INLINE void write(const byte* buf, int n);
	THR_INLINE const byte* read(int& n) const;
	
	static constexpr size_t BuffSize = 0x1000;
private:
	struct _FlipBuff;

	static constexpr size_t      	_WriteSideBuff = 0;
	static constexpr size_t      	_ReadSideBuff  = 1;
	std::unique_ptr<_FlipBuff> 		_buff_ptr[2];
	mutable std::mutex              _read_mutex;
	mutable std::mutex              _write_mutex;

	struct _FlipBuff
	{
		_FlipBuff(size_t buf_size);

		THR_INLINE void clear();

		int 					n;
		std::unique_ptr<byte[]> ptr;
	};
};

THR_INTERNAL OutputBuffer::OutputBuffer()
	: _buff_ptr{ 
		std::make_unique<_FlipBuff>(BuffSize), 
		std::make_unique<_FlipBuff>(BuffSize) }
{}

THR_INLINE void OutputBuffer::swap()
{
	std::scoped_lock lock { _read_mutex, _write_mutex };
	std::swap(_buff_ptr[_WriteSideBuff], _buff_ptr[_ReadSideBuff]);
	_buff_ptr[_WriteSideBuff]->n = 0;
}
	
THR_INLINE void OutputBuffer::write(const byte* buf, int n) 
{
	THR_HARD_ASSERT(n >= 0 && n < static_cast<int>(BuffSize));
	std::lock_guard<std::mutex> lock(_write_mutex);

	_FlipBuff& write_buff = *_buff_ptr[_WriteSideBuff];

	THR_HARD_ASSERT(write_buff.n + n < static_cast<int>(BuffSize));

	memCpy(write_buff.ptr.get() + write_buff.n, 
		   buf, 
		   n);

	write_buff.n += n;
}

THR_INLINE const byte* OutputBuffer::read(int& n) const
{
	std::lock_guard<std::mutex> lock(_read_mutex);
	_FlipBuff& read_buff = *_buff_ptr[_ReadSideBuff];
	const byte* r = read_buff.ptr.get();
	n = read_buff.n;
	return r;
}

THR_INTERNAL OutputBuffer::_FlipBuff::_FlipBuff(size_t buf_size)
	: n(0)
	, ptr(std::make_unique<byte[]>(buf_size))
{}

THR_INLINE void OutputBuffer::_FlipBuff::clear()
{
	n = 0;
}

} // namespace Thr
