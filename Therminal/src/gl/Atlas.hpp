#pragma once

#include "Common.hpp"
#include "memory/Memory.hpp"
#include <unordered_map>

namespace Thr
{

struct GlyphInfo
{
	int	     width;
	int	     height;
	int		 bearing_x;
	int		 bearing_y;
	int		 advance;
	uint32_t id;
};

class FontAtlas
{
public:
	FontAtlas();
	/* Specify size of atlas texture in pixels.
	*/
	FontAtlas(uint atlas_width, 
			  uint atlas_height);
	~FontAtlas();

	FontAtlas(const FontAtlas&) = delete;
	FontAtlas(FontAtlas&& atlas);

	/* Initialize Atlas resources and 
	*  provide active vao.
	*  Glyph width will be adjusted automaticaly and can be obtained later.
	*/
	void init(std::shared_ptr<GLuint> vao, int glyph_height);

	FontAtlas& operator=(const FontAtlas&) = delete;
	FontAtlas& operator=(FontAtlas&& atlas);
	
	/* Add/probe UNICODE glyph */
	void addGlyph(char32_t codepoint);
	uint32_t getGlyphInfo(char32_t codepoint, GlyphInfo& info) const;

	/* Bind underlaying textures and texture buffers.
	*  Active textures can be obtained using 'getAtlasTexUnit',
	*  'getAtlasTexBufUnit' or 'getCharFormatBufUnit' methods below.
	*/
	void bindAtlas() const;
	void unbindAtlas() const;

	GLenum getAtlasTexUnit() const;
	GLenum getAtlasTexBufUnit() const;
	GLenum getCharFormatBufUnit() const;

	/* Get single glyph size in pixels */
	void getGlyphPixSize(int& width, int& height) const;
private:
	THR_INLINE void clear();

	static constexpr uint DefaultAtlasWidth  = 1024;
	static constexpr uint DefaultAtlasHeight = 1024;
	static constexpr uint DefaultGlyphHeight = 48;

	std::unordered_map<char32_t, GlyphInfo> _glyph_map;
	GLuint								    _atlas_tex_id;
	GLuint 								    _tb_buf_uvs_id;
	GLuint								    _tb_tex_uvs_id;
	GLuint 									_tb_buf_form_id;
	GLuint 									_tb_tex_form_id;
	FT_Library							    _ft_lib;
	FT_Face								    _ft_face;
	uint32_t							    _glyph_id;
	const uint 							    _atlas_width;
	const uint 							    _atlas_height;
	uint									_glyph_width;
	uint 							        _glyph_height;
	uint 									_glyph_per_tb;
	int									    _atlas_x_offset;
	int									    _atlas_y_offset;
	std::shared_ptr<GLuint> 				_vao;
	bool								    _initialized;
};

} // namespace Thr
