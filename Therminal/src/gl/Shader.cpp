#include "Shader.hpp"
#include "filesys/ReadFile.hpp"

namespace Thr
{

GLShaderStage::GLShaderStage(ShaderStage stage)
	: _id(0)
	, _stage(stage)
{
	createStage();
}

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

		THR_LOG_ERROR("Failed to compile shader stage, err: {}", log);
		return;
	}
}

void GLShaderStage::compileStage(const FilePath& fp)
{
	const std::string src = readFile(fp);
	THR_HARD_ASSERT(!src.empty());
	compileStage(src);
}

THR_INLINE void GLShaderStage::createStage()
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
		gl_stage = GL_GEOMETRY_SHADER;
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
		THR_LOG_ERROR("Failed to link shader program, err: {}", log);

		return;
	}
}

void GLShader::useProgram() const 
{
	THR_HARD_ASSERT(_id != 0 && glIsProgram(_id) == GL_TRUE);
	glUseProgram(_id);
}

void GLShader::attachStage(const GLShaderStage& stage) const
{
	const GLuint stage_id = stage.getID();

	THR_HARD_ASSERT(stage_id != 0 && glIsShader(stage_id) == GL_TRUE);
	THR_HARD_ASSERT(_id != 0 && glIsProgram(_id) == GL_TRUE);

	glAttachShader(_id, stage_id);
}

template <typename T>
void GLShader::set1(std::string_view name, const T& val) const
{
	if constexpr (is_same_v<T, int>) {
		glUniform1i(glGetUniformLocation(_id, name.data()), val);
	}
	else if constexpr (is_same_v<T, uint>) {
		glUniform1ui(glGetUniformLocation(_id, name.data()), val);
	}
	else if constexpr (is_same_v<T, float>) {
		glUniform1f(glGetUniformLocation(_id, name.data()), val);
	}
	else if constexpr (is_same_v<T, double>) {
		glUniform1d(glGetUniformLocation(_id, name.data()), val);
	}
	else {
		THR_STATIC_ASSERT_LOG(false, "Unsupported uniform type");
	}
}

} // namespace Thr
