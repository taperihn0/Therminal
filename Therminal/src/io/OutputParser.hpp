#pragma once

#include "screen/Scrollback.hpp"
#include "col/Color.hpp"

namespace Thr
{
	
struct EscapeState
{
	Color3u8 fg;
	Color3u8 bg;
};

class OutputParser
{
public:
	OutputParser();
	void writeTo(std::shared_ptr<ScrollbackBuffer>& grid);
	void parseToGrid(const byte* stream, size_t n);
private:
	void processGraphemeCluster(GraphemeCluster& cluster);
	void processCSICommand(char32_t ch);

	enum class enumParseState
	{
		RAW = 0,
		ESCAPE = 1,
		CONTROL_SEQUENCE_INTRODUCER = 2,
	};

	std::shared_ptr<ScrollbackBuffer>       _grid;
	EscapeState                 _control_state;
	enumParseState              _parse_state;
	std::basic_string<char32_t> _control_buf;
};

} // namespace Thr
