#ifndef UTIL_H
#define UTIL_H
#include <stddef.h>

/* Convinience structs for in out */

typedef struct drawable_prim{
	unsigned int vao;
	size_t indices_count;
}drawable_prim;

typedef struct buffer{
	void*  data;
	size_t size;
}buffer;

 /* naive hashmap */

typedef struct hashmap_int_entry{
    int key;
    void* value;
} hashmap_int_entry;

typedef struct hashmap_int{
	hashmap_int_entry map[100];
	size_t size;
} hashmap_int;


/* pretty print vectors and matrices */
void print_vec3(float* vec);
void print_vec4(float* vec);
void print_mat4(float* mat);

/* Puts value in key if key exists and returns 1; if key doesn't exists adds it and returns 0 */
int hashmap_upsert(hashmap_int *hash, int key, void *value);
/* puts key value in map if doesnt exist and returns 0; if key exists doesnt do anything and returns 1; */
int hashmap_insert(hashmap_int *hash, int key, void *value);
void* hashmap_get(hashmap_int* hash, int n);

#endif
