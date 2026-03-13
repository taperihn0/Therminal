#include "TextRender.hpp"
#include "logger/Log.hpp"
#include "Utils.hpp"

namespace Thr {

struct ShaderCellInfo 
{
	glm::u32vec2 pos;
	uint32_t	 id;
	Col8		 fg;
	Col8		 bg;
};

TextRender::TextRender()
	: _atlas(128, 128, 48)
	, _vao_id_ptr(nullptr)
	, _base_vbo_id(0)
	, _vbo_id(0)
	, _window_width(0)
	, _window_height(0)
	, _cell_width(0)
	, _cell_height(0)
	, _cols(0)
	, _rows(0)
	, _shader(std::make_unique<ShaderProgram>())
	, _cell_count(0)
	, _initialized(false)
{}

void TextRender::init(TextRenderInfo spec)
{
	if (_initialized) {
		THR_LOG_ERROR("TextRender subsystem is already initialized, can't initialize again");
		return;
	}

	_window_width = spec.window_width;
	_window_height = spec.window_height;

	_vao_id_ptr = std::make_shared<GLuint>(0);

	glGenVertexArrays(1, _vao_id_ptr.get());

	_atlas.init(_vao_id_ptr);

	_atlas.getGlyphPixSize(_cell_width, _cell_height);

	_cols = _window_width / _cell_width;
	_rows = _window_height / _cell_height;

	glBindVertexArray(*_vao_id_ptr);
	THR_HARD_ASSERT(*_vao_id_ptr != 0 && glIsVertexArray(*_vao_id_ptr) == GL_TRUE);

	glGenBuffers(1, std::addressof(_base_vbo_id));
	glBindBuffer(GL_ARRAY_BUFFER, _base_vbo_id);
	THR_HARD_ASSERT(_base_vbo_id != 0 && glIsBuffer(_base_vbo_id) == GL_TRUE);

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
						   4, GL_UNSIGNED_BYTE,
						   sizeof(ShaderCellInfo), 
						   reinterpret_cast<GLvoid*>(offsetof(ShaderCellInfo, fg)));

	glEnableVertexAttribArray(4);
	glVertexAttribDivisor(4, 1);
	glVertexAttribIPointer(4, 
						   4, GL_UNSIGNED_BYTE,
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

void TextRender::submitCurrFrame(const Vec<Ptr<const Line>>& text) 
{
	if (!_initialized) {
		THR_LOG_ERROR("TextRender subsystem is not initialized, can't submit frame");
		return;
	}

	const size_t total_cells = _cols * _rows;

	Vec<ShaderCellInfo> buffer;
	buffer.reserve(total_cells); // TODO: preallocate that and reuse every time we got here

	uint xpos = 0;
	uint ypos = 0;

	for (const auto& ln : text) {
		THR_ASSERT(ln != nullptr);

		const Vec<Cell>& cells = ln->getCellLine();

		for (const auto& cell : cells) {
			GlyphInfo info;
			uint32_t id = _atlas.getGlyphInfo(cell.ch, info);

			THR_ASSERT(info.id == id);

			if (id == static_cast<uint32_t>(-1)) {
				_atlas.addGlyph(cell.ch);
				id = _atlas.getGlyphInfo(cell.ch, info);
			}
			
			buffer.push_back(ShaderCellInfo{
				glm::u32vec2{ xpos, ypos },
				id,
				cell.fg,
				cell.bg
			});

			THR_ASSERT(info.advance == static_cast<int>(_cell_width));
			xpos += _cell_width;
		}

		ypos += _cell_height;
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

	_atlas.bindAtlas();

	glBindVertexArray(*_vao_id_ptr);
	_shader->prog.useProgram();

	_shader->prog.setUniform2<GLuint>("ScreenResPix", _window_width, _window_height);
	_shader->prog.setUniform2<GLuint>("CellSizePix",  _cell_width, _cell_height);
	_shader->prog.setUniform1<GLint>("AtlasUVsLookup", 1);//_atlas.getActiveTextureBufferUnit());
	_shader->prog.setUniform1<GLint>("AtlasTexture", 0);//_atlas.getActiveTextureUnit());

	glDrawArraysInstanced(GL_TRIANGLE_STRIP, 0, 4, static_cast<GLsizei>(_cell_count));

	glBindVertexArray(0);

	pollGlErrors([](GLenum err) {
		THR_LOG_ERROR("OpenGL error during TextRender frame rendering: {}", getGlErrorStr(err));
	});
}

} // namespace Thr
