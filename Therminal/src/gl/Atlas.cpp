#include "Atlas.hpp"
#include "logger/Log.hpp"
#include "memory/Memory.hpp"
#include "Utils.hpp"
#include "char/Char.hpp"

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
	: FontAtlas(_DefaultAtlasWidth, 
				_DefaultAtlasHeight)
{}

FontAtlas::FontAtlas(uint atlas_width, 
					 uint atlas_height)
	: _atlas_tex_id(0)
	, _tb_buf_uvs_id(0)
	, _tb_tex_uvs_id(0)
	, _tb_buf_form_id(0)
	, _tb_tex_form_id(0)
	, _glyph_id(0)
	, _atlas_width(atlas_width)
	, _atlas_height(atlas_height)
	, _glyph_per_tb(0)
	, _atlas_x_offset(0)
	, _atlas_y_offset(0)
	, _initialized(false)
	, _act_tex_ctx(nullptr)
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
	, _glyph_id(atlas._glyph_id)
	, _atlas_width(atlas._atlas_width)
	, _atlas_height(atlas._atlas_height)
	, _glyph_per_tb(atlas._glyph_per_tb)
	, _atlas_x_offset(atlas._atlas_x_offset)
	, _atlas_y_offset(atlas._atlas_y_offset)
	, _initialized(false)
	, _font(std::move(atlas._font))
{
	atlas._atlas_tex_id = 0;
	atlas._tb_buf_uvs_id = 0;
	atlas._tb_tex_uvs_id = 0;
	atlas._glyph_id = 0;
	atlas._tb_buf_form_id = 0;
	atlas._tb_tex_form_id = 0;
}

FontAtlas& FontAtlas::operator=(FontAtlas&& atlas)
{
	if (this == std::addressof(atlas))
		return *this;

	clear();

	_glyph_map = std::move(atlas._glyph_map);
	_font      = std::move(atlas._font);

	_atlas_tex_id   = atlas._atlas_tex_id;
	_tb_buf_uvs_id  = atlas._tb_buf_uvs_id;
	_tb_tex_uvs_id  = atlas._tb_tex_uvs_id;
	_tb_buf_form_id = atlas._tb_buf_form_id;
	_tb_tex_form_id = atlas._tb_tex_form_id; 
	_glyph_id       = atlas._glyph_id;
	_atlas_width    = atlas._atlas_width;
	_atlas_height   = atlas._atlas_height;
	_glyph_per_tb   = atlas._glyph_per_tb;
	_atlas_x_offset = atlas._atlas_x_offset;
	_atlas_y_offset = atlas._atlas_y_offset;
	_initialized    = atlas._initialized;

	atlas._atlas_tex_id   = 0;
	atlas._tb_buf_uvs_id  = 0;
	atlas._tb_tex_uvs_id  = 0;
	atlas._tb_buf_form_id = 0;
	atlas._tb_tex_form_id = 0;
	atlas._glyph_id       = 0;
	atlas._initialized    = false;
	atlas._atlas_width    = 0;
	atlas._atlas_height   = 0;

	return *this;
}

