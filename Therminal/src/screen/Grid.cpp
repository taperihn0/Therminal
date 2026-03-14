#include "Grid.hpp"
#include "io/OutputParser.hpp"
#include <algorithm>

namespace Thr
{

LinePtrBuf::LinePtrBuf()
{
	v.reserve(DefaultBufSize);
}

void LinePtrBuf::clear()
{
	v.clear();
}

void LinePtrBuf::push(Ptr<const Line> ptr)
{
	v.push_back(ptr);
}

void LinePtrBuf::reserve(size_t ns)
{
	if (ns > v.size()) {
		THR_LOG_DEBUG("Line pointers buffer reallocation from size of {} to {}", 
					  v.size(), 
					  ns);
	}

	v.reserve(ns);
}

const Vec<Ptr<const Line>>& LinePtrBuf::getVec() const
{
	return v;
}

Grid::Grid(size_t ln_width)
	: _ln_width(ln_width)
	, _cursor_pos(0)
	, _ln_buf{}
	, _render_format{}
	, _formated(false)
	, _ln_ptrs(std::make_shared<LinePtrBuf>())
{
	std::for_each(_ln_buf.begin(), _ln_buf.end(), 
		[&](Line& l) {
			l.resize(_ln_width);
		}
	);
}

void Grid::specifyRenderFormat(const RenderFormat& format)
{
	_render_format = format;
	_formated = true;
}

void Grid::putChar(char32_t c, const EscapeState* state)
{
	if (c == U'\n') {
		_cursor_pos = (_cursor_pos + 1) % _BufSize;
		return;
	}

	Line& curr_ln = _ln_buf[_cursor_pos];
	curr_ln.putChar(c, state);
}

std::shared_ptr<const LinePtrBuf> Grid::getVisibleLines() const
{
	THR_ASSERT_LOG(_formated, "Cannot specify visible lines for unknown render format");

	const size_t start_pos = 0; // TODO: Implement scrolling
	const int line_count = _render_format.getCellCountHorizontal();

	_ln_ptrs->clear();
	_ln_ptrs->reserve(line_count);

	std::for_each_n(_ln_buf.begin() + start_pos, line_count, [&](const Line& ln) {
		_ln_ptrs->push(std::addressof(ln));
	});
	
	return _ln_ptrs;
}

} // namespace Thr
