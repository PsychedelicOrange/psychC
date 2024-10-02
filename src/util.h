#ifndef UTIL_H
#define UTIL_H
#include <stddef.h>
// convinience structs  for in out
typedef struct drawable_mesh{
	unsigned int vao;
	size_t indices_count;
}drawable_mesh;

typedef struct buffer{
	void*  data;
	size_t size;
}buffer;
#endif
