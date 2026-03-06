#include "OutputParser.hpp"

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
        THR_LOG_ERROR("Current grid was not bound");
        return;
    }

    const std::string_view sv(reinterpret_cast<const char*>(stream), n);
    _utf8_to_utf32.setBuf(sv);

    char32_t ch;

    while (_utf8_to_utf32.nextUTF32(&ch)) {
        processChar(ch);
    }
}

void OutputParser::processChar(char32_t ch)
{
    switch (_parse_state) {
    case enumParseState::RAW: {
        if (ch == U'\x1b') {
            _parse_state = enumParseState::ESCAPE;
            break;
        }
        _grid->putChar(ch, &_control_state);
        break;
    }
    case enumParseState::ESCAPE: {
        if (ch == U'[') {
            _parse_state = enumParseState::CONTROL_SEQUENCE_INTRODUCER;
            _control_buf.clear();
            break;
        }
        _parse_state = enumParseState::RAW;
        break;
    }
    case enumParseState::CONTROL_SEQUENCE_INTRODUCER: {
        if (ch >= U'\x40' && ch <= U'\x7E') {
            processCSICommand(ch);
            _parse_state = enumParseState::RAW;
            break;
        } 
        _control_buf += static_cast<char>(ch);
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
    default: break;
    }
}

} // namespace Thr
