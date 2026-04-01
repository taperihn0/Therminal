#include "Scrollback.hpp"
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

ScrollbackBuffer::ScrollbackBuffer()
	: _ln_width(0)
	, _start_ln_pos(0)
	, _write_pos{0, 0}
	, _ln_buf{}
	, _render_fmt{}
	, _formated(false)
	, _ln_ptrs(std::make_shared<LinePtrBuf>())
	, _after_nl_pos(0)
{}

void ScrollbackBuffer::specifyRenderFormat(const RenderFormat& format)
{
	_render_fmt = format;
	_formated = true;
	_ln_width = _render_fmt.getCellCount().x;
}

void ScrollbackBuffer::putGraphemeCluster(const GraphemeCluster& cluster, const EscapeState* state)
{
	THR_ASSERT_LOG(_formated, "Cannot add char for unknown render format");

	const size_t codepoint_cnt = cluster.codepoints.size();

	if (!codepoint_cnt) {
		THR_LOG_ERROR("Invalid cluster in grid, ignoring");
		return;
	}

	for (char32_t codepoint : cluster.codepoints) {
		switch (codepoint) {
		case U'\n': {			
			advanceWriteIdx();
			Line& income_ln = getActiveLine();

			if (!income_ln.getBufSize()) 
				income_ln.resize(_ln_width);

			income_ln.setCursorPos(_write_pos.x);
			_after_nl_pos = _write_pos.y;
			break;
		}
		case U'\r': {
			_write_pos.y = _after_nl_pos;

			Line& income_ln = getActiveLine();
			income_ln.onCarriageReturn();

			const size_t new_write_x = income_ln.getCursorPos();
			_write_pos.x = new_write_x;
			break;
		}
		default: {
			break;
		}
		}
	}

	THR_LOG_DEBUG("Scrollback: write-idx = {}", _write_pos.y);

	bool save_cluster = true;

	// Do not save CRLF sequence. It is given as single cluster with '\r\n'.
	// (May be also '\n\r', be aware of that).
	if (cluster.codepoints.size() == 2 &&
		cluster.codepoints.at(0) == '\r' && cluster.codepoints.at(1) == '\n')
		save_cluster = false;

	else if (cluster.codepoints.size() == 2 &&
			 cluster.codepoints.at(0) == '\n' && cluster.codepoints.at(1) == '\r')
		save_cluster = false;

	if (!save_cluster)
		return;

	if (_write_pos.x + cluster.getColumnWidth() > _ln_width) {
		advanceWriteIdx();
		_write_pos.x = 0;
		
		Line& curr_ln = getActiveLine();

		if (!curr_ln.getBufSize())
			curr_ln.resize(_ln_width);

		curr_ln.setCursorPos(_write_pos.x);
	}
	
	Line& curr_ln = getActiveLine();

	if (!curr_ln.getBufSize())
		curr_ln.resize(_ln_width);

	curr_ln.putGraphemeCluster(std::move(cluster), state);
}

std::shared_ptr<const LinePtrBuf> ScrollbackBuffer::getVisibleLines() const
{
	THR_ASSERT_LOG(_formated, "Cannot specify visible lines for unknown render format");

	const int total_line_cnt = _render_fmt.getCellCount().y;

	_ln_ptrs->clear();
	_ln_ptrs->reserve(total_line_cnt);

	int line_cnt = 0;

	for (int ln_pos = _write_pos.y;
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

THR_FORCEINLINE size_t ScrollbackBuffer::advanceWriteIdx() 
{
	shiftWriteIdx<+1>();
	return _write_pos.y;
}

THR_FORCEINLINE size_t ScrollbackBuffer::decreaseWriteIdx()
{
	shiftWriteIdx<-1>();
	return _write_pos.y;
}

template <int Shift>
void ScrollbackBuffer::shiftWriteIdx()
{
	THR_STATIC_ASSERT(Shift == 1 || Shift == -1);

	_write_pos.y = (_write_pos.y + Shift) % _BufSize;

	if (_write_pos.y == _start_ln_pos)
		_start_ln_pos = (_start_ln_pos + Shift) % _BufSize;
}

Line& ScrollbackBuffer::getActiveLine()
{
	return _ln_buf.at(_write_pos.y);
}

} // namespace Thr
