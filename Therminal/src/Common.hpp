#pragma once

#include "Assert.hpp"
#include "Types.hpp"
#include "Defines.hpp"

/* GLM headers */
#include "glm/vec2.hpp"
#include "glm/vec3.hpp"
#include "glm/vec4.hpp"

#define UNI_ALGO_STATIC_DATA
#include "uni_algo/ranges_grapheme.h"
#include "uni_algo/ranges_word.h"
#include "uni_algo/conv.h"

#if defined(THR_PLATFORM_WINDOWS)
#  include <windows.h>
#else
#  include <unistd.h>
#endif

#if defined(THR_PLATFORM_WINDOWS)
#	include <windows.h>
#	undef min
#	undef max
#endif

#define GLFW_INCLUDE_NONE
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <ft2build.h>
#include FT_FREETYPE_H
#include FT_TRUETYPE_TABLES_H

#include <cmath>
#include <type_traits>
