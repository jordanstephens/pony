#pragma once

#include <stdint.h>
#include <sys/types.h>

typedef struct {
  size_t offset;
  size_t size;
} pony_index_entry;

// TODO: #define i_valdrop to make sure values get freed on close
// TODO: #define i_keydrop to make sure keys get freed on close
#define _i_prefix pony_
#define i_key_str
#define i_val pony_index_entry
#define i_tag index

#include "../include/cmap.h"

#include "pony_cask.h"

void pony_index_load_cask(pony_index* self, pony_cask* cask);
