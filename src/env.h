#ifndef ENV_H_INCLUDED__
#define ENV_H_INCLUDED__

#include <stddef.h>
#include "cgltf.h"
#include "util.h"

#define MAX_PRIMITIVES 1000


typedef struct vertex_env{
	float position[3];
	float normal[3];
	float tangent[4];
	float uv[2];
	float uv1[2];
}vertex_env;

// single renderable unit
typedef struct primitive_env{
	vertex_env* vertices;
	size_t vertex_count;
	unsigned short* indices;
	size_t indices_count;
	// index into large buffer
	size_t base_vertex;
	size_t index_index; // lol
}primitive_env;

// Load model from file to memory. 
size_t load_model_env(char* model_path, primitive_env* primitives, size_t primitive_index);

drawable_mesh upload_single_primitive_env(primitive_env p);
void draw_single_primitive_env(unsigned int shaderProgram,drawable_mesh d);

// batches multiple primitives vertex data into one buffer
unsigned int upload_multiple_primitive_env(primitive_env* primitives, size_t primitive_count);
void draw_multiple_primitive_env(unsigned int shaderProgram,unsigned int env_vao,primitive_env* primitives, size_t primitive_count);

#endif

