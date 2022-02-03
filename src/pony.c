#include "pony.h"

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "../include/cstr.h"

#include "pony_record.h"

pony_db pony_open(const char* path_arg) {
  const char* path = strdup(path_arg);
  pony_cask writer = pony_cask_writer_open(path);
  assert(writer.fd > 2);
  pony_cask reader = pony_cask_reader_open(path);
  assert(reader.fd > 2);

  pony_db self = {
      .path = path,
      .index = pony_index_init(),
      .writer = writer,
      .reader = reader,
  };

  pony_index_load_cask(&self.index, &self.reader);
  return self;
}

int pony_put(pony_db* self, const char* key, const char* value) {
  size_t key_size = strlen(key);
  size_t value_size = strlen(value);
  pony_record record = {
      .key_size = key_size,
      .value_size = value_size,
      .key = strndup(key, key_size),
      .value = strndup(value, value_size),
  };
  pony_cask_entry cask_entry = pony_cask_append(&self->writer, &record);
  if (cask_entry.offset == 0 && cask_entry.size == 0) {
    return -1;
  }

  pony_index_entry index_entry = {
      .offset = cask_entry.offset,
      .size = cask_entry.size,
  };
  pony_index_result result =
      pony_index_put(&self->index, cstr_from(key), index_entry);
  pony_record_drop_keys(&record);
  return 0;
}

const char* pony_get(pony_db* self, const char* key) {
  const pony_index_value* result = pony_index_get(&self->index, key);
  if (NULL == result) {
    return NULL;
  }
  pony_index_entry entry = result->second;
  pony_record record;
  ssize_t bytes_read =
      pony_cask_read_record(&record, &self->reader, entry.offset, entry.size);
  assert(bytes_read > 0);
  const char* value = strndup(record.value, record.value_size);
  pony_record_drop_keys(&record);
  return value;
}

size_t pony_rm(pony_db* self, const char* key) {
  size_t found = pony_index_erase(&self->index, key);
  return found;
}

void pony_close(pony_db* self) {
  pony_index_drop(&self->index);
  free((char*)self->path);
}
