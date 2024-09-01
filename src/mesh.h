// add ifnde
//
#include <stddef.h>

typedef struct vertex_env{
	float position[3];
	float normal[3];
	float tangent[4];
	float uv[2];
	float uv1[2];
}vertex_env;

typedef struct primitive_env{
	vertex_env* vertices;
	size_t vertex_count;
	unsigned short* indices;
	size_t indices_count;
}primitive_env;



