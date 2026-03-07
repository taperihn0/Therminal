#include "Atlas.hpp"
#include "logger/Log.hpp"
#include "memory/Memory.hpp"

namespace Thr
{

/* Store UV coordinates for each glyph for indexing Atlas texture
*  inside shader program.
*/
struct AtlasUV {
	float32_t u0, v0;
	float32_t u1, v1;
};

FontAtlas::FontAtlas()
	: FontAtlas(1024, 1024)
{}

FontAtlas::FontAtlas(uint atlas_width, uint atlas_height)
	: _tex_id(0)
	, _tbo_buf_id(0)
	, _ft_lib(nullptr)
	, _ft_face(nullptr)
	, _curr_glyph_id(0)
	, _atlas_width(atlas_width)
	, _atlas_height(atlas_height)
	, _glyph_per_tbo(4096)
	, _atlas_x_offset(0)
	, _atlas_y_offset(0)
	, _glyph_height(48)
{}

FontAtlas::~FontAtlas()
{
	clear();
}

FontAtlas::FontAtlas(FontAtlas&& atlas)
	: _glyph_map(std::move(atlas._glyph_map))
	, _tex_id(atlas._tex_id)
	, _tbo_buf_id(atlas._tbo_buf_id)
	, _ft_lib(atlas._ft_lib)
	, _ft_face(atlas._ft_face)
	, _curr_glyph_id(atlas._curr_glyph_id)
	, _atlas_width(atlas._atlas_width)
	, _atlas_height(atlas._atlas_height)
	, _glyph_per_tbo(atlas._glyph_per_tbo)
	, _atlas_x_offset(atlas._atlas_x_offset)
	, _atlas_y_offset(atlas._atlas_y_offset)
	, _glyph_height(atlas._glyph_height)
{
	atlas._tex_id = 0;
	atlas._tbo_buf_id = 0;
	atlas._ft_lib = nullptr;
	atlas._ft_face = nullptr;
	atlas._curr_glyph_id = 0;
}

FontAtlas& FontAtlas::operator=(FontAtlas&& atlas)
{
	clear();
	_glyph_map = std::move(atlas._glyph_map);
	_tex_id = atlas._tex_id;
	atlas._tex_id = 0;
	_ft_lib = atlas._ft_lib;
	atlas._ft_lib = nullptr;
	_ft_face = atlas._ft_face;
	atlas._ft_face = nullptr;
	_curr_glyph_id = atlas._curr_glyph_id;
	atlas._curr_glyph_id = 0;
	THR_HARD_ASSERT(_atlas_width == atlas._atlas_width);
	THR_HARD_ASSERT(_atlas_height == atlas._atlas_height);
	THR_HARD_ASSERT(_glyph_per_tbo == atlas._glyph_per_tbo);
	_atlas_x_offset = atlas._atlas_x_offset;
	atlas._atlas_x_offset = 0;
	_atlas_y_offset = atlas._atlas_y_offset;
	atlas._atlas_y_offset = 0;
	THR_HARD_ASSERT(_glyph_height == atlas._glyph_height);
	return *this;
}

void FontAtlas::addGlyph(char32_t codepoint)
{
	GLint vao = 0;
	glGetIntegerv(GL_VERTEX_ARRAY_BINDING, &vao);

	if (!vao) {
		THR_LOG_ERROR("No VAO is currently bound. Cannot initialize FontAtlas.");
		return;
	}

	if (_glyph_map.count(codepoint))
		return;

	if (_ft_face == nullptr || 
		FT_Load_Char(_ft_face, codepoint, FT_LOAD_RENDER)) {
		THR_LOG_ERROR("Failed to load glyph with codepoint {}", codepoint);
		return;
	}

	const FT_GlyphSlot g = _ft_face->glyph;

	if (_atlas_y_offset + g->bitmap.rows >= _atlas_height) {
		THR_LOG_ERROR("Font atlas is full, cannot add more glyphs");
		return;
	}

	if (_atlas_x_offset + g->bitmap.width >= _atlas_width) {
		_atlas_x_offset = 0;
		_atlas_y_offset += _glyph_height + 1;
	}

	THR_HARD_ASSERT(_tex_id != 0 && glIsTexture(_tex_id) == GL_TRUE);

	glBindTexture(GL_TEXTURE_2D, _tex_id);

	glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
	glTexSubImage2D(GL_TEXTURE_2D, 
					0, 
					_atlas_x_offset, 
					_atlas_y_offset,
					g->bitmap.width, 
					g->bitmap.rows, 
					GL_RED, 
					GL_UNSIGNED_BYTE,
					reinterpret_cast<const void*>(g->bitmap.buffer));

	const AtlasUV glyph_uv = {
		static_cast<float32_t>(_atlas_x_offset)					  / _atlas_width,
		static_cast<float32_t>(_atlas_y_offset)					  / _atlas_height,
		static_cast<float32_t>(_atlas_x_offset + g->bitmap.width) / _atlas_width,
		static_cast<float32_t>(_atlas_y_offset + g->bitmap.rows)  / _atlas_height
	};

	const uint32_t glyph_index = _curr_glyph_id;

	glBindTexture(GL_TEXTURE_BUFFER, _tbo_buf_id);
	glBufferSubData(GL_TEXTURE_BUFFER, 
				    glyph_index * sizeof(AtlasUV),
					sizeof(AtlasUV), 
					reinterpret_cast<const void*>(std::addressof(glyph_uv)));

	const GlyphInfo glyph_info = {
		g->bitmap.width,
		g->bitmap.rows,
		g->bitmap_left,
		g->bitmap_top,
		g->advance.x >> 6,
		_curr_glyph_id++
	};

	_glyph_map[codepoint] = glyph_info;
	_atlas_x_offset += g->bitmap.width + 1;

	glBindTexture(GL_TEXTURE_2D, 0);
}

uint32_t FontAtlas::getGlyphInfo(char32_t codepoint, GlyphInfo& info) const
{
	const auto it = _glyph_map.find(codepoint);

	if (it != _glyph_map.end()) { // found
		info = it->second;
		return info.id;
	}
	
	memSet(std::addressof(info), 0, sizeof(GlyphInfo));
	return (info.id = static_cast<uint32_t>(-1));
}

void FontAtlas::bindAtlas() const
{
	GLint vao = 0;
	glGetIntegerv(GL_VERTEX_ARRAY_BINDING, &vao);

	if (!vao) {
		THR_LOG_ERROR("No VAO is currently bound. Cannot initialize FontAtlas.");
		return;
	}

	THR_HARD_ASSERT(_tex_id != 0 && glIsTexture(_tex_id) == GL_TRUE);

	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, _tex_id);

	glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_BUFFER, _tbo_tex_id);
}

void FontAtlas::unbindAtlas() const
{
	glBindTexture(GL_TEXTURE_2D, 0);
}

uint FontAtlas::getActiveTextureUnit() const
{
	return 0; // Since we always bind to GL_TEXTURE0 for the texture
}

uint FontAtlas::getActiveTextureBuffer() const
{
	return 1; // Since we always bind to GL_TEXTURE1 for the texture buffer
}

void FontAtlas::init()
{
	GLint vao = 0;
	glGetIntegerv(GL_VERTEX_ARRAY_BINDING, &vao);

	if (!vao) {
		THR_LOG_ERROR("No VAO is currently bound. Cannot initialize FontAtlas.");
		return;
	}

	THR_HARD_ASSERT(_tex_id == 0 && 
					_tbo_buf_id == 0 && 
					_ft_lib == nullptr && 
					_ft_face == nullptr);

	glGenTextures(1, std::addressof(_tex_id));
	THR_HARD_ASSERT(_tex_id != 0 && glIsTexture(_tex_id) == GL_TRUE);
	
	glBindTexture(GL_TEXTURE_2D, _tex_id);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

	glTexStorage2D(GL_TEXTURE_2D, 1, GL_R8, 1024, 1024);

	glBindTexture(GL_TEXTURE_2D, 0);

	glGenBuffers(1, &_tbo_buf_id);
	glBindBuffer(GL_TEXTURE_BUFFER, _tbo_buf_id);

	THR_HARD_ASSERT(_tbo_buf_id != 0 && glIsTexture(_tbo_buf_id) == GL_TRUE);
	glBindTexture(GL_TEXTURE_BUFFER, _tbo_buf_id);
	glBufferData(GL_TEXTURE_BUFFER, _glyph_per_tbo * sizeof(AtlasUV), nullptr, GL_DYNAMIC_DRAW);

	glGenTextures(1, &_tbo_tex_id);
	glBindTexture(GL_TEXTURE_BUFFER, _tbo_tex_id);
	glTexBuffer(GL_TEXTURE_BUFFER, GL_RGBA32F, _tbo_buf_id);

	glBindTexture(GL_TEXTURE_BUFFER, 0);

	if (FT_Init_FreeType(std::addressof(_ft_lib)) != 0) {
		THR_LOG_ERROR("Failed to initialize FreeType library");
		return;
	}

	if (FT_New_Face(_ft_lib, "assets/fonts/DejaVuSansMono.ttf", 0, std::addressof(_ft_face)) != 0) {
		THR_LOG_ERROR("Failed to load font face");
		return;
	}

	bool mono = (_ft_face->face_flags & FT_FACE_FLAG_FIXED_WIDTH);

	if (!mono) {
		THR_LOG_ERROR("Loaded font is not monospaced");
		return;
	}

	if (FT_Set_Pixel_Sizes(_ft_face, 0, _glyph_height)) {
		THR_LOG_ERROR("Failed to set pixel size for font face");
		return;
	}
}

THR_INLINE void FontAtlas::clear()
{
	if (_tex_id != 0 && glIsTexture(_tex_id) == GL_TRUE) {
		glDeleteTextures(1, std::addressof(_tex_id));
	}

	if (_tbo_buf_id != 0 && glIsBuffer(_tbo_buf_id) == GL_TRUE) {
		glDeleteBuffers(1, std::addressof(_tbo_buf_id));
	}

	if (_ft_lib != nullptr) {
		FT_Done_FreeType(_ft_lib);
	}

	if (_ft_face != nullptr) {
		FT_Done_Face(_ft_face);
	}
}

} // namespace Thr
