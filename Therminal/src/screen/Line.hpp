#pragma once

#include "Common.hpp"
#include "col/Color.hpp"
#include "char/Char.hpp"

namespace Thr
{

struct GraphemeCluster
{
	bool isPrintable() const;
	int getColumnWidth() const;
	bool isEmpty() const;

	std::basic_string<char32_t> codepoints;
};

struct Cell
{
	GraphemeCluster cluster;
	Color3u8        fg;
	Color3u8        bg;
};

struct EscapeState;

/* Represent single line of cells.
*/
class Line
{
public:
	Line() = default;
	
	using iterator = std::vector<Cell>::iterator;
	using const_iterator = std::vector<Cell>::const_iterator;
	
	const_iterator cbegin() const;
	const_iterator cend()  const;
	const_iterator begin() const;
	const_iterator end() const;

	void clear();
	void resize(size_t buf_size);

	size_t getBufSize() const;
	const Vec<Cell> getVec() const;
	size_t getCursorPos() const;
	const const_iterator getRightmostWriteIterator() const;

	void putGraphemeCluster(const GraphemeCluster& cluster, const EscapeState* state);
	void setCursorPos(size_t pos);
	void onCarriageReturn();
private:
	static constexpr size_t _BufSizeLimit = 0x800;
	size_t                  _buf_size = 0;
	Vec<Cell>               _ln;
	iterator                _write_it;
	const_iterator 			_rightmost_write_it;
};

} // namespace Thr
