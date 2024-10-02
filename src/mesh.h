// add ifnde
//
#include <stddef.h>
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

typedef struct joint{
	char* name;
	int index;
	int children[10];
	size_t children_count;
	float translation[4];
	float inverseBindMatrice[4][4];
	float rotation[4];
}joint;


typedef enum animation_property{
	prop_translation,
	prop_rotation,
	prop_scale,
	prop_weights
}animation_property;

typedef enum interpolation{
	STEP,
	LINEAR,
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

typedef struct mesh_actor{
	primitive_actor* primitives;
	size_t primitives_count ;
	joint* joints;
	size_t joints_count ;
	animation animations[10];
	size_t animations_count ;
}mesh_actor;
