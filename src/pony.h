#pragma once

#include <stdint.h>
#include <sys/types.h>
#include "pony_index.h"

typedef struct {
  const char* path;
  pony_index index;
} pony_db;

pony_db pony_open(const char* path);
void pony_put(pony_db* self, const char* key, const char* value);
const char* pony_get(pony_db* self, const char* key);
size_t pony_rm(pony_db* self, const char* key);
void pony_close(pony_db* self);
