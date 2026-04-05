#pragma once

#include "Common.hpp"
#include "Utils.hpp"
#include "memory/Memory.hpp"
#include "font/Font.hpp"
#include <unordered_map>
#include <utility>

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
	FontAtlas(uint atlas_width, uint atlas_height);
	~FontAtlas();

	FontAtlas(const FontAtlas&) = delete;
	FontAtlas(FontAtlas&& atlas);

	FontAtlas& operator=(const FontAtlas&) = delete;
	FontAtlas& operator=(FontAtlas&& atlas);

	/* Initialize Atlas resources and provide active vao.
	*  Glyph width will be adjusted automaticaly and can be obtained later.
	*/
	void init(std::shared_ptr<Font>& font, 
			  std::shared_ptr<TexturesUnitsContext>& act_tex_ctx);
	
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

	bool isReady() const;
private:
	THR_INLINE void clear();

	static constexpr uint _DefaultAtlasWidth  = 256;
	static constexpr uint _DefaultAtlasHeight = 256;
	static constexpr uint _DefaultGlyphHeight = 48;

	std::unordered_map<char32_t, GlyphInfo> _glyph_map;
	GLuint									_atlas_tex_id;
	GLuint 									_tb_buf_uvs_id;
	GLuint									_tb_tex_uvs_id;
	GLuint 									_tb_buf_form_id;
	GLuint 									_tb_tex_form_id;
	uint32_t								_glyph_id;
	uint 									_atlas_width;
	uint 									_atlas_height;
	uint 									_glyph_per_tb;
	int										_atlas_x_offset;
	int										_atlas_y_offset;
	bool									_initialized;
	std::shared_ptr<Font> 					_font;
	std::shared_ptr<TexturesUnitsContext>   _act_tex_ctx;
	GLenum 									_atlas_tex_unit;
	GLenum 									_atlas_lookup_unit;
	GLenum 									_char_lookup_unit;
};

/* Extended atlas class.
*  We store double internal storage textures for 
*  regular and colored fonts.
*/
class DoubleAtlas
{
public:
	DoubleAtlas() = default;
	DoubleAtlas(const DoubleAtlas&) = delete;
	DoubleAtlas(DoubleAtlas&& atlas) = default;
	DoubleAtlas(uint atlas_width, uint atlas_height);

	DoubleAtlas& operator=(const DoubleAtlas&) = delete;
	DoubleAtlas& operator=(DoubleAtlas&& atlas) = default;
	
	void init(std::shared_ptr<Font>& regular_font,
			  std::shared_ptr<Font>& colored_font);
		
	void addGlyph(char32_t codepoint);
	uint32_t getGlyphInfo(char32_t codepoint, GlyphInfo& info) const;

	void bindAtlases() const;
	void unbindAtlases() const;

	template <size_t Atlas>
	GLenum getAtlasTexUnit() const;
	template <size_t Atlas>
	GLenum getAtlasTexBufUnit() const;
	template <size_t Atlas>
	GLenum getCharFormatBufUnit() const;

	bool isReady() const;
	
	static constexpr size_t GetRegularAtlas = 0;
	static constexpr size_t GetColoredAtlas = 1;
private:
	TexturesUnitsContext 	 		_act_tex_ctx;
	std::pair<FontAtlas, FontAtlas> _atlas_pair;
};

} // namespace Thr
