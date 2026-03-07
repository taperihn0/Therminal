#pragma once

#include "Common.hpp"
#include <unordered_map>

namespace Thr
{

struct GlyphInfo
{
	uint	 width;
	uint	 height;
	int		 bearing_x;
	int		 bearing_y;
	int		 pix_advance;
	uint32_t id;
};

class FontAtlas
{
public:
	FontAtlas();
	FontAtlas(uint atlas_width, uint atlas_height);
	~FontAtlas();

	FontAtlas(const FontAtlas&) = delete;
	FontAtlas(FontAtlas&& atlas);

	FontAtlas& operator=(const FontAtlas&) = delete;
	FontAtlas& operator=(FontAtlas&& atlas);

	void addGlyph(char32_t codepoint);
	uint32_t getGlyphInfo(char32_t codepoint, GlyphInfo& info) const;

	void bindAtlas() const;
	void unbindAtlas() const;

	uint getActiveTextureUnit() const;
	uint getActiveTextureBuffer() const;
private:
	void init();
	THR_INLINE void clear();

	std::unordered_map<char32_t, GlyphInfo> _glyph_map;
	GLuint								    _tex_id;
	GLuint 								    _tbo_buf_id;
	GLuint								    _tbo_tex_id;
	FT_Library							    _ft_lib;
	FT_Face								    _ft_face;
	uint32_t							    _curr_glyph_id;
	const uint 							    _atlas_width;
	const uint 							    _atlas_height;
	const uint 							    _glyph_per_tbo;
	int									    _atlas_x_offset;
	int									    _atlas_y_offset;
	const uint 							    _glyph_height;
};

} // namespace Thr
