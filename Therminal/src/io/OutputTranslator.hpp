#pragma once

#include "Common.hpp"
#include <cuchar>

namespace Thr
{
	
class OutputStreamTransl
{
public:
	OutputStreamTransl();
	THR_INLINE void setBuf(std::string_view utf8);
	THR_INLINE bool nextUTF32(char32_t* ch);
private:
	static inline std::mbstate_t     _state = {};
	std::string_view                 _utf8;
	size_t                           _to_read;
	std::string_view::const_iterator _it;
};

THR_INTERNAL OutputStreamTransl::OutputStreamTransl()
	: _utf8("")
	, _to_read(0)
	, _it()
{}

THR_INLINE void OutputStreamTransl::setBuf(std::string_view utf8)
{
	_utf8 = utf8;
	_it = _utf8.begin();
	_to_read = std::distance(_it, utf8.end());
}

THR_INLINE bool OutputStreamTransl::nextUTF32(char32_t* ch)
{
	if (!_to_read)
		return false;

	THR_HARD_ASSERT(_it != _utf8.end());

	const size_t nread = std::mbrtoc32(ch, std::addressof(*_it), _to_read, &_state);

	if (nread == 0) {
		_to_read = 0;
		return false;
	}

	THR_HARD_ASSERT(nread != static_cast<size_t>(-3));

	if (nread == static_cast<size_t>(-1) ||
		nread == static_cast<size_t>(-2)) {
		_to_read = 0;
		return false;
	}

	THR_HARD_ASSERT(_to_read >= nread);
	std::advance(_it, nread);
	_to_read -= nread;

	return true;
}

} // namespace Thr
