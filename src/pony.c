#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "pony.h"

const uint8_t MAX_DIR_LEN = 255;

char* str_cp(const char*);

pony_db pony_open(const char* path_arg) {
  const char* path = str_cp(path_arg);
  pony_db self = {
    .path = path,
    .index = pony_index_init(),
  };

  return self;
}

void pony_put(pony_db* self, const char* key, const char* value) {
  pony_index_result result = pony_index_put(&self->index, cstr_from(key), (pony_index_entry){
    .value = value,
  });
  assert(result.inserted);
}

const char* pony_get(pony_db* self, const char* key) {
  const pony_index_value* result = pony_index_get(&self->index, key);
  if (NULL == result) {
    return NULL;
  }
  pony_index_entry entry = result->second;
  return entry.value;
}

size_t pony_rm(pony_db* self, const char* key) {
  size_t found = pony_index_erase(&self->index, key);
  return found;
}

void pony_close(pony_db* self) {
  pony_index_drop(&self->index);
  free((char*)self->path);
}

char* str_cp(const char* src) {
  size_t srclen = strnlen(src, MAX_DIR_LEN);
  assert(srclen > 0);
  assert(srclen < MAX_DIR_LEN);
  size_t dstlen = srclen + 1;
  char* dst = malloc(dstlen);
  assert(dst != NULL);
  size_t n = strlcpy(dst, src, dstlen);
  assert(n == srclen);
  return dst;
}
