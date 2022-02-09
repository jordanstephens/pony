#include "pony_record.h"

#include <stdint.h>
#include <stdlib.h>
#include <string.h>

const uint8_t CRC_SIZE = sizeof(uint32_t);
size_t pony_record_size(const pony_record* record) {
  return CRC_SIZE + sizeof(record->key_size) + sizeof(record->value_size) +
         record->key_size + record->value_size;
}

pony_record* pony_record_new(uint16_t key_size,
                             uint16_t value_size,
                             const char* key,
                             const char* value) {
  pony_record* record = malloc(sizeof(pony_record));
  record->key_size = key_size;
  record->value_size = value_size;
  record->key = strndup(key, key_size);
  record->value = strndup(value, value_size);
  return record;
}

pony_record pony_record_tombstone(uint16_t key_size, const char* key) {
  return (pony_record){
      .key_size = key_size,
      .value_size = 0,
      .key = key,
      .value = NULL,
  };
}

bool pony_record_is_tombstone(pony_record* record) {
  return record->value_size == 0;
}

void pony_record_drop(const pony_record* record) {
  pony_record_drop_keys(record);
  free((pony_record*)record);
}

void pony_record_drop_keys(const pony_record* record) {
  free((char*)record->key);
  free((char*)record->value);
}
