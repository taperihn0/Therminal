#include "TextRender.hpp"
#include "logger/Log.hpp"
#include "Utils.hpp"

namespace Thr {

struct ShaderCellInfo 
{
	glm::u32vec2 pos;
	uint32_t	 id;
	Color3u8     fg;
	Color3u8     bg;
};

TextRender::TextRender()
	: _vao_id_ptr(nullptr)
	, _base_vbo_id(0)
	, _vbo_id(0)
	, _cols(0)
	, _rows(0)
	, _shader(std::make_unique<ShaderProgram>())
	, _cell_count(0)
	, _initialized(false)
{}

void TextRender::init(const RenderFormat& fmt, 
					  std::shared_ptr<DoubleAtlas>& datlas,
					  std::shared_ptr<Font>& regular_font,
					  std::shared_ptr<Font>& colored_font)
{
	if (_initialized) {
		THR_LOG_ERROR("TextRender subsystem is already initialized, can't initialize again");
		return;
	}

	_fmt = fmt;

	_vao_id_ptr = std::make_shared<GLuint>(0);

	glGenVertexArrays(1, _vao_id_ptr.get());

	{
		glm::ivec2 winsize = _fmt.getWindowSize();
		THR_ASSERT(winsize.x > 0 && winsize.y > 0);
	}

	_cols = _fmt.getCellCount().x;
	_rows = _fmt.getCellCount().y;

	glBindVertexArray(*_vao_id_ptr);
	THR_HARD_ASSERT(*_vao_id_ptr != 0 && glIsVertexArray(*_vao_id_ptr) == GL_TRUE);
		
	_datlas = datlas;
	_datlas->init(regular_font, colored_font);
		
	glGenBuffers(1, std::addressof(_base_vbo_id));
	glBindBuffer(GL_ARRAY_BUFFER, _base_vbo_id);
	THR_HARD_ASSERT(_base_vbo_id != 0 && glIsBuffer(_base_vbo_id) == GL_TRUE);

	{
		static std::array<glm::vec2, 4> vert = {
			glm::vec2{ 0.f, 0.f },
			glm::vec2{ 1.f, 0.f },
			glm::vec2{ 0.f, 1.f },
			glm::vec2{ 1.f, 1.f }
		};

		glBufferData(GL_ARRAY_BUFFER, 
						sizeof(vert),
						reinterpret_cast<GLvoid*>(vert.data()),
						GL_STATIC_DRAW);
	}

	glEnableVertexAttribArray(0);

	glVertexAttribPointer(0, 
						  2, GL_FLOAT, GL_FALSE, 
						  2 * sizeof(float), 
						  nullptr);

	glGenBuffers(1, std::addressof(_vbo_id));
	glBindBuffer(GL_ARRAY_BUFFER, _vbo_id);
	THR_HARD_ASSERT(_vbo_id != 0 && glIsBuffer(_vbo_id) == GL_TRUE);

	glBufferData(GL_ARRAY_BUFFER, 
				 _cols * _rows * sizeof(ShaderCellInfo), 
				 nullptr, 
				 GL_DYNAMIC_DRAW);

	glEnableVertexAttribArray(1);
	glVertexAttribDivisor(1, 1);
	glVertexAttribIPointer(1,
						   2, GL_UNSIGNED_INT,
						   sizeof(ShaderCellInfo),
						   reinterpret_cast<GLvoid*>(offsetof(ShaderCellInfo, pos)));

	glEnableVertexAttribArray(2);
	glVertexAttribDivisor(2, 1);
	glVertexAttribIPointer(2, 
						   1, GL_UNSIGNED_INT, 
						   sizeof(ShaderCellInfo), 
						   reinterpret_cast<GLvoid*>(offsetof(ShaderCellInfo, id)));

	glEnableVertexAttribArray(3);
	glVertexAttribDivisor(3, 1);
	glVertexAttribIPointer(3, 
						   3, GL_UNSIGNED_BYTE,
						   sizeof(ShaderCellInfo), 
						   reinterpret_cast<GLvoid*>(offsetof(ShaderCellInfo, fg)));

	glEnableVertexAttribArray(4);
	glVertexAttribDivisor(4, 1);
	glVertexAttribIPointer(4, 
						   3, GL_UNSIGNED_BYTE,
						   sizeof(ShaderCellInfo), 
						   reinterpret_cast<GLvoid*>(offsetof(ShaderCellInfo, bg)));

	glBindVertexArray(0);

	_shader->vert.init();
	_shader->frag.init();
	_shader->prog.init();

	_shader->vert.compileStage(FilePath("Therminal/assets/shaders/TextShader.vert"));
	THR_HARD_ASSERT(_shader->vert.isCompiled());

	_shader->frag.compileStage(FilePath("Therminal/assets/shaders/TextShader.frag"));
	THR_HARD_ASSERT(_shader->frag.isCompiled());

	_shader->prog.attachStage(_shader->vert);
	_shader->prog.attachStage(_shader->frag);
	_shader->prog.linkProgram();
	THR_HARD_ASSERT(_shader->prog.isLinked());

	/* Setup uniforms for textures and tex-buffers */
	{
		if (!_datlas->isReady()) {
			THR_LOG_ERROR("Atlas is not initialized");
			return;
		}

		_shader->prog.useProgram();

		const glm::ivec2 window_size = _fmt.getWindowSize();
		const glm::ivec2 cell_size = _fmt.getCellSize();

		_shader->prog.setUniform2<GLuint>("ScreenResPix", window_size.x, window_size.y);
		_shader->prog.setUniform2<GLuint>("CellSizePix",  cell_size.x, cell_size.y);

		const GLint uvs_buf_unit = getGlActiveTexIndex(
			_datlas->getAtlasTexBufUnit<DoubleAtlas::GetRegularAtlas>());
		_shader->prog.setUniform1<GLint>("AtlasUVsLookup", uvs_buf_unit);

		const GLint format_buf_unit = getGlActiveTexIndex(
			_datlas->getCharFormatBufUnit<DoubleAtlas::GetRegularAtlas>());
		_shader->prog.setUniform1<GLint>("CharFormatLookup", format_buf_unit);

		const GLint atlas_tex_unit = getGlActiveTexIndex(
			_datlas->getAtlasTexUnit<DoubleAtlas::GetRegularAtlas>());
		_shader->prog.setUniform1<GLint>("AtlasTexture", atlas_tex_unit);

		_shader->prog.unuseProgram();
	}

	const GLenum err = pollGlErrors([](GLenum err) {
		THR_LOG_ERROR("OpenGL error during TextRender initialization: {}", getGlErrorStr(err));
	});

	if (err == GL_NO_ERROR) {
		THR_LOG_INFO("TextRender subsystem initialization completed");
	} 
	else {
		THR_LOG_ERROR("TextRender subsystem initialization completed with errors");
	}

	_initialized = true;
}

void TextRender::getRenderFormat(RenderFormat& fmt)
{
	if (!_initialized) {
		THR_LOG_ERROR("Querying render format of unitialized TextRender");
		return;
	}

	fmt = _fmt;
}

TextRender::~TextRender() 
{
	if (!_initialized)
		return;

	if (_vao_id_ptr != nullptr && glIsVertexArray(*_vao_id_ptr) == GL_TRUE) {
		glDeleteVertexArrays(1, _vao_id_ptr.get());
	}

	if (_vbo_id != 0) {
		glDeleteBuffers(1, std::addressof(_vbo_id));
	}

	if (_base_vbo_id != 0) {
		glDeleteBuffers(1, std::addressof(_base_vbo_id));
	}
}

void TextRender::submitCurrFrame(const RenderFramePacket& packet) 
{
	if (!_initialized) {
		THR_LOG_ERROR("TextRender subsystem is not initialized, can't submit frame");
		return;
	}

	const size_t total_cells = _cols * _rows;

	Vec<ShaderCellInfo> buffer;
	buffer.reserve(total_cells); // TODO: preallocate that and reuse every time we got here

	const glm::ivec2 cell_size = _fmt.getCellSize();
	const glm::ivec2 total_shift = cell_size;

	uint xpos = 0;
	uint ypos = 0;

	for (const auto& ln : *packet.ln_ptrs) {
		for (auto it = ln->cbegin(); it != ln->getRightmostWriteIterator(); it++) {
			if (!it->cluster.isEmpty()) {
				for (char32_t codepoint : it->cluster.codepoints) {
					GlyphInfo info;
					uint32_t id = _datlas->getGlyphInfo(codepoint, info);

					if (id == static_cast<uint32_t>(-1)) {
						glBindVertexArray(*_vao_id_ptr);
						_datlas->addGlyph(codepoint);
						glBindVertexArray(0);
						
						id = _datlas->getGlyphInfo(codepoint, info);
					}

					THR_ASSERT(info.id == id);

					buffer.push_back(ShaderCellInfo{
						glm::u32vec2{ xpos, ypos },
						id,
						it->fg,
						it->bg
					});
				}
			}

			xpos += total_shift.x;
		}

		ypos += total_shift.y;
		xpos = 0;
	}

	THR_ASSERT(!buffer.empty());

	glBindVertexArray(*_vao_id_ptr);

	glBindBuffer(GL_ARRAY_BUFFER, _vbo_id);
	glBufferSubData(GL_ARRAY_BUFFER, 0, 
					buffer.size() * sizeof(ShaderCellInfo), 
					reinterpret_cast<GLvoid*>(buffer.data()));

	_cell_count = buffer.size();

	const GLenum err = pollGlErrors([](GLenum err) {
		THR_LOG_ERROR("OpenGL error during TextRender frame submission: {}", getGlErrorStr(err));
	});

	if (err != GL_NO_ERROR) {
		THR_LOG_ERROR("Failed to submit frame of text to TextRender subsystem");
	}
}

void TextRender::renderText() const
{
	if (!_initialized) {
		THR_LOG_ERROR("TextRender subsystem is not initialized, can't render frame of text");
		return;
	}

	glBindVertexArray(*_vao_id_ptr);
	_datlas->bindAtlases();
	_shader->prog.useProgram();

	glDrawArraysInstanced(GL_TRIANGLE_STRIP, 0, 4, static_cast<GLsizei>(_cell_count));

	_shader->prog.unuseProgram();
	_datlas->unbindAtlases();
	glBindVertexArray(0);

	pollGlErrors([](GLenum err) {
		THR_LOG_ERROR("OpenGL error during TextRender frame rendering: {}", getGlErrorStr(err));
	});
}

void TextRender::clearScreen(Color4f col)
{
	glClearColor(col.r, col.g, col.b, col.a);
	glClear(GL_COLOR_BUFFER_BIT);
}

} // namespace Thr
