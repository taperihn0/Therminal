#include "Atlas.hpp"
#include "logger/Log.hpp"
#include "memory/Memory.hpp"
#include "Utils.hpp"

namespace Thr
{

/* Store UV coordinates for further lookup using texture buffer object
*  inside actual shader program.
*/
struct GlyphTextureUVs 
{
	THR_FORCEINLINE GlyphTextureUVs(float32_t u0, 
									float32_t v0, 
									float32_t u1, 
									float32_t v1);
	glm::vec4 v;
};

/* Store character-specific format data inside lookup texture buffer 
*  object for shader program.
*/
struct GlyphFormatData
{
	THR_FORCEINLINE GlyphFormatData(int u0, 
									int v0, 
									int u1, 
									int v1);
	glm::ivec4 v;
};

THR_FORCEINLINE GlyphTextureUVs::GlyphTextureUVs(float32_t u0, 
												 float32_t v0, 
												 float32_t u1, 
												 float32_t v1)
	: v(u0, v0, u1, v1)
{}

THR_FORCEINLINE GlyphFormatData::GlyphFormatData(int width, 
												 int height, 
												 int bearing_x, 
												 int bearing_y)
	: v(width, height, bearing_x, bearing_y)
{}

FontAtlas::FontAtlas()
	: FontAtlas(DefaultAtlasWidth, 
				DefaultAtlasHeight)
{}

FontAtlas::FontAtlas(uint atlas_width, 
					 uint atlas_height)
	: _atlas_tex_id(0)
	, _tb_buf_uvs_id(0)
	, _tb_tex_uvs_id(0)
	, _tb_buf_form_id(0)
	, _tb_tex_form_id(0)
	, _ft_lib(nullptr)
	, _ft_face(nullptr)
	, _glyph_id(0)
	, _atlas_width(atlas_width)
	, _atlas_height(atlas_height)
	, _glyph_width(0)
	, _glyph_height(0)
	, _glyph_per_tb(0)
	, _atlas_x_offset(0)
	, _atlas_y_offset(0)
	, _vao(nullptr)
	, _initialized(false)
{}

FontAtlas::~FontAtlas()
{
	clear();
}

FontAtlas::FontAtlas(FontAtlas&& atlas)
	: _glyph_map(std::move(atlas._glyph_map))
	, _atlas_tex_id(atlas._atlas_tex_id)
	, _tb_buf_uvs_id(atlas._tb_buf_uvs_id)
	, _tb_tex_uvs_id(atlas._tb_tex_uvs_id)
	, _tb_buf_form_id(atlas._tb_buf_form_id)
	, _tb_tex_form_id(atlas._tb_tex_form_id)
	, _ft_lib(atlas._ft_lib)
	, _ft_face(atlas._ft_face)
	, _glyph_id(atlas._glyph_id)
	, _atlas_width(atlas._atlas_width)
	, _atlas_height(atlas._atlas_height)
	, _glyph_width(0)
	, _glyph_height(atlas._glyph_height)
	, _glyph_per_tb(atlas._glyph_per_tb)
	, _atlas_x_offset(atlas._atlas_x_offset)
	, _atlas_y_offset(atlas._atlas_y_offset)
	, _vao(std::move(atlas._vao))
	, _initialized(false)
{
	atlas._atlas_tex_id = 0;
	atlas._tb_buf_uvs_id = 0;
	atlas._tb_tex_uvs_id = 0;
	atlas._ft_lib = nullptr;
	atlas._ft_face = nullptr;
	atlas._glyph_id = 0;
	atlas._vao = nullptr;
}

FontAtlas& FontAtlas::operator=(FontAtlas&& atlas)
{
	clear();
	_glyph_map = std::move(atlas._glyph_map);
	_atlas_tex_id = atlas._atlas_tex_id;
	atlas._atlas_tex_id = 0;
	_tb_buf_uvs_id = atlas._tb_buf_uvs_id;
	atlas._tb_buf_uvs_id = 0;
	_tb_tex_uvs_id = atlas._tb_tex_uvs_id;
	atlas._tb_tex_uvs_id = 0;
	_tb_buf_form_id = atlas._tb_buf_form_id;
	atlas._tb_buf_form_id = 0;
	_tb_tex_form_id = atlas._tb_tex_form_id;
	atlas._tb_tex_form_id = 0;
	_ft_lib = atlas._ft_lib;
	atlas._ft_lib = nullptr;
	_ft_face = atlas._ft_face;
	atlas._ft_face = nullptr;
	_glyph_id = atlas._glyph_id;
	atlas._glyph_id = 0;
	THR_HARD_ASSERT(_atlas_width == atlas._atlas_width);
	THR_HARD_ASSERT(_atlas_height == atlas._atlas_height);
	THR_HARD_ASSERT(_glyph_per_tb == atlas._glyph_per_tb);
	_atlas_x_offset = atlas._atlas_x_offset;
	atlas._atlas_x_offset = 0;
	_atlas_y_offset = atlas._atlas_y_offset;
	atlas._atlas_y_offset = 0;
	THR_HARD_ASSERT(_glyph_width == atlas._glyph_width);
	THR_HARD_ASSERT(_glyph_height == atlas._glyph_height);
	_vao = std::move(atlas._vao);
	atlas._vao = nullptr;
	_initialized = atlas._initialized;
	atlas._initialized = false;
	return *this;
}

void FontAtlas::addGlyph(char32_t codepoint)
{
	if (!_initialized) {
		THR_LOG_ERROR("FontAtlas subsystem is not initialized, can't add glyph");
		return;
	}

	THR_HARD_ASSERT(_vao != nullptr && glIsVertexArray(*_vao) == GL_TRUE);

	glBindVertexArray(*_vao);

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

	THR_HARD_ASSERT(_atlas_tex_id != 0 && glIsTexture(_atlas_tex_id) == GL_TRUE);

	/* Render new glyph onto the atlas */
	glBindTexture(GL_TEXTURE_2D, _atlas_tex_id);

	glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
	glTexSubImage2D(GL_TEXTURE_2D, 
					0, 
					_atlas_x_offset, 
					_atlas_y_offset,
					g->bitmap.width, 
					g->bitmap.rows, 
					GL_RED, 
					GL_UNSIGNED_BYTE,
					reinterpret_cast<const GLvoid*>(g->bitmap.buffer));

	/* Update UVs texture buffer */

	const GlyphTextureUVs glyph_uv(
		static_cast<float32_t>(_atlas_x_offset) / _atlas_width,
		static_cast<float32_t>(_atlas_y_offset) / _atlas_height,
		static_cast<float32_t>(_atlas_x_offset + g->bitmap.width) / _atlas_width,
		static_cast<float32_t>(_atlas_y_offset + g->bitmap.rows) / _atlas_height
	);

	glBindBuffer(GL_TEXTURE_BUFFER, _tb_buf_uvs_id);

	const uint32_t glyph_index = _glyph_id;

	glBufferSubData(GL_TEXTURE_BUFFER, 
				    glyph_index * sizeof(GlyphTextureUVs),
					sizeof(GlyphTextureUVs), 
					reinterpret_cast<const GLvoid*>(std::addressof(glyph_uv)));

	/* Update Character Format texture buffer */

	const GlyphFormatData glyph_format(
		static_cast<int>(g->bitmap.width), 
		static_cast<int>(g->bitmap.rows), 
		g->bitmap_left,
		g->bitmap_top
	);

	glBindBuffer(GL_TEXTURE_BUFFER, _tb_buf_form_id);
	glBufferSubData(GL_TEXTURE_BUFFER, 
				    glyph_index * sizeof(GlyphFormatData),
					sizeof(GlyphFormatData), 
					reinterpret_cast<const GLvoid*>(std::addressof(glyph_format)));	

	const GlyphInfo glyph_info = {
		static_cast<int>(g->bitmap.width), 
		static_cast<int>(g->bitmap.rows), 
		g->bitmap_left,
		g->bitmap_top,
		static_cast<int>(g->advance.x >> 6),
		_glyph_id++
	};

	_glyph_map[codepoint] = glyph_info;
	_atlas_x_offset += g->bitmap.width + 1;

	glBindTexture(GL_TEXTURE_2D, 0);
	glBindVertexArray(0);

	const GLenum err = pollGlErrors([](GLenum err) {
		THR_LOG_ERROR("OpenGL error while adding a new glyph to FontAtlas: {}", getGlErrorStr(err));
	});

	if (err != GL_NO_ERROR) {
		THR_LOG_DEBUG("FontAtlas adding glyph resulted in OpenGL error");
	}
}

uint32_t FontAtlas::getGlyphInfo(char32_t codepoint, GlyphInfo& info) const
{
	if (!_initialized) {
		THR_LOG_ERROR("FontAtlas subsystem is not initialized, can't get glyph info");
		return (info.id = static_cast<uint32_t>(-1));
	}

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
	if (!_initialized) {
		THR_LOG_ERROR("FontAtlas subsystem is not initialized, can't bind atlas");
		return;
	}

	THR_ASSERT(_vao != nullptr && glIsVertexArray(*_vao) == GL_TRUE);

	GLint vao_bound = 0;
	glGetIntegerv(GL_VERTEX_ARRAY_BINDING, std::addressof(vao_bound));

	if (vao_bound > 0 && static_cast<GLuint>(vao_bound) != *_vao) {
		THR_LOG_ERROR("VAO already bound for Atlas bind call");
		return;
	}

	glBindVertexArray(*_vao);

	THR_HARD_ASSERT(_atlas_tex_id != 0 && glIsTexture(_atlas_tex_id) == GL_TRUE);
	glActiveTexture(getAtlasTexUnit());
	glBindTexture(GL_TEXTURE_2D, _atlas_tex_id);

	THR_HARD_ASSERT(_tb_tex_uvs_id != 0 && glIsTexture(_tb_tex_uvs_id) == GL_TRUE);
	glActiveTexture(getAtlasTexBufUnit());
	glBindTexture(GL_TEXTURE_BUFFER, _tb_tex_uvs_id);

	THR_HARD_ASSERT(_tb_tex_form_id != 0 && glIsTexture(_tb_tex_form_id) == GL_TRUE);
	glActiveTexture(getCharFormatBufUnit());
	glBindTexture(GL_TEXTURE_BUFFER, _tb_tex_form_id);

	glBindVertexArray(0);

	pollGlErrors([](GLenum err) {
		THR_LOG_ERROR("OpenGL error during FontAtlas binding: {}", getGlErrorStr(err));
	});
}

void FontAtlas::unbindAtlas() const
{
	static std::array<GLuint, 3> TexIds = {
		_atlas_tex_id,
		_tb_tex_uvs_id,
		_tb_tex_form_id
	};

	glBindTextures(0, 3, TexIds.data());
}

GLenum FontAtlas::getAtlasTexUnit() const
{
	return GL_TEXTURE0;
}

GLenum FontAtlas::getAtlasTexBufUnit() const
{
	return GL_TEXTURE1;
}

GLenum FontAtlas::getCharFormatBufUnit() const
{
	return GL_TEXTURE2;
}

void FontAtlas::getGlyphPixSize(int& width, int& height) const
{
	if (!_initialized) {
		THR_LOG_ERROR("FontAtlas subsystem is not initialized, can't get glyph pixel size");
		width = 0;
		height = 0;
		return;
	}

	width = static_cast<int>(_glyph_width);
	height = static_cast<int>(_glyph_height);
}

void FontAtlas::init(std::shared_ptr<GLuint> vao, int glyph_height)
{
	if (_initialized) {
		THR_LOG_ERROR("FontAtlas subsystem is already initialized, can't initialize again");
		return;
	}

	if (FT_Init_FreeType(std::addressof(_ft_lib)) != 0) {
		THR_LOG_ERROR("Failed to initialize FreeType library");
		return;
	}

	if (FT_New_Face(_ft_lib, "Therminal/assets/fonts/DejaVuSansMono.ttf", 0, std::addressof(_ft_face)) != 0) {
		THR_LOG_ERROR("Failed to load font face");
		return;
	}

	const bool mono = (_ft_face->face_flags & FT_FACE_FLAG_FIXED_WIDTH);

	if (!mono) {
		THR_LOG_ERROR("Loaded font is not monospaced");
		return;
	}

	_glyph_height = static_cast<uint>(glyph_height);

	if (FT_Set_Pixel_Sizes(_ft_face, 0, _glyph_height)) {
		THR_LOG_ERROR("Failed to set pixel size for font face");
		return;
	}

	_glyph_width = _ft_face->size->metrics.max_advance >> 6;

	_glyph_per_tb = static_cast<uint>(_atlas_width / _glyph_width) *
					static_cast<uint>(_atlas_height / _glyph_height);

	THR_ASSERT(vao != nullptr);

	GLint vao_bound = 0;
	glGetIntegerv(GL_VERTEX_ARRAY_BINDING, std::addressof(vao_bound));

	if (vao_bound > 0 && static_cast<GLuint>(vao_bound) != *vao) {
		THR_LOG_ERROR("VAO already bound for Atlas initialization");
		return;
	}

	_vao = std::move(vao);

	glBindVertexArray(*_vao);
	THR_HARD_ASSERT(glIsVertexArray(*_vao) == GL_TRUE);

	/* Initialize atlas texture */
	glGenTextures(1, std::addressof(_atlas_tex_id));
	glBindTexture(GL_TEXTURE_2D, _atlas_tex_id);
	THR_HARD_ASSERT(_atlas_tex_id != 0 && glIsTexture(_atlas_tex_id) == GL_TRUE);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

	glTexStorage2D(GL_TEXTURE_2D, 1, GL_R8, _atlas_width, _atlas_height);

	glBindTexture(GL_TEXTURE_2D, 0);

	/* Generate and specify texture buffers for UVs codepoint lookup */
	glGenBuffers(1, std::addressof(_tb_buf_uvs_id));
	glBindBuffer(GL_TEXTURE_BUFFER, _tb_buf_uvs_id);
	THR_HARD_ASSERT(_tb_buf_uvs_id != 0 && glIsBuffer(_tb_buf_uvs_id) == GL_TRUE);

	glBufferData(GL_TEXTURE_BUFFER, 
				 _glyph_per_tb * sizeof(GlyphTextureUVs), 
				 nullptr, 
				 GL_DYNAMIC_DRAW);

	// Associate new texture with previous buffer
	glGenTextures(1, std::addressof(_tb_tex_uvs_id));
	glBindTexture(GL_TEXTURE_BUFFER, _tb_tex_uvs_id);
	glTexBuffer(GL_TEXTURE_BUFFER, GL_RGBA32F, _tb_buf_uvs_id);

	/* Generate and specify texture buffer for format specs */
	glGenBuffers(1, std::addressof(_tb_buf_form_id));
	glBindBuffer(GL_TEXTURE_BUFFER, _tb_buf_form_id);
	THR_HARD_ASSERT(_tb_buf_form_id != 0 && glIsBuffer(_tb_buf_form_id) == GL_TRUE);

	glBufferData(GL_TEXTURE_BUFFER, 
				 _glyph_per_tb * sizeof(GlyphFormatData), 
				 nullptr, 
				 GL_DYNAMIC_DRAW);

	// Associate with buffer
	glGenTextures(1, std::addressof(_tb_tex_form_id));
	glBindTexture(GL_TEXTURE_BUFFER, _tb_tex_form_id);
	glTexBuffer(GL_TEXTURE_BUFFER, GL_RGBA32I, _tb_buf_form_id);

	glBindTexture(GL_TEXTURE_BUFFER, 0);
	glBindVertexArray(0);

	const GLenum err = pollGlErrors([](GLenum err) {
		THR_LOG_ERROR("OpenGL error during FontAtlas initialization: {}", getGlErrorStr(err));
	});

	if (err != GL_NO_ERROR) {
		THR_LOG_DEBUG("FontAtlas initializing resulted in OpenGL error");
	}

	_initialized = true;
}

THR_INLINE void FontAtlas::clear()
{
	if (_atlas_tex_id != 0 && glIsTexture(_atlas_tex_id) == GL_TRUE) {
		glDeleteTextures(1, std::addressof(_atlas_tex_id));
	}

	if (_tb_buf_uvs_id != 0 && glIsBuffer(_tb_buf_uvs_id) == GL_TRUE) {
		glDeleteBuffers(1, std::addressof(_tb_buf_uvs_id));
	}

	if (_tb_buf_form_id != 0 && glIsBuffer(_tb_buf_form_id) == GL_TRUE) {
		glDeleteBuffers(1, std::addressof(_tb_buf_form_id));
	}

	if (_ft_face != nullptr) {
		FT_Done_Face(_ft_face);
	}

	if (_ft_lib != nullptr) {
		FT_Done_FreeType(_ft_lib);
	}

	_initialized = false;
}

} // namespace Thr
