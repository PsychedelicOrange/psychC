#include "util.h"
#include <assert.h>


void hashmap_add_int(hashmap_int *hash, int key, void *value) {
  hashmap_int_entry new_entry = {key,value};
  assert(hash->size < 100);
  hash->map[hash->size++] = new_entry;
}

hashmap_int_entry *hashmap_linear_search_int(hashmap_int *hash, int key){
  assert(hash->size < 100);
  for (size_t i = 0; i < hash->size; i++) {
    if (hash->map[i].key == key) {
      return &hash->map[i];
    }
  }
  printf("\nNot found!");
  fflush(stdout);
  return NULL;
}

