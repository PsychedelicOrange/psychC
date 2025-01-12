#ifndef CGLTF_HELPER_H
#define CGLTF_HELPER_H

#include "util.h"
#include "glad/glad.h"
#include "cgltf.h"

buffer load_accessor(cgltf_accessor* acc);
size_t getBytesPerElement(cgltf_accessor* acc);
void print_indices(cgltf_accessor* indices);
void print_accessor(cgltf_accessor *accessor );

#endif
