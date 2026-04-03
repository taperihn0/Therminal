#include "Scrollback.hpp"
#include "io/OutputParser.hpp"
#include <algorithm>

namespace Thr
{

LineView::LineView(size_t size)
{
	_v.reserve(size);
}

LineView::const_iterator LineView::begin() const
{
	return _v.begin();
}

LineView::const_iterator LineView::end() const
{
	return _v.end();
}

LineView::const_iterator LineView::cbegin() const
{
	return _v.cbegin();
}

LineView::const_iterator LineView::cend() const
{
	return _v.cend();
}

void LineView::clear()
{
	_v.clear();
}

void LineView::push(Ptr<const Line> ptr)
{
	_v.push_back(ptr);
}

void LineView::reserve(size_t ns)
{
	if (ns > _v.capacity()) {
		THR_LOG_DEBUG("Line pointers buffer reallocation from capacity of {} to {}", 
					  _v.size(), ns);
	}

	_v.reserve(ns);
}

void LineView::reverse()
{
	std::reverse(_v.begin(), _v.end());
}

ScrollbackBuffer::ScrollbackBuffer()
	: _ln_buf(_BufSize)
	, _ln_width(0)
	, _formated(false)
{}

void ScrollbackBuffer::specifyRenderFormat(const RenderFormat& format)
{
	_render_fmt = format;
	_formated = true;
	_ln_width = _render_fmt.getCellCount().x;
}

void ScrollbackBuffer::pushLine(const Line& line)
{
	_ln_buf.put(line);
}

} // namespace Thr
