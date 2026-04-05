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

	/* CSI sequence handlers */
	void cursorUpCSI();
	void cursorDownCSI();
	void cursorForwardCSI();
	void cursorBackCSI();
	void cursorNextLineCSI();
	void cursorPrevLineCSI();
	void cursorHorizontalAbsCSI();
	void cursorPosCSI();
	void cursorTabControlCSI();
	void eraseInDisplayCSI();
	void eraseInLineCSI();
	void eraseCharsCSI();
	void insertLineCSI();
	void deleteLineCSI();
	void deleteCharsCSI();
	void scrollUpCSI();
	void scrollDownCSI();
	void insertCharsCSI();
	void selGraphicsRenditionCSI();
	void horizontalVertPosCSI();
	void tabClearCSI();
	void setModeCSI();
	void resetModeCSI();
	void decSetModeCSI();
	void decResetModeCSI();
	void deviceStatusReportCSI();
	void deviceAttributesCSI();
	void softTerminalResetCSI();
	void setScrollRegionCSI();
	
	enum class enumParseState
	{
		RAW = 0,
		ESCAPE = 1,
		SINGLE_SHIFT_2 = 2,
		SINGLE_SHIFT_3 = 3,
		DEVICE_CONTROL_STRING = 4,
		CONTROL_SEQUENCE_INTRODUCER = 5,
		STRING_TERMINATOR = 6,
		OS_COMMAND = 7,
		START_OF_STRING = 8,
		PRIVACY_MSG = 9,
		APP_COMMAND = 10,
		DECKPAM = 11,
		DECKPNM = 12,
		INVALID
	};
	
	enumParseState stateFromControlChar(char32_t c);
	enumParseState stateFromC1Char(char32_t c);

	std::shared_ptr<Grid> 		_grid;
	EscapeState                 _control_state;
	enumParseState              _parse_state;
	std::basic_string<char32_t> _control_buf;
};

} // namespace Thr
