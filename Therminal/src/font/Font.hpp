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
    void markScale(float on_scr_scale);
    float getScale() const;

    struct Glyph 
    {
        uint32_t          glyph_index;
        FT_Glyph_Metrics  ft_metrics;
        glm::i32vec2      advance_pix;
        FT_Bitmap         ft_bitmap;
        int32_t           bearing_x_pix;
        int32_t           bearing_y_pix;
    };

    struct Metrics
    {
        int32_t max_advance_pix;
        int32_t height_pix;
        int32_t ascender_pix;
    };

    bool getGlyphOf(char32_t ch, Glyph& glyph);
    bool getGlyphMetrics(Metrics& metrics) const;

    bool isReady() const;
    bool isColoredFont() const;
private:
    bool selectHeight(int req_glyph_height);

	FT_Library  _ft_lib    = nullptr;
	FT_Face	    _ft_face   = nullptr;
    FT_Render_Mode _ft_render = FT_RENDER_MODE_NORMAL;
    FT_Int32    _ft_load_flags = 0;
    bool        _initialized = false;
    float       _on_scr_scale = std::nanf("1");
};

} // namespace Thr
