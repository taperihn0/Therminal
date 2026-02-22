cmake_minimum_required(VERSION 3.16)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

project(Therminal CXX)

if (NOT CMAKE_BUILD_TYPE)
	set(CMAKE_BUILD_TYPE Debug
		CACHE STRING "Choose one of build types: Debug, Release."
		FORCE)
	message(STATUS "CMAKE_BUILD_TYPE not specified. Defaulting to Debug.")
else ()
	message(STATUS "Build type: ${CMAKE_BUILD_TYPE}")
endif ()

message("CMake build type: ${CMAKE_BUILD_TYPE}")

file(GLOB SOURCES CONFIGURE_DEPENDS
	"${CMAKE_SOURCE_DIR}/src/*.cpp"
)

if (MSVC)
	set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /Zc:__cplusplus")
   set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /std:c++17")
	set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /permissive-")
else()
	set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fms-extensions")
endif()

add_executable(Therminal ${SOURCES})

target_compile_definitions(Therminal PUBLIC 
	$<$<CONFIG:Debug>:THR_DEBUG>
	$<$<CONFIG:Release>:THR_RELEASE>
	$<$<CONFIG:RelWithDebInfo>:THR_RELEASE>
)

set_target_properties(Therminal PROPERTIES
   VS_DEBUGGER_WORKING_DIRECTORY "${CMAKE_SOURCE_DIR}"
)

target_include_directories(Therminal PUBLIC
	"${CMAKE_SOURCE_DIR}/src"
)

# Platform detection
if (WIN32)
	target_compile_definitions(Therminal PUBLIC THR_PLATFORM_WINDOWS)
elseif (APPLE)
	target_compile_definitions(Therminal PUBLIC THR_PLATFORM_MACOS)
elseif (UNIX)
	target_compile_definitions(Therminal PUBLIC THR_PLATFORM_LINUX)
else ()
	message(FATAL_ERROR "Platform not recognised.")
endif ()

if (CMAKE_BUILD_TYPE STREQUAL "Debug")
	if (NOT MSVC)
		set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -g -Og -O0")
		set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fno-omit-frame-pointer")

		if (NOT BUILD_UTILS)
			set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fsanitize=address")
		endif ()
	else ()
		set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /Od")
	endif ()
elseif (CMAKE_BUILD_TYPE STREQUAL "Release")
	if (NOT MSVC)
		set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -O2")
	else ()
		set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /O2")
	endif ()
elseif (CMAKE_BUILD_TYPE STREQUAL "RelWithDebInfo")
	if (NOT MSVC)
		set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -O2")
	else ()
		set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /O2")
	endif()
endif ()

if (MSVC)
	set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /W3")
else ()
	set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall")
	set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wextra")
	set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wpedantic")
endif()

if (MSVC)
	# 
else ()
	set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wno-unused-function")
	set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wno-shift-count-negative")
endif ()

set(CMAKE_CONFIGURATION_TYPES "Debug;Release;Final" CACHE STRING "" FORCE)

# SIMD instruction set suppport
option(THR_ENABLE_SIMD_SSE2   "Enable SSE2 optimizations"	   OFF)
option(THR_ENABLE_SIMD_SSE3   "Enable SSE3 optimizations"	   OFF)
option(THR_ENABLE_SIMD_SSSE3  "Enable SSSE3 optimizations"	   OFF)
option(THR_ENABLE_SIMD_SSE4_1 "Enable SSE 4.1 optimizations"   OFF)
option(THR_ENABLE_SIMD_SSE4_2 "Enable SSE 4.2 optimizations"   OFF)
option(THR_ENABLE_SIMD_AVX    "Enable AVX optimizations"	      OFF)
option(THR_ENABLE_SIMD_AVX2   "Enable AVX2 optimizations"      OFF)
option(THR_ENABLE_SIMD_AVX512 "Enable AVX2 optimizations"      OFF)
option(THR_FORCE_PURE         "Force 'pure' instructions"	   OFF)

if (FORCE_PURE)
	target_compile_definitions(Therminal PUBLIC THR_FORCE_PURE)

	message(STATUS "SIMD instructions disabled")
elseif (ENABLE_SIMD_SSE2)
	target_compile_definitions(Therminal PUBLIC THR_SIMD_SSE2)

	if (MSVC)
		set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /arch:SSE2")
	else()
		set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -msse2")
	endif()

	message(STATUS "SIMD instructions enabled - SSE2 instruction set")
elseif (ENABLE_SIMD_SSE3)
	target_compile_definitions(Therminal PUBLIC THR_SIMD_SSE3)

	if (MSVC)
		set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /arch:SSE3")
	else()
		set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -msse3")
	endif()

	message(STATUS "SIMD instructions enabled - SSE3 instruction set")
elseif (ENABLE_SIMD_SSSE3)
	target_compile_definitions(Therminal PUBLIC THR_SIMD_SSSE3)

	if (MSVC)
		set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /arch:SSSE3")
	else()
		set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -mssse3")
	endif()

	message(STATUS "SIMD instructions enabled - SSSE3 instruction set")
elseif (ENABLE_SIMD_SSE4_1)
	target_compile_definitions(Therminal PUBLIC THR_SIMD_SSE4_1)

	if (MSVC)
		set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /arch:SSE4.1")
	else()
		set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -msse4.1")
	endif()

	message(STATUS "SIMD instructions enabled - SSE4.1 instruction set")
elseif (ENABLE_SIMD_SSE4_2)
	target_compile_definitions(Therminal PUBLIC THR_SIMD_SSE4_2)

	if (MSVC)
		set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /arch:SSE4.2")
	else()
		set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -msse4.2")
	endif()

	message(STATUS "SIMD instructions enabled - SSE4.2 instruction set")
elseif (ENABLE_SIMD_AVX)
	target_compile_definitions(Therminal PUBLIC THR_SIMD_AVX)

	if (MSVC)
		set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /arch:AVX")
	else()
		set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -mavx")
	endif()

	message(STATUS "SIMD instructions enabled - AVX instruction set")
elseif (ENABLE_SIMD_AVX2)
	target_compile_definitions(Therminal PUBLIC THR_SIMD_AVX2)

	if (MSVC)
		set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /arch:AVX2")
	else()
		set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -mavx2")
	endif()

	message(STATUS "SIMD instructions enabled - AVX2 instruction set")
elseif (ENABLE_SIMD_AVX512)
	target_compile_definitions(Therminal PUBLIC THR_SIMD_AVX512)

	if (MSVC)
		set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /arch:AVX512")
	else()
		set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -mavx512")
	endif()

	message(STATUS "SIMD instructions enabled - AVX512 instruction set")
endif()

set_target_properties(Therminal PROPERTIES
	RUNTIME_OUTPUT_DIRECTORY 
	"${CMAKE_SOURCE_DIR}/bin/${PROJECT_NAME}/${CMAKE_BUILD_TYPE}"
	LIBRARY_OUTPUT_DIRECTORY 
	"${CMAKE_SOURCE_DIR}/bin/${PROJECT_NAME}/${CMAKE_BUILD_TYPE}"
	ARCHIVE_OUTPUT_DIRECTORY 
	"${CMAKE_SOURCE_DIR}/bin/intermediates/${PROJECT_NAME}/${CMAKE_BUILD_TYPE}"
)

message(STATUS "Compiler flags: ${CMAKE_CXX_FLAGS}")
message("Building Therminal sources")

