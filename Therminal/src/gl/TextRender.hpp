#pragma once

#include "Shader.hpp"
#include "Atlas.hpp"
#include "screen/Line.hpp"

namespace Thr
{

struct TextRenderInfo
{
	int window_width;
	int window_height;
	int cell_width;
	int cell_height;
};

class TextRender
{
public:
	TextRender() = delete;
	~TextRender();
	TextRender(TextRenderInfo spec);

	TextRender(const TextRender&) = delete;
	TextRender(TextRender&&) = delete;

	TextRender operator=(const TextRender&) = delete;
	TextRender operator=(TextRender&&) = delete;

	void submitLines(const Vec<Ptr<Line>>& text);
	void renderText() const;
private:

	struct ShaderProgram
	{
		GLShaderStage vert = GLShaderStage(SHADER_STAGE_VERTEX);
		GLShaderStage frag = GLShaderStage(SHADER_STAGE_FRAGMENT);
		GLShader	  prog;
	};

	FontAtlas					   _atlas;
	GLuint						   _vao_id;
	GLuint						   _vbo_id;
	int							   _window_width;
	int							   _window_height;
	int 						   _cell_width;
	int 						   _cell_height;
	int 						   _cols;
	int 						   _rows;
	std::unique_ptr<ShaderProgram> _shader;
};

} // namespace Thr
