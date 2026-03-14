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
	const Vec<Ptr<const Line>>& getVec() const;
private:
	static constexpr size_t DefaultBufSize = 512;
	Vec<Ptr<const Line>> v;
};

class Grid
{
public:
	Grid(size_t ln_width);

	void specifyRenderFormat(const RenderFormat& format);
	void putChar(char32_t c, const EscapeState* state);

	std::shared_ptr<LinePtrBuf> getVisibleLines() const;
private:
	static constexpr size_t     _BufSize = 0x10000;
	const size_t                _ln_width;
	OutputStreamTransl          _utf8_utf32;
	size_t                      _cursor_pos;
	Arr<Line, _BufSize>         _ln_buf;
	RenderFormat                _render_format;
	bool                        _formated;
	std::shared_ptr<LinePtrBuf> _ln_ptrs;
};

} // namespace Thr
