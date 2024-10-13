#ifndef UTIL_H
#define UTIL_H
#include <stddef.h>
#include <stdio.h>
#include <string.h>
// convinience structs for in out
typedef struct drawable_prim{
	unsigned int vao;
	size_t indices_count;
}drawable_prim;

typedef struct buffer{
	void*  data;
	size_t size;
}buffer;

typedef struct hashmap_int_entry{
    int key;
    void* value;
} hashmap_int_entry;

typedef struct hashmap_int{
	hashmap_int_entry map[100];
	size_t size;
} hashmap_int;

void hashmap_add_int(hashmap_int *hash, int key, void *value);

hashmap_int_entry *hashmap_linear_search_int(hashmap_int *hash, int key);

#endif
