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

	void init();

	GLShaderStage  operator=(const GLShaderStage&) = delete;
	GLShaderStage& operator=(GLShaderStage&& shader);

	void compileStage(std::string_view src);
	void compileStage(const FilePath& fp);

	THR_NODISCARD THR_INLINE GLuint getID() const;
	THR_NODISCARD THR_INLINE bool   isCompiled() const;
private:
	GLuint		_id;
	ShaderStage _stage;
	bool 		_compiled;
};

class GLShader
{
public:
	GLShader() = default;
	~GLShader();
	GLShader(const GLShader&) = delete;
	GLShader(GLShader&& shader);

	void init();

	GLShader operator=(const GLShader&) = delete;
	GLShader operator=(GLShader&&) = delete;

	void linkProgram();
	void useProgram() const;
	void unuseProgram() const;
	void attachStage(const GLShaderStage& stage) const;

	template <typename T>
	void setUniform1(std::string_view name, 
					 const T val) const;

	template <typename T>
	void setUniform1(std::string_view name,
					 const T* ptr) const;

	template <typename T>
	void setUniform2(std::string_view name, 
					 const T val0, const T val1) const;

	template <typename T>
	void setUniform2(std::string_view name,
					 const T* ptr) const;

	template <typename T>
	void setUniform3(std::string_view name, 
					 const T val0, const T val1, const T val2) const;

	template <typename T>
	void setUniform3(std::string_view name,
					 const T* ptr) const;

	template <typename T>
	void setUniform4(std::string_view name, 
					 const T val0, const T val1, const T val2, const T val3) const;

	template <typename T>
	void setUniform4(std::string_view name,
					 const T* ptr) const;

	THR_NODISCARD THR_INLINE GLuint getID() const;
	THR_NODISCARD THR_INLINE GLuint isLinked() const;
private:
	GLuint _id;
	bool   _linked;
};

THR_INLINE GLuint GLShaderStage::getID() const { return _id; }
THR_INLINE bool   GLShaderStage::isCompiled() const { return _compiled; }
THR_INLINE GLuint GLShader::getID()		 const { return _id; }
THR_INLINE GLuint GLShader::isLinked() const   { return _linked; }

} // namespace Thr
