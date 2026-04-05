#include "Font.hpp"
#include "logger/Log.hpp"

namespace Thr
{

constexpr THR_FORCEINLINE int32_t fracPixelToPixel(FT_Int frac)
{
	return frac >> 6;
}

constexpr THR_FORCEINLINE glm::i32vec2 fracPixelToPixel(FT_Vector v) 
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

void Font::init(const FilePath& font_path, int glyph_height)
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

	const bool mono = (_ft_face->face_flags & FT_FACE_FLAG_FIXED_WIDTH);

	if (!mono) {
		THR_LOG_ERROR("Loaded font is not monospaced");
		return;
	}

	if (FT_Set_Pixel_Sizes(_ft_face, 0, glyph_height)) {
		THR_LOG_ERROR("Failed to set pixel size for font face");
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

glm::ivec2 toWraperVector(const FT_Vector& ft_vec) 
{
	return glm::ivec2{ ft_vec.x, ft_vec.y };
}

bool Font::getGlyphOf(char32_t ch, Font::Glyph& glyph)
{
	if (_ft_face == nullptr || 
		FT_Load_Char(_ft_face, ch, FT_LOAD_RENDER)) {
		THR_LOG_ERROR("Failed to load glyph for '{}'", ch);
		return false;
	}

    glyph = {
		_ft_face->glyph->glyph_index,
		_ft_face->glyph->metrics,
		fracPixelToPixel(_ft_face->glyph->advance),
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

} // namespace Thr
