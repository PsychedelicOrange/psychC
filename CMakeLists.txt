cmake_minimum_required(VERSION 3.15)

project(
	psychspiration
	VERSION 1.0
	DESCRIPTION "game engine"
	LANGUAGES CXX C
	)

file(GLOB_RECURSE srcfiles     src/*.c src/*.h)

add_executable(psychspiration 
	${srcfiles}
	)

target_compile_options(psychspiration PRIVATE -Wall )

target_include_directories(psychspiration PRIVATE src)

find_package(glm CONFIG REQUIRED)
target_link_libraries(
	psychspiration
	PRIVATE
	glfw3
	X11 # for glfw3 linux
	m # math
	cglm
	)
