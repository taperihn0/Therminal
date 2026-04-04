#include "OutputParser.hpp"
#include <cwctype>

namespace Thr
{
	
OutputParser::OutputParser()
	: _grid(nullptr)
	, _control_state{}
	, _parse_state(enumParseState::RAW)
{}

void OutputParser::writeTo(std::shared_ptr<Grid>& grid)
{
	_grid = grid;
}

void OutputParser::parseToGrid(const byte* stream, size_t n)
{
	if (_grid.get() == nullptr) {
		THR_LOG_ERROR("Current scrollback was not bound");
		return;
	}

	const std::basic_string_view<char8_t> sv(reinterpret_cast<const char8_t*>(stream), n);
	auto raw_clusters = una::views::grapheme::utf8(sv);

	for (const auto& raw_cluster : raw_clusters) {
		const std::basic_string<char32_t> u32str = una::utf8to32<char8_t, char32_t>(raw_cluster);
		GraphemeCluster cluster{ std::move(u32str) };
		processGraphemeCluster(cluster);
	}
}

THR_FORCEINLINE OutputParser::enumParseState OutputParser::stateFromControlChar(char32_t c)
{
	switch (c) {
	case U'N':  return enumParseState::SINGLE_SHIFT_2;
	case U'0':  return enumParseState::SINGLE_SHIFT_3;
	case U'P':	return enumParseState::DEVICE_CONTROL_STRING;
	case U'[':  return enumParseState::CONTROL_SEQUENCE_INTRODUCER;
	case U'\\': return enumParseState::STRING_TERMINATOR;
	case U']':  return enumParseState::OS_COMMAND;
	case U'X':  return enumParseState::START_OF_STRING;
	case U'^':  return enumParseState::PRIVACY_MSG;
	case U'_':  return enumParseState::APP_COMMAND;
	case U'=':  return enumParseState::DECKPAM;
	case U'>':  return enumParseState::DECKPNM;
	default:	break;
	}

	return enumParseState::INVALID;
}

THR_FORCEINLINE OutputParser::enumParseState OutputParser::stateFromC1Char(char32_t c)
{
	switch (c) {
	case U'\x8E': return enumParseState::SINGLE_SHIFT_2;
	case U'\x8F': return enumParseState::SINGLE_SHIFT_3;
	case U'\x90': return enumParseState::DEVICE_CONTROL_STRING;
	case U'\x9B': return enumParseState::CONTROL_SEQUENCE_INTRODUCER;
	case U'\x9C': return enumParseState::STRING_TERMINATOR;
	case U'\x9D': return enumParseState::OS_COMMAND;
	case U'\x98': return enumParseState::START_OF_STRING;
	case U'\x9E': return enumParseState::PRIVACY_MSG;
	case U'\x9F': return enumParseState::APP_COMMAND;
	default:	  break;
	}

	return enumParseState::INVALID;
}

void OutputParser::processGraphemeCluster(GraphemeCluster& cluster)
{
	const size_t codepoint_cnt = cluster.codepoints.size();

	if (!codepoint_cnt) {
		THR_LOG_DEBUG("Invalid grapheme cluster, ignoring");
		return;
	}
	
	const char32_t fch = cluster.codepoints[0];

	bool handle;

	while (!handle) {
		handle = true;

		switch (_parse_state) {
		case enumParseState::RAW: {
			if (codepoint_cnt == 1 && fch == U'\x1b') {
				_parse_state = enumParseState::ESCAPE;
				break;
			}
			else if (codepoint_cnt == 1 &&
					Char32(fch).isControl0()) {
				// TODO: handle c0 codes here
			}
			else if (codepoint_cnt == 1 &&
					Char32(fch).isControl1()) {
				_parse_state = stateFromC1Char(fch);
				break;
			}

			_grid->putGraphemeCluster(std::move(cluster), std::addressof(_control_state));
			break;
		}
		case enumParseState::ESCAPE: {
			if (codepoint_cnt != 1) {
				THR_LOG_DEBUG("Invalid codepoint in ESCAPE state");
				_parse_state = enumParseState::RAW;
			}
			else {
				_control_buf.clear();
				_parse_state = stateFromControlChar(fch);
			}

			break;
		}
		case enumParseState::SINGLE_SHIFT_2: {
			_parse_state = enumParseState::RAW;
			handle = true;
			break;
		}
		case enumParseState::SINGLE_SHIFT_3: {
			_parse_state = enumParseState::RAW;
			handle = true;
			break;
		}
		case enumParseState::DEVICE_CONTROL_STRING: {
			_parse_state = enumParseState::RAW;
			handle = true;
			break;
		}
		case enumParseState::CONTROL_SEQUENCE_INTRODUCER: {
			if (codepoint_cnt > 1) {
				THR_LOG_DEBUG("Invalid CSI command, back to RAW state");
				_parse_state = enumParseState::RAW;
				break;
			}
			else if (fch >= U'\x40' && fch <= U'\x7e') {
				processCSICommand(fch);
				_parse_state = enumParseState::RAW;
				break;
			} 

			_control_buf += fch;
			break;
		}
		case enumParseState::STRING_TERMINATOR: {
			_parse_state = enumParseState::RAW;
			break;
		}
		case enumParseState::OS_COMMAND: {
			if (codepoint_cnt > 1) {
				THR_LOG_DEBUG("Invalid OS command, back to RAW state");
				_parse_state = enumParseState::RAW;
				break;
			}
			else if (fch == U'\x07' || fch == U'\x9c' ||
					(fch == U'\x5c' && _control_buf.back() == U'\x1b') /* ST */) {
				processOSCommand();
				_parse_state = fch == U'\x5c' ? enumParseState::STRING_TERMINATOR 
											: enumParseState::RAW;

				break;
			}

			_control_buf += fch;
			break;
		}
		case enumParseState::START_OF_STRING: {
			_parse_state = enumParseState::RAW;
			handle = true;
			break;
		}
		case enumParseState::PRIVACY_MSG: {
			_parse_state = enumParseState::RAW;
			handle = true;
			break;
		}
		case enumParseState::APP_COMMAND: {
			_parse_state = enumParseState::RAW;
			handle = true;
			break;
		}
		case enumParseState::DECKPAM: {
			_parse_state = enumParseState::RAW;
			handle = false;
			break;
		}
		case enumParseState::DECKPNM: {
			_parse_state = enumParseState::RAW;
			handle = false;
			break;
		}
		default:
			break;
		}
	}
}

void OutputParser::processCSICommand(char32_t ch)
{
	switch (ch) {
	case 'm': { /* Select Graphic Rendition	*/
		// TODO
		break;
	}
	case 'J': { /* Erase in Display */
		const size_t intermediate_cnt = _control_buf.size();
		int n = 0;

		if (intermediate_cnt > 1) {
			THR_LOG_ERROR("Got multiple intermediate bytes, ignoring");
			break;
		}
		else if (!intermediate_cnt) {
			n = 0;
		}
		else {
			const char32_t fch = _control_buf.front();

			if (fch > 127 || !std::iswdigit(static_cast<std::wint_t>(fch))) {
				THR_LOG_ERROR("Invalid intermediate number n, ignoring");
				break;
			}

			n = static_cast<int>(fch - U'0');
		}

		switch (n) {
		case 0: {
			_grid->eraseRightFromCursor();
			break;
		}
		case 1: {
			_grid->eraseLeftFromCursor();
			break;
		}
		case 2: {
			_grid->eraseAll();
			_grid->setCursorPos(glm::u64vec2{0, 0});
			break;
		}
		case 3: {
			_grid->eraseAll();
			_grid->clearScrollbackBuffer();
			break;
		}
		default: {
			THR_LOG_ERROR("Invalid intermediate number n, ignoring");
			break;
		}
		}

		break;
	}
	case 'K': { /* Erase in Line */
		const size_t intermediate_cnt = _control_buf.size();
		int n = 0;

		if (intermediate_cnt > 1) {
			THR_LOG_ERROR("Got multiple intermediate bytes, ignoring");
			break;
		}
		else if (!intermediate_cnt) {
			n = 0;
		}
		else {
			const char32_t fch = _control_buf.front();

			if (fch > 127 || !std::iswdigit(static_cast<std::wint_t>(fch))) {
				THR_LOG_ERROR("Invalid intermediate number n, ignoring");
				break;
			}

			n = static_cast<int>(fch - U'0');
		}

		switch (n) {
		case 0: {
			_grid->eraseRightLineFromCursor();
			break;
		}
		case 1: {
			_grid->eraseLeftLineFromCursor();
			break;
		}
		case 2: {
			_grid->eraseAllLine();
			break;
		}
		default: {
			THR_LOG_ERROR("Invalid intermediate number n, ignoring");
			break;
		}
		}
	}
	default: 
		break;
	}
}

void OutputParser::processOSCommand()
{
	// TODO
}

} // namespace Thr