void FontAtlas::addGlyph(char32_t codepoint)
{
	if (!_initialized) {
		THR_LOG_ERROR("FontAtlas subsystem is not initialized, can't add glyph");
		return;
	}

	GLint vao_bound = 0;
	glGetIntegerv(GL_VERTEX_ARRAY_BINDING, std::addressof(vao_bound));

	if (!vao_bound) {
		THR_LOG_ERROR("addGlyph requires bounded VAO");
		return;
	}

	if (_glyph_map.count(codepoint))
		return;

	Font::Glyph g;

	{
		const bool success = _font->getGlyphOf(codepoint, g);

		if (!success) {
			THR_LOG_FATAL("Failed to load glyph with codepoint {}", codepoint);
			return;
		}
	}

	if (_atlas_y_offset + g.ft_bitmap.rows >= _atlas_height) {
		THR_LOG_ERROR("Font atlas is full, cannot add more glyphs");
		return;
	}

	{
		Font::Metrics glyph_metrics;
		const bool success = _font->getGlyphMetrics(glyph_metrics);

		if (!success) {
			THR_LOG_FATAL("Failed to get glyph metrics");
			return;
		}

		if (_atlas_x_offset + g.ft_bitmap.width >= _atlas_width) {
			_atlas_x_offset = 0;
			_atlas_y_offset += glyph_metrics.height_pix + 1;
		}
	}

	THR_HARD_ASSERT(_atlas_tex_id != 0 && glIsTexture(_atlas_tex_id) == GL_TRUE);

	/* Render new glyph onto the atlas */
	glBindTexture(GL_TEXTURE_2D, _atlas_tex_id);

	glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
	glTexSubImage2D(GL_TEXTURE_2D, 
					0, 
					_atlas_x_offset, 
					_atlas_y_offset,
					g.ft_bitmap.width, 
					g.ft_bitmap.rows, 
					GL_RED, 
					GL_UNSIGNED_BYTE,
					reinterpret_cast<const GLvoid*>(g.ft_bitmap.buffer));

	/* Update UVs texture buffer */

	const GlyphTextureUVs glyph_uv(
		static_cast<float32_t>(_atlas_x_offset) / _atlas_width,
		static_cast<float32_t>(_atlas_y_offset) / _atlas_height,
		static_cast<float32_t>(_atlas_x_offset + g.ft_bitmap.width) / _atlas_width,
		static_cast<float32_t>(_atlas_y_offset + g.ft_bitmap.rows) / _atlas_height
	);

	glBindBuffer(GL_TEXTURE_BUFFER, _tb_buf_uvs_id);

	const uint32_t glyph_index = _glyph_id;

	glBufferSubData(GL_TEXTURE_BUFFER, 
				    glyph_index * sizeof(GlyphTextureUVs),
					sizeof(GlyphTextureUVs), 
					reinterpret_cast<const GLvoid*>(std::addressof(glyph_uv)));

	/* Update Character Format texture buffer */

	const GlyphFormatData glyph_format(
		static_cast<int>(g.ft_bitmap.width), 
		static_cast<int>(g.ft_bitmap.rows), 
		g.bearing_x_pix,
		g.bearing_y_pix
	);

	glBindBuffer(GL_TEXTURE_BUFFER, _tb_buf_form_id);
	glBufferSubData(GL_TEXTURE_BUFFER, 
				    glyph_index * sizeof(GlyphFormatData),
					sizeof(GlyphFormatData), 
					reinterpret_cast<const GLvoid*>(std::addressof(glyph_format)));	

	const GlyphInfo glyph_info {
		static_cast<int>(g.ft_bitmap.width), 
		static_cast<int>(g.ft_bitmap.rows), 
		g.bearing_x_pix,
		g.bearing_y_pix,
		g.advance_pix.x,
		_glyph_id++
	};

	_glyph_map[codepoint] = glyph_info;
	_atlas_x_offset += g.ft_bitmap.width + 1;

	glBindTexture(GL_TEXTURE_2D, 0);

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

	GLint vao_bound = 0;
	glGetIntegerv(GL_VERTEX_ARRAY_BINDING, std::addressof(vao_bound));

	if (!vao_bound) {
		THR_LOG_ERROR("bindAtlas requires bounded VAO");
		return;
	}

	THR_HARD_ASSERT(_atlas_tex_id != 0 && glIsTexture(_atlas_tex_id) == GL_TRUE);
	glActiveTexture(getAtlasTexUnit());
	glBindTexture(GL_TEXTURE_2D, _atlas_tex_id);

	THR_HARD_ASSERT(_tb_tex_uvs_id != 0 && glIsTexture(_tb_tex_uvs_id) == GL_TRUE);
	glActiveTexture(getAtlasTexBufUnit());
	glBindTexture(GL_TEXTURE_BUFFER, _tb_tex_uvs_id);

	THR_HARD_ASSERT(_tb_tex_form_id != 0 && glIsTexture(_tb_tex_form_id) == GL_TRUE);
	glActiveTexture(getCharFormatBufUnit());
	glBindTexture(GL_TEXTURE_BUFFER, _tb_tex_form_id);

	pollGlErrors([](GLenum err) {
		THR_LOG_ERROR("OpenGL error during FontAtlas binding: {}", getGlErrorStr(err));
	});
}

void FontAtlas::unbindAtlas() const
{
	GLint vao_bound = 0;
	glGetIntegerv(GL_VERTEX_ARRAY_BINDING, std::addressof(vao_bound));

	if (!vao_bound) {
		THR_LOG_ERROR("unbindAtlas requires bounded VAO");
		return;
	}
	
	static std::array<GLuint, 3> TexIds = {
		_atlas_tex_id,
		_tb_tex_uvs_id,
		_tb_tex_form_id
	};

	glBindTextures(0, 3, TexIds.data());
}

GLenum FontAtlas::getAtlasTexUnit() const
{
	return _atlas_tex_unit;
}

GLenum FontAtlas::getAtlasTexBufUnit() const
{
	return _atlas_lookup_unit;
}

GLenum FontAtlas::getCharFormatBufUnit() const
{
	return _char_lookup_unit;
}

bool FontAtlas::isReady() const
{
	return _initialized;
}

