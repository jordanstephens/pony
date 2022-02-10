#pragma once

#include <stdint.h>
#include <sys/types.h>
#include "../include/cstr.h"

#include "pony_cask.h"
#include "pony_cask_set.h"
#include "pony_index.h"

typedef struct {
  size_t max_generation_size;
} pony_options;

typedef struct {
  const char* path;
  pony_index index;
  pony_cask_set readers;
  pony_cask writer;
  size_t generation;
  pony_options options;
} pony_db;

pony_db pony_open(const char* path, pony_options options);
int pony_put(pony_db* self, const char* key, const char* value);
const char* pony_get(pony_db* self, const char* key);
int pony_rm(pony_db* self, const char* key);
void pony_close(pony_db* self);
