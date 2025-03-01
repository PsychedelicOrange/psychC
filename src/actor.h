#ifndef ACTOR_H
#define ACTOR_H
#include <stddef.h>
#include <cglm/cglm.h>
#include <cglm/struct.h>
#include "cgltf.h"
#include "util.h"

typedef struct vertex_actor{
	float position[3];
	float normal[3];
	float tangent[4];
	float uv[2];
	unsigned short joints[4];
	float weights[4];
}vertex_actor;

typedef struct primitive_actor{
	vertex_actor* vertices;
	size_t vertex_count;
	unsigned short* indices;
	size_t indices_count;
}primitive_actor;

typedef struct drawable_primitive_actor{
	unsigned int shaderProgram;
	unsigned int vao;
	size_t indices_count;
}drawable_primitive_actor;

typedef struct joint{
	char* name;
	int gltf_index;
	struct joint* parent;
	float translation[3];
	vec4s rotation;
	mat4s transform;
}joint;

typedef enum animation_property{
	prop_translation,
	prop_rotation,
	prop_scale,
	prop_weights
}animation_property;

typedef enum interpolation{
	LINEAR,
	STEP,
	CUBICSPLINE
}interpolation;

typedef struct sampler{
	buffer keyframes;
	buffer data;
	size_t element_size;
	interpolation interpolation;
}sampler;

typedef struct channel{
	sampler sampler;
	void* data_ptr;
	animation_property property;
}channel;

typedef struct animation{
	channel* channels;
	size_t channels_count;
	float started ;
}animation;

// all arrays in this struct are stored in the order of joints in the skin gltf model array
typedef struct skin{
	mat4s inverseBindMatrices[100];
	joint* joint_refs[100];
	size_t joints_count;
	buffer topo_order;
	vec3 joint_translation_rest[100];
	vec4 joint_rotation_rest[100];
}skin;

typedef struct mesh_actor{
	primitive_actor* primitives;
	size_t primitives_count ;
	skin* skin_ref;
}mesh_actor;

typedef struct model_actor{
	mesh_actor* meshes;
	size_t meshes_count;
	hashmap_int joints;
	skin* skins;
	size_t skins_count;
	animation animations[10];
	size_t animations_count;
}model_actor;

typedef struct drawable_mesh{
	skin* skin_ref;
	drawable_prim* prims;
	size_t prims_count;
	// used to store calculated jointMatrices from joints to upload to shader
	float* jointMatricesData;
}drawable_mesh;

typedef struct drawable_model{
	drawable_mesh* meshes;
	size_t meshes_count;
	hashmap_int* joints;
	animation animations[10];
	size_t animations_count;
}drawable_model;


model_actor load_model_actor(char* model_path);
drawable_model upload_model_actor(model_actor* model);
void draw_model(drawable_model* dmodel,unsigned int shaderProgram);
void draw_single_primitive_actor(unsigned int shaderProgram,drawable_prim d);

void print_vertex_actor(vertex_actor a);
void print_sampler(sampler samp);
void print_joint(joint* j);

#endif
