#pragma once

#include "Line.hpp"
#include "memory/CircBuff.hpp"
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

class ScrollbackBuffer
{
public:
	ScrollbackBuffer();

	void specifyRenderFormat(const RenderFormat& format);
	void putGraphemeCluster(const GraphemeCluster& cluster, const EscapeState* state);

	std::shared_ptr<const LinePtrBuf> getVisibleLines() const;
private:
	size_t advanceWriteIdx();
	size_t decreaseWriteIdx();
	
	template <int Shift>
	void shiftWriteIdx();

	Line& getActiveLine();

	static constexpr size_t     _BufSize = 65536;
	size_t                		_ln_width;
	size_t 						_start_ln_pos;
	glm::u64vec2                _write_pos;
	Arr<Line, _BufSize>         _ln_buf;
	RenderFormat                _render_fmt;
	bool                        _formated;
	std::shared_ptr<LinePtrBuf> _ln_ptrs;
	size_t   					_after_nl_pos;
};

} // namespace Thr
