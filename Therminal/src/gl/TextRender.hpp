#pragma once

#include "Shader.hpp"
#include "Atlas.hpp"
#include "screen/Line.hpp"
#include "RenderFormat.hpp"
#include "screen/Grid.hpp"

namespace Thr
{

/* Rendering frame data
*/
struct RenderFramePacket
{
	std::shared_ptr<const LinePtrBuf> ln_ptrs;
};

class TextRender
{
public:
	TextRender();
	~TextRender();

	TextRender(const TextRender&) = delete;
	TextRender(TextRender&&) = delete;

	void init(const RenderFormat& fmt);
	void getRenderFormat(RenderFormat& fmt);
		
	TextRender operator=(const TextRender&) = delete;
	TextRender operator=(TextRender&&) = delete;

	void submitCurrFrame(const RenderFramePacket& packet);
	void renderText() const;

	void clearScreen(Color4f col);
private:

	struct ShaderProgram
	{
		GLShaderStage vert = GLShaderStage(SHADER_STAGE_VERTEX);
		GLShaderStage frag = GLShaderStage(SHADER_STAGE_FRAGMENT);
		GLShader	  prog;
	};

	static constexpr int DefaultAtlasWidth  = 256;
	static constexpr int DefaultAtlasHeight = 256;

	FontAtlas					   _atlas;
	// we share VAO that with atlas and other subsystems
	std::shared_ptr<GLuint>		   _vao_id_ptr;
	GLuint						   _base_vbo_id;
	GLuint						   _vbo_id;
	RenderFormat				   _fmt;
	uint 						   _cols;
	uint 						   _rows;
	std::unique_ptr<ShaderProgram> _shader;
	size_t						   _cell_count;
	bool						   _initialized;
};

} // namespace Thr
