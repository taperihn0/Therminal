#pragma once

#include "Line.hpp"
#include "memory/CircBuff.hpp"
#include "gl/RenderFormat.hpp"

namespace Thr
{

class LineView
{
public:
	LineView();

	using const_iterator = Vec<Ptr<const Line>>::const_iterator;

	const_iterator begin() const;
	const_iterator end() const;
	const_iterator cbegin() const;
	const_iterator cend() const;

	void clear();
	void push(Ptr<const Line> ptr);
	void reserve(size_t ns);
	void reverse();
private:
	static constexpr size_t _DefaultBufSize = 512;
	Vec<Ptr<const Line>> _v;
};

class ScrollbackBuffer
{
public:
	ScrollbackBuffer();

	void specifyRenderFormat(const RenderFormat& format);
	void pushLine(const Line& line);
private:
	static constexpr size_t  _BufSize = 65536;
	CircularBuff<Line> 		 _ln_buf;
	size_t                	 _ln_width;
	bool                     _formated;
	RenderFormat             _render_fmt;
};

} // namespace Thr
