#pragma once

#include "Common.hpp"
#include <bitset>

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

THR_INLINE int getGlActiveTexIndex(GLenum tex);

struct TexturesUnitsContext
{
	TexturesUnitsContext() = default;
	TexturesUnitsContext(const TexturesUnitsContext&) = default;
	TexturesUnitsContext(TexturesUnitsContext&& tuc)
		: active_tex_unit(std::move(tuc.active_tex_unit))
	{
		tuc.active_tex_unit.reset();
	}
	
	TexturesUnitsContext& operator=(const TexturesUnitsContext&) = default;
	THR_FORCEINLINE TexturesUnitsContext& operator=(TexturesUnitsContext&& tuc)
	{
		active_tex_unit = std::move(tuc.active_tex_unit);
		tuc.active_tex_unit.reset();
		return *this;
	}

	void useTextureUnit(GLenum tex)
	{
		const size_t idx = getGlActiveTexIndex(tex);
		active_tex_unit.set(idx, true);
	}	

	void dropTextureUnit(GLenum tex) 
	{
		const size_t idx = getGlActiveTexIndex(tex);
		active_tex_unit.set(idx, false);
	}

	bool getTextureUnit(GLenum& tex)
	{
		for (size_t i = 0; i < ActiveTexturesLimit; i++) {
			if (!active_tex_unit.test(i)) {
				tex = GL_TEXTURE0 + i;
				useTextureUnit(tex);
				return true;
			}
		}

		tex = GL_INVALID_ENUM;
		return false;
	}

	static constexpr size_t ActiveTexturesLimit = 16;
	std::bitset<ActiveTexturesLimit> active_tex_unit;
};

THR_INLINE int getGlActiveTexIndex(GLenum tex) 
{
	THR_ASSERT(tex >= GL_TEXTURE0 && 
			   tex <= GL_TEXTURE0 + TexturesUnitsContext::ActiveTexturesLimit);
	return tex - GL_TEXTURE0;
}

} // namespace Thr
