#include "Font.hpp"
#include "logger/Log.hpp"

namespace Thr
{

constexpr THR_FORCEINLINE int32_t fracPixelToPixel(FT_Int frac)
{
	return frac >> 6;
}

constexpr THR_FORCEINLINE glm::i32vec2 fracPixel2ToPixel2(FT_Vector v) 
{
	return glm::i32vec2{v.x >> 6, v.y >> 6};
}

Font::Font(Font&& f)
{
    _ft_lib = f._ft_lib;
    f._ft_lib = nullptr;
    _ft_face = f._ft_face;
    f._ft_face = nullptr;
}
    
Font::~Font()
{
    if (_ft_face != nullptr) {
		FT_Done_Face(_ft_face);
	}

	if (_ft_lib != nullptr) {
		FT_Done_FreeType(_ft_lib);
	}
}

Font& Font::operator=(Font&& f)
{
    _ft_lib = f._ft_lib;
    f._ft_lib = nullptr;
    _ft_face = f._ft_face;
    f._ft_face = nullptr;
    return *this;
}

void Font::init(const FilePath& font_path, int req_glyph_height)
{
    if (_initialized) {
        THR_LOG_ERROR("FreeType font already initialized");
        return;
    }

	if (FT_Init_FreeType(std::addressof(_ft_lib)) != 0) {
		THR_LOG_ERROR("Failed to initialize FreeType library");
		return;
	}

	if (FT_New_Face(_ft_lib, font_path.toCStr(), 0, std::addressof(_ft_face)) != 0) {
		THR_LOG_ERROR("Failed to load font face");
		return;
	}

	if (!FT_IS_FIXED_WIDTH(_ft_face)) {
		THR_LOG_ERROR("Loaded font is not monospaced");
		return;
	}

	if (!selectHeight(req_glyph_height)) {
		THR_LOG_FATAL("Font sizing failed");
		return;
	}

    _initialized = true;
}

void Font::markScale(float on_scr_scale)
{
	if (std::isnan(on_scr_scale)) {
		THR_LOG_ERROR("Invalid screen scale for font");
		return;
	}

	_on_scr_scale = on_scr_scale;
}

float Font::getScale() const
{
	if (std::isnan(_on_scr_scale)) {
		THR_LOG_ERROR("Scale not marked");
		return 0.f;
	}

	return _on_scr_scale;
}

bool Font::getGlyphOf(char32_t ch, Font::Glyph& glyph)
{
	if (_ft_face == nullptr) {
		THR_LOG_ERROR("Invalid font face");
		return false;
	}

	const uint ft_index = FT_Get_Char_Index(_ft_face, ch);

	if (!ft_index) 
		return false;

	if (FT_Load_Glyph(_ft_face, ft_index, _ft_load_flags))
		return false;

	if (FT_Render_Glyph(_ft_face->glyph, _ft_render))
		return false;

    glyph = {
		_ft_face->glyph->glyph_index,
		_ft_face->glyph->metrics,
		fracPixel2ToPixel2(_ft_face->glyph->advance),
		_ft_face->glyph->bitmap,
		_ft_face->glyph->bitmap_left,
		_ft_face->glyph->bitmap_top,
	};

	return true;
}

bool Font::getGlyphMetrics(Font::Metrics& metrics) const
{
	if (_ft_face == nullptr) {
		THR_LOG_ERROR("Failed to get glyph metrics");
		return false;
	}

	metrics = {
		fracPixelToPixel(_ft_face->size->metrics.max_advance),
		fracPixelToPixel(_ft_face->size->metrics.height),
		fracPixelToPixel(_ft_face->size->metrics.ascender)
	};

	return true;
}

bool Font::isReady() const
{
    return _initialized;
}

bool Font::isColoredFont() const
{
	static constexpr FT_ULong ColoredBitmapTag = FT_MAKE_TAG('C', 'B', 'D', 'T');
	FT_ULong len = 0;

	const bool status = FT_Load_Sfnt_Table(_ft_face, ColoredBitmapTag, 
								  		   0, nullptr, std::addressof(len));

	if (!status) {
		THR_LOG_FATAL("Failed to load SFNT table for font");
		return false;
	}

	return len > 0;
}

bool Font::selectHeight(int req_glyph_height)
{
	if (!isColoredFont()) /* Regular font */ {
		if (FT_Set_Pixel_Sizes(_ft_face, 0, req_glyph_height)) {
			THR_LOG_ERROR("Failed to set pixel size for font face");
			return false;
		}

		_ft_load_flags |= FT_LOAD_COLOR;
		return true;
	}

	/* Colored font*/
	if (!_ft_face->num_fixed_sizes)
		return false;

	int pick = 0;
	int min_diff = std::abs(req_glyph_height - _ft_face->available_sizes[0].height);

	for (int i = 1; i < _ft_face->num_fixed_sizes; i++) {
		const int diff = std::abs(req_glyph_height - _ft_face->available_sizes[i].height);

		if (diff < min_diff) {
			min_diff = diff;
			pick = i;
		}
	}

	return FT_Select_Size(_ft_face, pick);
}

} // namespace Thr
