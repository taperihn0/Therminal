#include "Grid.hpp"

namespace Thr
{

Grid::Grid(const RenderFormat& format)
	: _ln_buf(format.getCellCount().y)
	, _write_pos{0, 0}
	, _ln_ptrs(std::make_shared<LineView>())
	, _after_nl_pos(0)
	, _cell_cnt{format.getCellCount()}
	, _fmt(format)
	, _formated(true)
	, _scrollback(nullptr)
{
	init();
}

void Grid::specifyScrollbackBuffer(std::shared_ptr<ScrollbackBuffer>& scrollback)
{
	_scrollback = scrollback;
}
	
void Grid::setCursorPos(glm::u64vec2 pos)
{
	if (!_formated) {
		THR_LOG_ERROR("Grid not formatted");
		return;
	}

	if (pos.x >= _cell_cnt.x || pos.y >= _cell_cnt.y) {
		THR_LOG_FATAL("Invalid cursor position");
		return;
	}

	_write_pos = pos;

	Line& curr_ln = getActiveLine();
	curr_ln.setCursorPos(_write_pos.x);
}

void Grid::eraseRightFromCursor()
{

}

void Grid::eraseLeftFromCursor()
{

}

void Grid::eraseAll()
{

}

void Grid::clearScrollbackBuffer()
{

}

void Grid::eraseRightLineFromCursor()
{

}

void Grid::eraseLeftLineFromCursor()
{

}

void Grid::eraseAllLine()
{

}

glm::u64vec2 Grid::getCursorPos() const
{
	return _write_pos;
}

void Grid::putGraphemeCluster(const GraphemeCluster& cluster, const EscapeState* state)
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
			const uint new_write_y = _write_pos.y + 1;

			if (new_write_y == _cell_cnt.y) {
				Line nln = _ln_buf.get();
				nln.fill(Cell{});
				_ln_buf.put(nln);
			}
			else _write_pos.y = new_write_y;

			Line& income_ln = getActiveLine();

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
		case U'\b': {
			if (!_write_pos.x && _write_pos.y > 0) {
				_write_pos.y--;

				const size_t new_write_x = _cell_cnt.x - 1;
				_write_pos.x = new_write_x;

				Line& income_ln = getActiveLine();
				income_ln.setCursorPos(_write_pos.x);
			}
			else {
				const size_t new_write_x = std::max(0, static_cast<int>(_write_pos.x) - 1);
				_write_pos.x = new_write_x;

				Line& curr_ln = getActiveLine();
				curr_ln.setCursorPos(_write_pos.x);
			}

			break;
		}
		default: {
			break;
		}
		}
	}

	bool save_cluster = true;

	// Do not save CRLF sequence. It is given as single cluster with '\r\n'.
	// (May be also '\n\r', be aware of that).
	if (cluster.codepoints == U"\r\n" ||
		cluster.codepoints == U"\n\r")
		save_cluster = false;

	else if (cluster.codepoints == U"\n" ||
			 cluster.codepoints == U"\r" ||
			 cluster.codepoints == U"\b")
		save_cluster = false;

	if (!save_cluster)
		return;

	if (_write_pos.x + cluster.isPrintable() * cluster.getColumnWidth() > _cell_cnt.x) {
		const uint new_write_y = _write_pos.y + 1;

		if (new_write_y == _cell_cnt.y) {
			Line nln = _ln_buf.get();
			nln.fill(Cell{});
			_ln_buf.put(nln);
		}
		else _write_pos.y = new_write_y;

		_write_pos.x = 0;
		_after_nl_pos = _write_pos.y;

		Line& curr_ln = getActiveLine();
		curr_ln.setCursorPos(_write_pos.x);
	}

	Line& curr_ln = getActiveLine();
	curr_ln.putGraphemeCluster(std::move(cluster), state);
	_write_pos.x = curr_ln.getCursorPos();
}

Line& Grid::getActiveLine()
{
	return _ln_buf.getIthElement(_write_pos.y);
}

void Grid::init()
{
	for (size_t i = 0; i < _cell_cnt.y; i++) {
		Line ln;
		ln.resize(_cell_cnt.x);
		_ln_buf.put(ln);
	}
}

std::shared_ptr<const LineView> Grid::getLineView() const
{
	if (!_formated) {
		THR_LOG_FATAL("Cannot get line view of unformatted grid");
		return nullptr;
	}

	_ln_ptrs->clear();
	_ln_ptrs->reserve(_cell_cnt.y);
	
	for (size_t i = 0; i < _cell_cnt.y; i++) {
		const Line& ln = _ln_buf.getIthElement(i);
		_ln_ptrs->push(std::addressof(ln));
	}

	return _ln_ptrs;
}

} // namespace Thr
