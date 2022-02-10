
#pragma once

#include "../include/cstr.h"
#include "pony_buffer.h"
#include "pony_record.h"

typedef enum { PONY_CASK_MODE_READ, PONY_CASK_MODE_WRITE } pony_cask_mode;

typedef struct {
  size_t offset;
  size_t size;
  size_t generation;
} pony_cask_entry;

typedef struct {
  int fd;
  size_t offset;
  pony_cask_mode mode;
  size_t generation;
} pony_cask;

typedef struct {
  pony_cask* reader;
  pony_buffer buffer;
  size_t offset;
  size_t page_size;
} pony_cask_buf_reader;

pony_cask pony_cask_writer_open(const char* directory_path, size_t generation);
pony_cask pony_cask_reader_open(const char* directory_path, size_t generation);
pony_cask_entry pony_cask_append(pony_cask* writer, pony_record* record);
ssize_t pony_cask_read_record(pony_cask* reader,
                              pony_record* record,
                              size_t offset,
                              size_t size);

pony_cask_buf_reader pony_cask_buf_reader_new(pony_cask* reader);
ssize_t pony_cask_buf_reader_next(pony_record* record,
                                  pony_cask_buf_reader* buf_reader);

void pony_cask_close(pony_cask* cask);
