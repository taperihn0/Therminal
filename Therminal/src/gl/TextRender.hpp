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
};

class TextRender
{
public:
	TextRender();
	~TextRender();
	
	void init(TextRenderInfo spec);

	TextRender(const TextRender&) = delete;
	TextRender(TextRender&&) = delete;

	TextRender operator=(const TextRender&) = delete;
	TextRender operator=(TextRender&&) = delete;

	void submitCurrFrame(const Vec<Ptr<const Line>>& text);
	void renderText() const;
private:

	struct ShaderProgram
	{
		GLShaderStage vert = GLShaderStage(SHADER_STAGE_VERTEX);
		GLShaderStage frag = GLShaderStage(SHADER_STAGE_FRAGMENT);
		GLShader	  prog;
	};

	FontAtlas					   _atlas;
	// we share VAO that with atlas and other subsystems
	std::shared_ptr<GLuint>		   _vao_id_ptr;
	GLuint						   _base_vbo_id;
	GLuint						   _vbo_id;
	int							   _window_width;
	int							   _window_height;
	uint 						   _cell_width;
	uint 						   _cell_height;
	uint 						   _cols;
	uint 						   _rows;
	std::unique_ptr<ShaderProgram> _shader;
	size_t						   _cell_count;
	bool						   _initialized;
};

} // namespace Thr
