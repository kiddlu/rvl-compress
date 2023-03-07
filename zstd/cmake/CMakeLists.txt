cmake_minimum_required( VERSION 3.16.0 )

project(zstd LANGUAGES C VERSION 1.5.2)

add_library(zstd STATIC
	zstd.h
	zstd.c
)

if (CMAKE_CXX_COMPILER_ID STREQUAL "Clang")
	target_compile_options(${PROJECT_NAME} PRIVATE "-Wno-unused-function")
endif()
