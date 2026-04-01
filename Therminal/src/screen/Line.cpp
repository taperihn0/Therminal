#include "Line.hpp"
#include "io/OutputParser.hpp"

namespace Thr
{

bool GraphemeCluster::isPrintable() const
{
	return std::any_of(codepoints.cbegin(), codepoints.cend(),
		[](char32_t ch) {
			return Char32(ch).isPrintable();
		});
}

int GraphemeCluster::getColumnWidth() const
{
	if (isEmpty())
		return 0;

	const auto max_it = std::max_element(codepoints.cbegin(), codepoints.cend(), 
		[](char32_t ch0, char32_t ch1) {
			return Char32(ch0).getWidth() < Char32(ch1).getWidth();
		});

	return Char32(*max_it).getWidth();
}

bool GraphemeCluster::isEmpty() const
{
	return codepoints.empty();
}

size_t Line::getBufSize() const
{
	return _buf_size;
}

Line::const_iterator Line::cbegin() const
{
	return begin();
}

Line::const_iterator Line::cend() const
{
	return end();
}

Line::const_iterator Line::begin() const
{
	return _ln.begin();
}

Line::const_iterator Line::end() const
{
	return _ln.end();
}

void Line::clear()
{
	_ln.clear();
}

void Line::putGraphemeCluster(const GraphemeCluster& cluster, const EscapeState* state)
{
	const size_t write_idx = std::distance(_ln.begin(), _write_it);

	if (write_idx >= _buf_size) {
		THR_LOG_FATAL("Overflowing in line");
		return;
	}

	markUnused(state);
	const Cell cell = { cluster, {}, {} };

	*_write_it = std::move(cell);
	_rightmost_write_it = _write_it;
	++_write_it;
}

const Vec<Cell> Line::getVec() const
{
	return _ln;
}

void Line::onCarriageReturn()
{
	if (!_buf_size) {
		THR_LOG_FATAL("OnCarriageReturn for empty line");
		return;
	}

	const auto r_it = std::find_if(_ln.rbegin(), _ln.rend(), 
		[](const Cell& cell) {
			return std::any_of(cell.cluster.codepoints.cbegin(), cell.cluster.codepoints.cend(), 
				[](char32_t ch) {
					return ch == U'\n';
				});
		});

	_write_it = r_it != _ln.rend() ? r_it.base() : _ln.begin();
}

size_t Line::getCursorPos() const
{
	size_t column = 0;

	for (auto it = _ln.begin(); it != _write_it; it++) {
		if (it->cluster.isEmpty()) 
			column++;
		else
			column += it->cluster.isPrintable() * it->cluster.getColumnWidth();
	}

	return column;
}

const Line::const_iterator Line::getRightmostWriteIterator() const
{
	return std::next(_rightmost_write_it);
}

void Line::setCursorPos(size_t pos)
{
	size_t column = 0;

	for (auto it = _ln.begin(); it != _ln.end(); it++) {
		if (it->cluster.isEmpty())
			column++;
		else 
			column += it->cluster.isPrintable() * it->cluster.getColumnWidth();
		
		if (column >= pos) {
			_write_it = it;
			break;
		}
	}
}

void Line::resize(size_t buf_size)
{
	THR_ASSERT(buf_size < _BufSizeLimit);
	_buf_size = buf_size;
	_ln.resize(_buf_size);
	_write_it = _ln.begin();
	_rightmost_write_it = _write_it;
}

} // namespace Thr
