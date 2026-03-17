#include "Grid.hpp"
#include "io/OutputParser.hpp"
#include <algorithm>

namespace Thr
{

LinePtrBuf::LinePtrBuf()
{
	_v.reserve(_DefaultBufSize);
}

void LinePtrBuf::clear()
{
	_v.clear();
}

void LinePtrBuf::push(Ptr<const Line> ptr)
{
	_v.push_back(ptr);
}

void LinePtrBuf::reserve(size_t ns)
{
	if (ns > _v.size()) {
		THR_LOG_DEBUG("Line pointers buffer reallocation from size of {} to {}", 
					  _v.size(), 
					  ns);
	}

	_v.reserve(ns);
}

void LinePtrBuf::reverse()
{
	std::reverse(_v.begin(), _v.end());
}

const Vec<Ptr<const Line>>& LinePtrBuf::getVec() const
{
	return _v;
}

Grid::Grid()
	: _ln_width(0)
	, _start_ln_pos(0)
	, _curr_ln_pos(0)
	, _ln_buf{}
	, _render_fmt{}
	, _formated(false)
	, _ln_ptrs(std::make_shared<LinePtrBuf>())
{}

void Grid::specifyRenderFormat(const RenderFormat& format)
{
	_render_fmt = format;
	_formated = true;

	_ln_width = _render_fmt.getCellCountVertical();

	std::for_each(_ln_buf.begin(), _ln_buf.end(), [&](Line& l) {
		l.reserve(_ln_width);
	});
}

void Grid::putChar(char32_t c, const EscapeState* state)
{
	THR_ASSERT_LOG(_formated, "Cannot add char for unknown render format");

	if (c == U'\n') {
		_curr_ln_pos = (_curr_ln_pos + 1) % _BufSize;

		if (_curr_ln_pos == _start_ln_pos)
			_start_ln_pos = (_start_ln_pos + 1) % _BufSize;

		return;
	}

	Line& curr_ln = _ln_buf[_curr_ln_pos];
	curr_ln.putChar(c, state);

	if (curr_ln.getPrintableCount() >= _ln_width) {
		_curr_ln_pos = (_curr_ln_pos + 1) % _BufSize;

		if (_curr_ln_pos == _start_ln_pos)
			_start_ln_pos = (_start_ln_pos + 1) % _BufSize;
	}
}

std::shared_ptr<const LinePtrBuf> Grid::getVisibleLines() const
{
	THR_ASSERT_LOG(_formated, "Cannot specify visible lines for unknown render format");

	const int ln_cell_cnt = _render_fmt.getCellCountVertical();
	const int total_line_cnt = _render_fmt.getCellCountHorizontal();

	_ln_ptrs->clear();
	_ln_ptrs->reserve(total_line_cnt);

	int line_cnt = 0;

	THR_LOG_DEBUG("{}", _curr_ln_pos);

	for (int ln_pos = _curr_ln_pos;
		 ;
		 ln_pos = (ln_pos - 1) % _BufSize)
	{
		const Line& ln = _ln_buf[ln_pos];
		line_cnt++;

		_ln_ptrs->push(std::addressof(ln));

		if (line_cnt >= total_line_cnt || 
			ln_pos == static_cast<int>(_start_ln_pos))
			break;
	}
	
	_ln_ptrs->reverse();
	return _ln_ptrs;
}

} // namespace Thr
