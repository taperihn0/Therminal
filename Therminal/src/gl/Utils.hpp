#pragma once

#include "Common.hpp"

namespace Thr
{

THR_NODISCARD THR_FORCEINLINE std::string_view getGlErrorStr(GLenum err)
{
	switch (err) {
	case GL_NO_ERROR:
		return "GL_NO_ERROR";
	case GL_INVALID_ENUM:
		return "GL_INVALID_ENUM";
	case GL_INVALID_VALUE:
		return "GL_INVALID_VALUE";
	case GL_INVALID_OPERATION:
		return "GL_INVALID_OPERATION";
	case GL_INVALID_FRAMEBUFFER_OPERATION:
		return "GL_INVALID_FRAMEBUFFER_OPERATION";
	case GL_OUT_OF_MEMORY:
		return "GL_OUT_OF_MEMORY";
	case GL_STACK_UNDERFLOW:
		return "GL_STACK_UNDERFLOW";
	case GL_STACK_OVERFLOW:
		return "GL_STACK_OVERFLOW";
	default: break;
	}

	return "Invalid enum value";
}

THR_INLINE GLenum pollGlErrors(std::function<void(GLenum)> policy)
{

#if defined(THR_DEBUG) || defined(THR_RELEASE_DEBUG_INFO)
	GLenum err = GL_NO_ERROR;

	while ((err = glGetError()) != GL_NO_ERROR) {
		policy(err);
	}

	return err;
#else
	return GL_NO_ERROR;
#endif
}

THR_FORCEINLINE GLint getGlActiveTexUniformVal(GLenum tex) 
{
	THR_ASSERT(tex >= GL_TEXTURE0 && tex <= GL_TEXTURE31);
	return tex - GL_TEXTURE0;
}

} // namespace Thr
