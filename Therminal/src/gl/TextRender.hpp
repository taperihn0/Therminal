#pragma once

#include "Shader.hpp"
#include "Atlas.hpp"
#include "screen/Line.hpp"
#include "RenderFormat.hpp"
#include "screen/Grid.hpp"
#include "Atlas.hpp"

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

	void init(const RenderFormat& fmt, 
			  std::shared_ptr<FontAtlas>& atlas, 
			  std::shared_ptr<Font>& font);

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

	std::shared_ptr<GLuint>		   _vao_id_ptr;
	GLuint						   _base_vbo_id;
	GLuint						   _vbo_id;
	RenderFormat				   _fmt;
	uint 						   _cols;
	uint 						   _rows;
	std::unique_ptr<ShaderProgram> _shader;
	size_t						   _cell_count;
	bool						   _initialized;
	std::shared_ptr<FontAtlas> 	   _atlas;
	std::shared_ptr<Font>		   _font;
};

} // namespace Thr
