#include "util.h"
#include <assert.h>
#include <stdlib.h>

#include "log.h"
void print_vec3(float* vec){
    logd("(%f,%f,%f)",vec[0],vec[1],vec[2]);
}
void print_vec4(float* vec){
    logd("(%f,%f,%f,%f)",vec[0],vec[1],vec[2],vec[3]);
}
void print_mat4(float* mat){
    logd("[");
    for(int i =0; i < 4; i++){
	for(int j =0; j < 4; j++){
	    logd("%f,",mat[(4*i)+j]);
	}
	logd("");
    }
    logd("]");
}

int hashmap_upsert(hashmap_int *hash, int key, void *value) {
  assert(hash->size < 100);
  for (size_t i = 0; i < hash->size; i++) {
    if (hash->map[i].key == key) {
		// possible leak
		hash->map[i].value = value;
		return 0;
    }
  }
  hashmap_int_entry new_entry = {key,value};
  assert(hash->size < 100);
  hash->map[hash->size++] = new_entry;
  return 1;
}

int hashmap_insert(hashmap_int *hash, int key, void *value) {
  assert(hash->size < 100);
  for (size_t i = 0; i < hash->size; i++) {
    if (hash->map[i].key == key) {
		return 0;
    }
  }
  hashmap_int_entry new_entry = {key,value};
  assert(hash->size < 100);
  hash->map[hash->size++] = new_entry;
  return 1;
}

void* hashmap_get(hashmap_int *hash, int key){
  assert(hash->size < 100);
  for (size_t i = 0; i < hash->size; i++) {
    if (hash->map[i].key == key) {
      return hash->map[i].value;
    }
  }
  return NULL;
}
