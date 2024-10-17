#include "util.h"
#include <assert.h>
#include <stdlib.h>

int hashmap_reput(hashmap_int *hash, int key, void *value) {
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

int hashmap_put(hashmap_int *hash, int key, void *value) {
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
