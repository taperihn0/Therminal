#pragma once

#include "Defines.hpp"

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

#include <cmath>
