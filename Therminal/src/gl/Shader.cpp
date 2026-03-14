#include "Shader.hpp"
#include "filesys/ReadFile.hpp"
#include "Utils.hpp"

namespace Thr
{

GLShaderStage::GLShaderStage(ShaderStage stage)
	: _id(0)
	, _stage(stage)
	, _compiled(false)
{}

GLShaderStage::~GLShaderStage()
{
	if (_id != 0 && glIsShader(_id) == GL_TRUE) {
		THR_HARD_ASSERT_LOG(_stage != SHADER_STAGE_UNDEF, "Undefined shader stage");
		glDeleteShader(_id);
	}
}

GLShaderStage::GLShaderStage(GLShaderStage&& shader)
{
	_id = shader._id;
	_stage = shader._stage;
	shader._id = 0;
	shader._stage = SHADER_STAGE_UNDEF;
}

GLShaderStage& GLShaderStage::operator=(GLShaderStage&& shader)
{
	THR_HARD_ASSERT_LOG(shader._stage == _stage,
		"Trying to assign different shader stage type");

	if (_id != 0) {
		THR_HARD_ASSERT_LOG(_stage != SHADER_STAGE_UNDEF, "Undefined shader stage");
		glDeleteShader(_id);
	}

	_id = shader._id;
	shader._id = 0;
	shader._stage = SHADER_STAGE_UNDEF;

	return *this;
}

void GLShaderStage::compileStage(std::string_view src) 
{
	if (!_id)
		init();

	THR_HARD_ASSERT(glIsShader(_id));

	const GLchar* c_str = src.data();
	glShaderSource(_id, 1, std::addressof(c_str), nullptr);

	glCompileShader(_id);

	GLint status = 0;
	glGetShaderiv(_id, GL_COMPILE_STATUS, std::addressof(status));

	if (!status) {
		GLint len = 0;
		glGetShaderiv(_id, GL_INFO_LOG_LENGTH, std::addressof(len));

		std::string log(len, '\0');
		glGetShaderInfoLog(_id, len, nullptr, log.data());

		THR_LOG_FATAL("Failed to compile shader stage");
		THR_LOG_FATAL("Output:");
		THR_LOG_FATAL("{}", log);

		return;
	}

	const GLenum err = pollGlErrors([&](GLenum err) {
		THR_LOG_ERROR("Failed to compile shader stage program of id: {}", _id);
		THR_LOG_ERROR("Error: {}", getGlErrorStr(err));
	});

	if (err != GL_NO_ERROR) {
		THR_LOG_ERROR("Failed to compile shader program of id: {}", _id);
	} 
	else _compiled = true;
}

void GLShaderStage::compileStage(const FilePath& fp)
{
	const std::string src = readFile(fp);
	THR_HARD_ASSERT(!src.empty());
	compileStage(src);
}

THR_INLINE void GLShaderStage::init()
{
	GLenum gl_stage = 0;

	switch (_stage) {
	case SHADER_STAGE_VERTEX:
		gl_stage = GL_VERTEX_SHADER;
		break;
	case SHADER_STAGE_TESS_CONTROL:
		gl_stage = GL_TESS_CONTROL_SHADER;
		break;
	case SHADER_STAGE_TESS_EVALUATION:
		gl_stage = GL_TESS_EVALUATION_SHADER;
		break;
	case SHADER_STAGE_GEOMETRY:
		gl_stage = GL_GEOMETRY_SHADER;
		break;
	case SHADER_STAGE_FRAGMENT:
		gl_stage = GL_FRAGMENT_SHADER;
		break;
	default:
		THR_HARD_ASSERT_LOG(false, "Undefined shader stage");
		break;
	}

	if ((_id = glCreateShader(gl_stage)) == 0) {
		THR_HARD_ASSERT_LOG(false, "Failed to create shader stage");
		return;
	}

	THR_HARD_ASSERT_LOG(glIsShader(_id) == GL_TRUE, "Failed to create shader stage");
}

GLShader::~GLShader()
{
	if (_id != 0 && glIsProgram(_id) == GL_TRUE) {
		glDeleteProgram(_id);
	}
}

GLShader::GLShader(GLShader&& shader)
{
	_id = shader._id;
	shader._id = 0;
}

void GLShader::init()
{
	_id = glCreateProgram();
	THR_HARD_ASSERT(_id != 0 && glIsProgram(_id) == GL_TRUE);
}

void GLShader::linkProgram() 
{
	THR_HARD_ASSERT(_id != 0 && glIsProgram(_id) == GL_TRUE);
	glLinkProgram(_id);

	GLint status = 0;
	glGetProgramiv(_id, GL_LINK_STATUS, std::addressof(status));

	if (!status) {
		GLint len = 0;
		glGetProgramiv(_id, GL_INFO_LOG_LENGTH, std::addressof(len));

		std::string log(len, '\0');
		glGetProgramInfoLog(_id, len, nullptr, log.data());
		
		THR_LOG_FATAL("Failed to link shader program");
		THR_LOG_FATAL("Output:");
		THR_LOG_FATAL("{}", log);

		return;
	}

	const GLenum err = pollGlErrors([&](GLenum err) {
		THR_LOG_ERROR("Failed to attach shader stage at shader {}", _id);
		THR_LOG_ERROR("Error: {}", getGlErrorStr(err));
	});

	if (err != GL_NO_ERROR) {
		THR_LOG_ERROR("Failed to link shader program of id: {}", _id);
	} 
	else _linked = true;
}

void GLShader::useProgram() const 
{
	THR_HARD_ASSERT(_id != 0 && glIsProgram(_id) == GL_TRUE);
	glUseProgram(_id);

	pollGlErrors([&](GLenum err) {
		THR_LOG_ERROR("Failed to use shader program of id: {}", _id);
		THR_LOG_ERROR("Error: {}", getGlErrorStr(err));
	});
}

void GLShader::unuseProgram() const
{
	glUseProgram(0);

	pollGlErrors([&](GLenum err) {
		THR_LOG_ERROR("Failed to unuse shader program of id: {}", _id);
		THR_LOG_ERROR("Error: {}", getGlErrorStr(err));
	});
}

void GLShader::attachStage(const GLShaderStage& stage) const
{
	const GLuint stage_id = stage.getID();

	THR_HARD_ASSERT(stage_id != 0 && glIsShader(stage_id) == GL_TRUE);
	THR_HARD_ASSERT(_id != 0 && glIsProgram(_id) == GL_TRUE);

	glAttachShader(_id, stage_id);

	const GLenum err = pollGlErrors([&](GLenum err) {
		THR_LOG_ERROR("Failed to attach shader stage at shader of id: {}", _id);
		THR_LOG_ERROR("Error: {}", getGlErrorStr(err));	
	});

	if (err != GL_NO_ERROR) {
		THR_LOG_ERROR("Failed to attach shader stage at shader of id: {}", _id);
	}
}

template <typename T>
void GLShader::setUniform1(std::string_view name, 
						   const T val) const
{
	THR_STATIC_ASSERT((is_same_v<T, GLint> ||
					   is_same_v<T, GLuint> ||
					   is_same_v<T, GLfloat>));

	const GLint l = glGetUniformLocation(_id, name.data());

	if (l >= 0) {
		if constexpr (is_same_v<T, GLint>) {
			glUniform1i(l, val);
		}
		else if constexpr (is_same_v<T, GLuint>) {
			glUniform1ui(l, val);
		}
		else if constexpr (is_same_v<T, GLfloat>) {
			glUniform1f(l, val);
		}
	}

	pollGlErrors([&](GLenum err) {
		THR_LOG_ERROR("Failed to set uniform: {}", name);
		THR_LOG_ERROR("Error: {}", getGlErrorStr(err));
	});
}

template <typename T>
void GLShader::setUniform1(std::string_view name,
						   const T* ptr) const
{
	THR_STATIC_ASSERT((is_same_v<T, GLint> ||
					   is_same_v<T, GLuint> ||
					   is_same_v<T, GLfloat>));

	THR_HARD_ASSERT(ptr != nullptr);

	setUniform1(name, ptr[0]);
}

template <typename T>
void GLShader::setUniform2(std::string_view name, 
						   const T val0, const T val1) const
{
	THR_STATIC_ASSERT((is_same_v<T, GLint> ||
					   is_same_v<T, GLuint> ||
					   is_same_v<T, GLfloat>));

	const GLint l = glGetUniformLocation(_id, name.data());

	if (l >= 0) {
		if constexpr (is_same_v<T, int>) {
			glUniform2i(l, val0, val1);
		}
		else if constexpr (is_same_v<T, uint>) {
			glUniform2ui(l, val0, val1);
		}
		else if constexpr (is_same_v<T, float>) {
			glUniform2f(l, val0, val1);
		}
	}

	pollGlErrors([&](GLenum err) {
		THR_LOG_ERROR("Failed to set uniform: {}", name);
		THR_LOG_ERROR("Error: {}", getGlErrorStr(err));
	});
}

template <typename T>
void GLShader::setUniform2(std::string_view name,
						   const T* ptr) const
{
	THR_STATIC_ASSERT((is_same_v<T, GLint> ||
					   is_same_v<T, GLuint> ||
					   is_same_v<T, GLfloat>));

	THR_HARD_ASSERT(ptr != nullptr);

	setUniform1(name, ptr[0], ptr[1]);
}

template <typename T>
void GLShader::setUniform3(std::string_view name,
						   const T val0, const T val1, const T val2) const
{
	THR_STATIC_ASSERT((is_same_v<T, GLint> ||
					   is_same_v<T, GLuint> ||
					   is_same_v<T, GLfloat>));

	const GLint l = glGetUniformLocation(_id, name.data());

	if (l >= 0) {
		if constexpr (is_same_v<T, int>) {
			glUniform3i(l, val0, val1, val2);
		}
		else if constexpr (is_same_v<T, uint>) {
			glUniform3ui(l, val0, val1, val2);
		}
		else if constexpr (is_same_v<T, float>) {
			glUniform3f(l, val0, val1, val2);
		}
	}

	pollGlErrors([&](GLenum err) {
		THR_LOG_ERROR("Failed to set uniform: {}", name);
		THR_LOG_ERROR("Error: {}", getGlErrorStr(err));
	});
}

template <typename T>
void GLShader::setUniform3(std::string_view name,
						   const T* ptr) const
{
	THR_STATIC_ASSERT((is_same_v<T, GLint> ||
					   is_same_v<T, GLuint> ||
					   is_same_v<T, GLfloat>));

	THR_HARD_ASSERT(ptr != nullptr);

	setUniform1(name, ptr[0], ptr[1], ptr[2]);
}

template <typename T>
void GLShader::setUniform4(std::string_view name,
						   const T val0, const T val1, const T val2, const T val3) const
{
	THR_STATIC_ASSERT((is_same_v<T, GLint> ||
					   is_same_v<T, GLuint> ||
					   is_same_v<T, GLfloat>));

	const GLint l = glGetUniformLocation(_id, name.data());

	if (l >= 0) {
		if constexpr (is_same_v<T, int>) {
			glUniform4i(l, val0, val1, val2, val3);
		}
		else if constexpr (is_same_v<T, uint>) {
			glUniform4ui(l, val0, val1, val2, val3);
		}
		else if constexpr (is_same_v<T, float>) {
			glUniform4f(l, val0, val1, val2, val3);
		}
	}

	pollGlErrors([&](GLenum err) {
		THR_LOG_ERROR("Failed to set uniform: {}", name);
		THR_LOG_ERROR("Error: {}", getGlErrorStr(err));
	});
}

template <typename T>
void GLShader::setUniform4(std::string_view name,
						   const T* ptr) const
{
	THR_STATIC_ASSERT((is_same_v<T, GLint> ||
					   is_same_v<T, GLuint> ||
					   is_same_v<T, GLfloat>));

	THR_HARD_ASSERT(ptr != nullptr);

	setUniform1(name,
			    ptr[0], ptr[1],
			    ptr[2], ptr[3]);
}

template void GLShader::setUniform1<GLint>(std::string_view, const GLint) const;
template void GLShader::setUniform1<GLuint>(std::string_view, const GLuint) const;
template void GLShader::setUniform1<GLfloat>(std::string_view, const GLfloat) const;
template void GLShader::setUniform2<GLint>(std::string_view, const GLint, const GLint) const;
template void GLShader::setUniform2<GLuint>(std::string_view, const GLuint, const GLuint) const;
template void GLShader::setUniform2<GLfloat>(std::string_view, const GLfloat, const GLfloat) const;
template void GLShader::setUniform3<GLint>(std::string_view, const GLint, const GLint, const GLint) const;
template void GLShader::setUniform3<GLuint>(std::string_view, const GLuint, const GLuint, const GLuint) const;
template void GLShader::setUniform3<GLfloat>(std::string_view, const GLfloat, const GLfloat, const GLfloat) const;
template void GLShader::setUniform4<GLint>(std::string_view, const GLint, const GLint, const GLint, const GLint) const;
template void GLShader::setUniform4<GLuint>(std::string_view, const GLuint, const GLuint, const GLuint, const GLuint) const;
template void GLShader::setUniform4<GLfloat>(std::string_view, const GLfloat, const GLfloat, const GLfloat, const GLfloat) const;

} // namespace Thr
