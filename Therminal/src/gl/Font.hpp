#pragma once

#include "Common.hpp"
#include "filesys/Filepath.hpp"

namespace Thr
{

/* Abstraction of FreeType glyphs.
*/
class Font
{
public:
    Font() = default;
    Font(const Font&) = default;
    Font(Font&& f);
    ~Font();

    Font& operator=(const Font&) = default;
    Font& operator=(Font&& f);

    void init(const FilePath& font_path, int req_glyph_height);

    // TODO: create custom datatype for glyph info
    FT_GlyphSlot getGlyphOf(char32_t ch);
    FT_Size_Metrics getGlyphMetrics() const;

    bool isReady() const;
private:
	FT_Library _ft_lib = nullptr;
	FT_Face	   _ft_face   = nullptr;
    bool       _initialized = false;
};

} // namespace Thr
