#include "OutputParser.hpp"

namespace Thr
{
    
OutputParser::OutputParser()
    : _grid(nullptr)
    , _control_state{}
    , _parse_state(enumParseState::RAW)
{}

void OutputParser::writeTo(std::shared_ptr<ScrollbackBuffer>& grid)
{
    _grid = grid;
}

void OutputParser::parseToGrid(const byte* stream, size_t n)
{
    if (_grid.get() == nullptr) {
        THR_LOG_ERROR("Current grid was not bound");
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

void OutputParser::processGraphemeCluster(GraphemeCluster& cluster)
{
    const size_t codepoint_cnt = cluster.codepoints.size();

    if (!codepoint_cnt) {
        THR_LOG_ERROR("Invalid grapheme cluster, ignoring");
        return;
    }
    
    const char32_t fch = cluster.codepoints[0];

    switch (_parse_state) {
    case enumParseState::RAW: {
        if (codepoint_cnt == 1 && fch == U'\x1b') {
            _parse_state = enumParseState::ESCAPE;
            break;
        }

        _grid->putGraphemeCluster(std::move(cluster), &_control_state);
        break;
    }
    case enumParseState::ESCAPE: {
        if (codepoint_cnt == 1 && fch == U'[') {
            _parse_state = enumParseState::CONTROL_SEQUENCE_INTRODUCER;
            _control_buf.clear();
            break;
        }

        _parse_state = enumParseState::RAW;
        break;
    }
    case enumParseState::CONTROL_SEQUENCE_INTRODUCER: {
        if (codepoint_cnt > 1) {
            THR_LOG_ERROR("Invalid CSI command, back to RAW state");
            _parse_state = enumParseState::RAW;
            break;
        }
        else if (fch >= U'\x40' && fch <= U'\x7E') {
            processCSICommand(fch);
            _parse_state = enumParseState::RAW;
            break;
        } 

        _control_buf += fch;
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
