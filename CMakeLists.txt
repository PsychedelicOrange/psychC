cmake_minimum_required(VERSION 3.15)

project(
	psychspiration
	VERSION 1.0
	DESCRIPTION "game engine"
	LANGUAGES C
	)

add_executable(psychspiration 
	src/main.c
	src/glad.c)

target_include_directories(psychspiration PRIVATE src)

target_link_libraries(
	psychspiration
	PRIVATE
	glfw3
	X11 # for glfw3 linux
	m # math
	)
