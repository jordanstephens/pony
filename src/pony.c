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
    .path = path
  };

  return self;
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
