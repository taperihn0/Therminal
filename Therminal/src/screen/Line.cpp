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

bool GraphemeCluster::isCRLF() const
{
	return codepoints.size() == 2 &&
		   (codepoints == U"\r\n" ||
			codepoints == U"\n\r");
}

Line::Line(const Line& l)
	: _buf_size(l._buf_size)
	, _ln(l._ln)
{
	const auto write_dist = std::distance(l._ln.cbegin(), Vec<Cell>::const_iterator(l._write_it));
	const auto rightmost_dist = std::distance(l._ln.cbegin(), Vec<Cell>::const_iterator(l._write_it));

	_write_it = std::next(_ln.begin(), write_dist);
	_rightmost_write_it = std::next(_ln.begin(), rightmost_dist);
}

Line::Line(Line&& l)
	: _buf_size(l._buf_size)
	, _ln(std::move(l._ln))
	, _write_it(l._write_it)
	, _rightmost_write_it(l._rightmost_write_it)
{}

void swap(Line& l0, Line& l1)
{
	std::swap(l0._buf_size, l1._buf_size);
	std::swap(l0._ln, l1._ln);
	std::swap(l0._write_it, l1._write_it);
	std::swap(l0._rightmost_write_it, l1._rightmost_write_it);
}

Line& Line::operator=(const Line& l)
{
	Line tmp = l;
	swap(*this, tmp);
	return *this;
}

Line& Line::operator=(Line&& l)
{
	swap(*this, l);
	return *this;
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
	return _ln.cbegin();
}

Line::const_iterator Line::end() const
{
	return _ln.cend();
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
	
	if (std::distance(_ln.begin(), _write_it) >= std::distance(_ln.cbegin(), _rightmost_write_it))
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

	_write_it = _ln.begin();
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
		
		if (column > pos) {
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

void Line::fill(const Cell& c)
{
	std::fill(_ln.begin(), _ln.end(), c);
}

} // namespace Thr
