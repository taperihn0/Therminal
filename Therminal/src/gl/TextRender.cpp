#include "TextRender.hpp"

namespace Thr {

struct ShaderCellInfo {
	uint32_t id;
	Col8     fg;
	Col8     bg;
};

TextRender::TextRender(TextRenderInfo spec)
	: _atlas(1024, 1024)
	, _vao_id(0)
	, _vbo_id(0)
	, _window_width(spec.window_width)
	, _window_height(spec.window_height)
	, _cell_width(spec.cell_width)
	, _cell_height(spec.cell_height)
	, _cols(static_cast<int>(_window_width / _cell_width))
	, _rows(static_cast<int>(_window_height / _cell_height))
	, _shader(std::make_unique<ShaderProgram>())
{
	glGenVertexArrays(1, std::addressof(_vao_id));
	THR_HARD_ASSERT(_vao_id != 0 && glIsVertexArray(_vao_id) == GL_TRUE);

	glGenBuffers(1, std::addressof(_vbo_id));
	THR_HARD_ASSERT(_vbo_id != 0 && glIsBuffer(_vbo_id) == GL_TRUE);

	glBindVertexArray(_vao_id);
	glBindBuffer(GL_ARRAY_BUFFER, _vbo_id);

	glBufferData(GL_ARRAY_BUFFER, 
				 _cols * _rows * sizeof(ShaderCellInfo), 
				 nullptr, 
				 GL_DYNAMIC_DRAW);

	glEnableVertexAttribArray(0);
	glVertexAttribDivisor(0, 1);
	glVertexAttribIPointer(0, 
						   1, GL_UNSIGNED_INT, 
						   sizeof(ShaderCellInfo), 
						   reinterpret_cast<void*>(offsetof(ShaderCellInfo, id)));

	glEnableVertexAttribArray(1);
	glVertexAttribDivisor(1, 1);
	glVertexAttribPointer(1, 
						  4, GL_UNSIGNED_BYTE, GL_FALSE, 
						  sizeof(ShaderCellInfo), 
						  reinterpret_cast<void*>(offsetof(ShaderCellInfo, fg)));

	glEnableVertexAttribArray(2);
	glVertexAttribDivisor(2, 1);
	glVertexAttribPointer(2, 
						  4, GL_UNSIGNED_BYTE, GL_FALSE, 
						  sizeof(ShaderCellInfo), 
						  reinterpret_cast<void*>(offsetof(ShaderCellInfo, bg)));

	_atlas.init();

	glBindVertexArray(0);

	_shader->vert.compileStage(FilePath("assets/shaders/AtlasTextShader.vert"));
	_shader->frag.compileStage(FilePath("assets/shaders/AtlasTextShader.frag"));

	_shader->prog.attachStage(_shader->vert);
	_shader->prog.attachStage(_shader->frag);
	_shader->prog.linkProgram();
}

TextRender::~TextRender() 
{
	THR_HARD_ASSERT(_vao_id != 0 && glIsVertexArray(_vao_id) == GL_TRUE);
	glDeleteVertexArrays(1, &_vao_id);

	THR_HARD_ASSERT(_vbo_id != 0 && glIsBuffer(_vbo_id) == GL_TRUE);
	glDeleteBuffers(1, &_vbo_id);
}

void TextRender::submitLines(const Vec<Ptr<Line>>& text) {
	const size_t total_cells = _cols * _rows;

	Vec<ShaderCellInfo> buffer(total_cells); // TODO: preallocate that and reuse every time we got here

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
				id,
				cell.fg,
				cell.bg
			});
		}
	}

	THR_ASSERT(!buffer.empty());

	glBindVertexArray(_vao_id);

	glBindBuffer(GL_ARRAY_BUFFER, _vbo_id);
	glBufferSubData(GL_ARRAY_BUFFER, 0, 
					buffer.size() * sizeof(ShaderCellInfo), 
					reinterpret_cast<void*>(buffer.data()));

	glBindVertexArray(0);
}

void TextRender::renderText() const
{
	glBindVertexArray(_vao_id);
	_shader->prog.useProgram();
	_atlas.bindAtlas();

	//glDrawArraysInstanced(GL_TRIANGLE_STRIP, 0, 4, _cols * _rows);

	glBindVertexArray(0);
}

} // namespace Thr
