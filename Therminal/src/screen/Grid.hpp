#pragma once

#include "Line.hpp"
#include "memory/CircBuff.hpp"
#include "io/OutputTranslator.hpp"
#include "gl/RenderFormat.hpp"

namespace Thr
{

class LinePtrBuf
{
public:
	LinePtrBuf();
	void clear();
	void push(Ptr<const Line> ptr);
	void reserve(size_t ns);
	void reverse();
	const Vec<Ptr<const Line>>& getVec() const;
private:
	static constexpr size_t _DefaultBufSize = 512;
	Vec<Ptr<const Line>> _v;
};

class Grid
{
public:
	Grid();

	void specifyRenderFormat(const RenderFormat& format);
	void putChar(char32_t c, const EscapeState* state);

	std::shared_ptr<const LinePtrBuf> getVisibleLines() const;
private:
	size_t advanceWriteIdx();
	size_t decreaseWriteIdx();
	void processCarriageReturn();

	static constexpr size_t     _BufSize = 0x10000;
	size_t                		_ln_width;
	OutputStreamTransl          _utf8_utf32;
	size_t 						_start_ln_pos;
	size_t                      _write_pos;
	Arr<Line, _BufSize>         _ln_buf;
	RenderFormat                _render_fmt;
	bool                        _formated;
	std::shared_ptr<LinePtrBuf> _ln_ptrs;
	size_t   					_last_nl_pos;
};

} // namespace Thr
