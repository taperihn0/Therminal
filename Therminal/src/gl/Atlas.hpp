#pragma once

#include "Common.hpp"
#include "memory/Memory.hpp"
#include <unordered_map>

namespace Thr
{

struct GlyphInfo
{
	uint	 width;
	uint	 height;
	int		 bearing_x;
	int		 bearing_y;
	int		 advance;
	uint32_t id;
};

class FontAtlas
{
public:
	FontAtlas();
	FontAtlas(uint atlas_width, 
			  uint atlas_height,
			  uint glyph_height);
	~FontAtlas();

	FontAtlas(const FontAtlas&) = delete;
	FontAtlas(FontAtlas&& atlas);

	void init(std::shared_ptr<GLuint> vao);

	FontAtlas& operator=(const FontAtlas&) = delete;
	FontAtlas& operator=(FontAtlas&& atlas);

	void addGlyph(char32_t codepoint);
	uint32_t getGlyphInfo(char32_t codepoint, GlyphInfo& info) const;

	void bindAtlas() const;
	void unbindAtlas() const;

	GLenum getActiveTextureUnit() const;
	GLenum getActiveTextureBufferUnit() const;

	void getGlyphPixSize(uint& width, uint& height) const;
private:
	THR_INLINE void clear();

	static constexpr uint DefaultAtlasWidth  = 1024;
	static constexpr uint DefaultAtlasHeight = 1024;
	static constexpr uint DefaultGlyphHeight = 48;

	std::unordered_map<char32_t, GlyphInfo> _glyph_map;
	GLuint								    _atlas_tex_id;
	GLuint 								    _tb_buf_id;
	GLuint								    _tb_tex_id;
	FT_Library							    _ft_lib;
	FT_Face								    _ft_face;
	uint32_t							    _glyph_id;
	const uint 							    _atlas_width;
	const uint 							    _atlas_height;
	uint									_glyph_width;
	const uint 							    _glyph_height;
	uint 									_glyph_per_tb;
	int									    _atlas_x_offset;
	int									    _atlas_y_offset;
	std::shared_ptr<GLuint> 				_vao;
	bool								    _initialized;
};

} // namespace Thr
