#include "pony.h"

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "../include/cstr.h"

#include "pony_record.h"

#define DEFAULT_MAX_GENERATION_SIZE (1 << 30)

pony_cask_entry pony_persist_record(pony_db* self, pony_record* record);

pony_options process_options(pony_options options) {
  options.max_generation_size = options.max_generation_size
    ? options.max_generation_size
    : DEFAULT_MAX_GENERATION_SIZE;

  return options;
}

pony_db pony_open(const char* path_arg, pony_options options) {
  const char* path = strdup(path_arg);
  size_t generation = 1;
  pony_cask writer = pony_cask_writer_open(path, generation);
  assert(writer.fd > 2);
  pony_cask reader = pony_cask_reader_open(path, generation);
  assert(reader.fd > 2);

  pony_cask_set readers = pony_cask_set_with_capacity(8);
  pony_cask_set_put(&readers, generation, reader);

  pony_db self = {
      .path = path,
      .index = pony_index_init(),
      .writer = writer,
      .readers = readers,
      .generation = generation,
      .options = process_options(options),
  };

  pony_index_load_cask(&self.index, &self.readers);
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
  pony_cask_entry cask_entry = pony_persist_record(self, &record);
  if (cask_entry.offset == 0 && cask_entry.size == 0) {
    return -1;
  }

  pony_index_entry index_entry = {
      .offset = cask_entry.offset,
      .size = cask_entry.size,
      .generation = self->writer.generation,
  };
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
  const pony_cask_set_value* cask_set_entry = pony_cask_set_get(&self->readers, entry.generation);
  assert(cask_set_entry != NULL);
  pony_cask reader = cask_set_entry->second;
  pony_record record;
  ssize_t bytes_read =
      pony_cask_read_record(&reader, &record, entry.offset, entry.size);
  assert(bytes_read > 0);
  const char* value = strndup(record.value, record.value_size);
  pony_record_drop_keys(&record);
  return value;
}

int pony_rm(pony_db* self, const char* key) {
  size_t found = pony_index_erase(&self->index, key);
  if (0 == found) return 0;
  uint16_t key_size = strlen(key);
  pony_record tombstone = pony_record_tombstone(key_size, key);
  pony_cask_entry cask_entry = pony_cask_append(&self->writer, &tombstone);
  if (cask_entry.offset == 0 && cask_entry.size == 0) {
    return -1;
  }

  return 1;
}

pony_cask_entry pony_persist_record(pony_db* self, pony_record* record) {
  size_t record_size = pony_record_size(record);
  size_t next_size = self->writer.offset + record_size;
  if (next_size <= self->options.max_generation_size) {
    return pony_cask_append(&self->writer, record);
  }

  pony_cask_close(&self->writer);
  self->generation++;
  pony_cask writer = pony_cask_writer_open(self->path, self->generation);
  memcpy(&self->writer, &writer, sizeof(pony_cask));
  pony_cask_entry cask_entry = pony_cask_append(&self->writer, record);
  pony_cask reader = pony_cask_reader_open(self->path, self->generation);
  pony_cask_set_put(&self->readers, self->generation, reader);
  return cask_entry;
}

void pony_close(pony_db* self) {
  pony_cask_close(&self->writer);
  c_foreach (it, pony_cask_set, self->readers) {
    const pony_cask_set_value* cask_set_entry = it.ref;
    pony_cask reader = cask_set_entry->second;
    pony_cask_close(&reader);
  }
  pony_index_drop(&self->index);
  free((char*)self->path);
}
