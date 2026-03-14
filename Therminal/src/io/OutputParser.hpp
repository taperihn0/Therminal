#pragma once

#include "OutputTranslator.hpp"
#include "screen/Grid.hpp"
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
    void writeTo(std::shared_ptr<Grid>& grid);
    void parseToGrid(const byte* stream, size_t n);
private:
    void processChar(char32_t ch);
    void processCSICommand(char32_t ch);

    enum class enumParseState
    {
        RAW = 0,
        ESCAPE = 1,
        CONTROL_SEQUENCE_INTRODUCER = 2,
    };

    std::shared_ptr<Grid> _grid;
    OutputStreamTransl    _utf8_to_utf32;
    EscapeState           _control_state;
    enumParseState       _parse_state;
    std::string          _control_buf;
};

} // namespace Thr
