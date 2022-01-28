#pragma once

#include <sys/types.h>
#include <stdint.h>

typedef struct {
  const char* value;
} pony_index_entry;

#define _i_prefix pony_
#define i_key_str
#define i_val pony_index_entry
#define i_tag index

#include "../include/cmap.h"
