cmake_minimum_required(VERSION 3.16)
project(Glad C)

add_library(glad 
   include/glad/glad.h 
   src/glad.c
)

if (NOT MSVC)
   target_compile_options(glad PRIVATE -fPIC)
endif()

target_include_directories(glad PUBLIC 
   include/
)