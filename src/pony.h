#pragma once

#include <stdint.h>
#include <sys/types.h>

typedef struct {
  const char* path;
} pony_db;

pony_db pony_open(const char* path);
void pony_close(pony_db* self);
