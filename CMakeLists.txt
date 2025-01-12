cmake_minimum_required(VERSION 3.15)

project(
	psychspiration
	VERSION 1.0
	DESCRIPTION "game engine"
	LANGUAGES C
	)
# generate compile_compands for clangd
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

file(GLOB_RECURSE srcfiles src/*.c src/*.h)

add_executable(psychspiration ${srcfiles})

target_compile_options(psychspiration PRIVATE -Wall -g)

target_include_directories(psychspiration PRIVATE src)

find_package(OpenAL REQUIRED)
# find_package(cglm CONFIG REQUIRED)
target_link_libraries( psychspiration
	PRIVATE
	glfw3
	X11 # for glfw3
	m # math
	OpenAL::OpenAL
	cglm
	)


