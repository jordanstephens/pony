#pragma once

#include <stdint.h>
#include <sys/types.h>
#include "../include/cstr.h"

#include "pony_cask.h"
#include "pony_index.h"

typedef struct {
  const char* path;
  pony_index index;
  pony_cask reader;
  pony_cask writer;
} pony_db;

pony_db pony_open(const char* path);
int pony_put(pony_db* self, const char* key, const char* value);
const char* pony_get(pony_db* self, const char* key);
int pony_rm(pony_db* self, const char* key);
void pony_close(pony_db* self);
