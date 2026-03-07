#pragma once

#include "Common.hpp"
#include "filesys/Filepath.hpp"

namespace Thr
{

/* Platform agnostic shader stages.
*/
enum ShaderStage : uint8_t
{
	SHADER_STAGE_UNDEF			 = 0,
	SHADER_STAGE_VERTEX		     = 1,
	SHADER_STAGE_TESS_CONTROL	 = 2,
	SHADER_STAGE_TESS_EVALUATION = 3,
	SHADER_STAGE_GEOMETRY		 = 4,
	SHADER_STAGE_FRAGMENT		 = 5,
	SHADER_STAGE_MAX			 = 5
};

class GLShaderStage
{
public:
	GLShaderStage() = delete;
	explicit GLShaderStage(ShaderStage stage);
	~GLShaderStage();
	GLShaderStage(const GLShaderStage&) = delete;
	GLShaderStage(GLShaderStage&& shader);

	GLShaderStage  operator=(const GLShaderStage&) = delete;
	GLShaderStage& operator=(GLShaderStage&& shader);

	void compileStage(std::string_view src);
	void compileStage(const FilePath& fp);

	THR_INLINE GLuint getID() const;
private:
	void createStage();

	GLuint		_id;
	ShaderStage _stage;
};

class GLShader
{
public:
	GLShader() = default;
	~GLShader();
	GLShader(const GLShader&) = delete;
	GLShader(GLShader&& shader);

	GLShader operator=(const GLShader&) = delete;
	GLShader operator=(GLShader&&) = delete;

	void linkProgram();
	void useProgram() const;
	void attachStage(const GLShaderStage& stage) const;

	template <typename T>
	void set1(std::string_view name, const T& val) const;

	THR_INLINE GLuint getID() const;
private:
	GLuint _id;
};

THR_INLINE GLuint GLShaderStage::getID() const { return _id; }
THR_INLINE GLuint GLShader::getID()		 const { return _id; }

} // namespace Thr
