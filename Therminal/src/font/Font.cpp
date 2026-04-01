#include "Font.hpp"
#include "logger/Log.hpp"

namespace Thr
{

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

	const bool mono = (_ft_face->face_flags & FT_FACE_FLAG_FIXED_WIDTH);

	if (!mono) {
		THR_LOG_ERROR("Loaded font is not monospaced");
		return;
	}

	if (FT_Set_Pixel_Sizes(_ft_face, 0, req_glyph_height)) {
		THR_LOG_ERROR("Failed to set pixel size for font face");
		return;
	}

    _initialized = true;
}

FT_GlyphSlot Font::getGlyphOf(char32_t ch)
{
	if (_ft_face == nullptr || 
		FT_Load_Char(_ft_face, ch, FT_LOAD_RENDER)) {
		THR_LOG_ERROR("Failed to load glyph for '{}'", ch);
        return nullptr;
	}

    return _ft_face->glyph;
}

FT_Size_Metrics Font::getGlyphMetrics() const
{
	return _ft_face->size->metrics;
}

bool Font::isReady() const
{
    return _initialized;
}

} // namespace Thr
