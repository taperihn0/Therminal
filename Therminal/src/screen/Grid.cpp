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
	if (ns > _v.capacity()) {
		THR_LOG_DEBUG("Line pointers buffer reallocation from capacity of {} to {}", 
					  _v.size(), ns);
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
	, _write_pos(0)
	, _ln_buf{}
	, _render_fmt{}
	, _formated(false)
	, _ln_ptrs(std::make_shared<LinePtrBuf>())
	, _last_nl_pos(0)
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

	Line& curr_ln = _ln_buf[_write_pos];

	if (c == U'\n') {
		advanceWriteIdx();
		++_last_nl_pos;
		return;
	}

	bool put_char = true;
	
	/* Handle special control characters */
	switch (c) {
	case U'\n':
		_last_nl_pos = _write_pos;
		advanceWriteIdx();
		put_char = false;
		break;
	//case U'\r':
	//	processCarriageReturn();
	//	put_char = false;
	//	break;
	}

	if (!put_char)
		return;

	curr_ln.putChar(c, state);

	if (curr_ln.getPrintableCount() >= _ln_width) {
		advanceWriteIdx();
	}
}

std::shared_ptr<const LinePtrBuf> Grid::getVisibleLines() const
{
	THR_ASSERT_LOG(_formated, "Cannot specify visible lines for unknown render format");

	const int total_line_cnt = _render_fmt.getCellCountHorizontal();

	_ln_ptrs->clear();
	_ln_ptrs->reserve(total_line_cnt);

	int line_cnt = 0;

	for (int ln_pos = _write_pos;
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

THR_FORCEINLINE size_t Grid::advanceWriteIdx() 
{
	_write_pos = (_write_pos + 1) % _BufSize;

	if (_write_pos == _start_ln_pos)
		_start_ln_pos = (_start_ln_pos + 1) % _BufSize;

	return _write_pos;
}

size_t Grid::decreaseWriteIdx()
{
	_write_pos = (_write_pos - 1) % _BufSize;

	if (_write_pos == _start_ln_pos)
		_start_ln_pos = (_start_ln_pos - 1) % _BufSize;

	return _write_pos;
}

void Grid::processCarriageReturn()
{
	while (_write_pos != _last_nl_pos)
		decreaseWriteIdx();

	Line& curr_ln = _ln_buf[_write_pos];
	curr_ln.trimToNewLine();
}

} // namespace Thr
