#pragma once

#include <stddef.h>
#include <stdint.h>
#include <sys/types.h>

typedef struct {
  uint16_t key_size;
  uint16_t value_size;
  const char* key;
  const char* value;
} pony_record;

size_t pony_record_size(const pony_record* record);

pony_record* pony_record_new(uint16_t key_size,
                             uint16_t value_size,
                             const char* key,
                             const char* value);
void pony_record_drop(const pony_record* record);
void pony_record_drop_keys(const pony_record* record);
