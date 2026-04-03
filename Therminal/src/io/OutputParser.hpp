#pragma once

#include "screen/Scrollback.hpp"
#include "col/Color.hpp"
#include "screen/Grid.hpp"

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
	void writeTo(std::shared_ptr<Grid>& grid);
	void parseToGrid(const byte* stream, size_t n);
private:
	void processGraphemeCluster(GraphemeCluster& cluster);
	void processCSICommand(char32_t ch);
	void processOSCommand();
	
	enum class enumParseState
	{
		RAW = 0,
		ESCAPE = 1,
		DEVICE_CONTROL_STRING = 2,
		CONTROL_SEQUENCE_INTRODUCER = 3,
		STRING_TERMINATOR = 4,
		OS_COMMAND = 5,
		START_OF_STRING = 6,
		PRIVACY_MSG = 7,
		APP_COMMAND = 8,
		INVALID
	};
	
	enumParseState stateFromControlChar(char32_t c);

	std::shared_ptr<Grid> 		_grid;
	EscapeState                 _control_state;
	enumParseState              _parse_state;
	std::basic_string<char32_t> _control_buf;
};

} // namespace Thr