void FontAtlas::init(std::shared_ptr<Font>& font,
					 std::shared_ptr<TexturesUnitsContext>& act_tex_ctx)
{
	if (_initialized) {
		THR_LOG_ERROR("FontAtlas subsystem is already initialized, can't initialize again");
		return;
	}

	_font = font;

	if (!_font->isReady()) {
		THR_LOG_ERROR("Font is not initialized");
		return;
	}

	{
		Font::Metrics glyph_metrics;
		const bool success = _font->getGlyphMetrics(glyph_metrics);

		if (!success) {
			THR_LOG_FATAL("Failed to get glyph metrics");
			return;
		}

		_glyph_per_tb = static_cast<uint>(_atlas_width / glyph_metrics.max_advance_pix) *
						static_cast<uint>(_atlas_height / glyph_metrics.height_pix);
	}

	GLint vao_bound = 0;
	glGetIntegerv(GL_VERTEX_ARRAY_BINDING, std::addressof(vao_bound));

	if (!vao_bound) {
		THR_LOG_ERROR("Atlas initialization requires bounded VAO");
		return;
	}

	/* Initialize atlas texture */
	glGenTextures(1, std::addressof(_atlas_tex_id));
	glBindTexture(GL_TEXTURE_2D, _atlas_tex_id);
	THR_HARD_ASSERT(_atlas_tex_id != 0 && glIsTexture(_atlas_tex_id) == GL_TRUE);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
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

	_act_tex_ctx = act_tex_ctx;

	if (!_act_tex_ctx->getTextureUnit(_atlas_tex_unit) ||
		!_act_tex_ctx->getTextureUnit(_atlas_lookup_unit) ||
		!_act_tex_ctx->getTextureUnit(_char_lookup_unit)) {
		THR_LOG_FATAL("Failed to setup texture units");
		return;
	}

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

	_initialized = false;
}

/**** Double-atlas implementation ****/

DoubleAtlas::DoubleAtlas(uint atlas_width, uint atlas_height)
	: _atlas_pair{ FontAtlas(atlas_width, atlas_height), 
			  	   FontAtlas(atlas_width, atlas_height) }
{}

void DoubleAtlas::init(std::shared_ptr<Font>& regular_font,
					   std::shared_ptr<Font>& colored_font)
{
	std::shared_ptr<TexturesUnitsContext> act_tex_ctx_ptr = 
		std::make_shared<TexturesUnitsContext>(_act_tex_ctx);

	std::get<GetRegularAtlas>(_atlas_pair).init(regular_font, act_tex_ctx_ptr);
	std::get<GetColoredAtlas>(_atlas_pair).init(colored_font, act_tex_ctx_ptr);
}
	
void DoubleAtlas::addGlyph(char32_t codepoint)
{
	const Char32 ch(codepoint);

	if (ch.isEmoji())
		std::get<GetColoredAtlas>(_atlas_pair).addGlyph(ch);
	else
		std::get<GetRegularAtlas>(_atlas_pair).addGlyph(ch);
}

uint32_t DoubleAtlas::getGlyphInfo(char32_t codepoint, GlyphInfo& info) const
{
	const Char32 ch(codepoint);

	if (ch.isEmoji())
		return std::get<GetColoredAtlas>(_atlas_pair).getGlyphInfo(ch, info);
	else
		return std::get<GetRegularAtlas>(_atlas_pair).getGlyphInfo(ch, info);
}

void DoubleAtlas::bindAtlases() const
{
	std::get<GetRegularAtlas>(_atlas_pair).bindAtlas();
	std::get<GetColoredAtlas>(_atlas_pair).bindAtlas();
}

void DoubleAtlas::unbindAtlases() const
{
	std::get<GetRegularAtlas>(_atlas_pair).unbindAtlas();
	std::get<GetColoredAtlas>(_atlas_pair).unbindAtlas();
}

template <size_t Atlas>
GLenum DoubleAtlas::getAtlasTexUnit() const
{
	return std::get<Atlas>(_atlas_pair).getAtlasTexUnit();
}

template <size_t Atlas>
GLenum DoubleAtlas::getAtlasTexBufUnit() const
{
	return std::get<Atlas>(_atlas_pair).getAtlasTexBufUnit();
}

template <size_t Atlas>
GLenum DoubleAtlas::getCharFormatBufUnit() const
{
	return std::get<Atlas>(_atlas_pair).getCharFormatBufUnit();
}

bool DoubleAtlas::isReady() const
{
	return std::get<GetRegularAtlas>(_atlas_pair).isReady() &&
		   std::get<GetColoredAtlas>(_atlas_pair).isReady();
}

template GLenum DoubleAtlas::getAtlasTexUnit<DoubleAtlas::GetRegularAtlas>() const;
template GLenum DoubleAtlas::getAtlasTexBufUnit<DoubleAtlas::GetRegularAtlas>() const;
template GLenum DoubleAtlas::getCharFormatBufUnit<DoubleAtlas::GetRegularAtlas>() const;
template GLenum DoubleAtlas::getAtlasTexUnit<DoubleAtlas::GetColoredAtlas>() const;
template GLenum DoubleAtlas::getAtlasTexBufUnit<DoubleAtlas::GetColoredAtlas>() const;
template GLenum DoubleAtlas::getCharFormatBufUnit<DoubleAtlas::GetColoredAtlas>() const;

} // namespace Thr
