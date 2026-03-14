#include "Grid.hpp"
#include "io/OutputParser.hpp"
#include <algorithm>

namespace Thr
{

Grid::Grid(size_t ln_width)
	: _ln_width(ln_width)
	, _cursor_pos(0)
	, _ln_buf{}
	, _render_format{}
	, _formated(false)
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

Vec<Ptr<const Line>> Grid::getVisibleLines() const
{
	THR_ASSERT_LOG(_formated, "Cannot specify visible lines for unknown render format");

	const size_t start_pos = 0; // TODO: Implement scrolling
	const int line_count = _render_format.getCellCountHorizontal();

	Vec<Ptr<const Line>> render_pack;
	render_pack.reserve(line_count);
	
	std::for_each_n(_ln_buf.begin() + start_pos, line_count, [&](const Line& ln) {
		render_pack.push_back(std::addressof(ln));
	});
	
	return render_pack;
}

} // namespace Thr
