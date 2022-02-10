#include "pony_cask.h"

#include <assert.h>
#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <unistd.h>

#include "pony_crc32.h"
#include "pony_record.h"

#define eprintf(...) fprintf(stderr, __VA_ARGS__)

const uint8_t PONY_RECORD_HEADER_SIZE = 2 * sizeof(uint16_t);

int mkdir_p(const char* path);
size_t pony_record_serialize(pony_buffer* buffer, const pony_record* record);
size_t pony_record_deserialize(pony_record* record,
                               pony_buffer* buffer,
                               size_t position);

pony_cask_entry pony_cask_entry_empty();

const char* cask_path(const char* directory_path, size_t generation) {
  size_t base_length = strlen(directory_path);
  cstr base_path = cstr_from_n(directory_path, base_length);
  cstr* path = &base_path;
  if (directory_path[base_length - 1] != '/') {
    path = cstr_append(path, "/");
  }
  cstr basename = cstr_from_fmt("%zu.pony", generation);
  path = cstr_append_s(path, basename);
  const char* copy = strndup(cstr_str(path), cstr_length(*path));
  cstr_drop(path);
  return copy;
}

int access_mode(pony_cask_mode mode) {
  switch (mode) {
    case PONY_CASK_MODE_READ:
      return O_RDONLY;
    case PONY_CASK_MODE_WRITE:
      return O_WRONLY | O_CREAT | O_APPEND;
    default:
      return -1;
  }
}

pony_cask pony_cask_open(const char* directory_path, size_t generation, pony_cask_mode mode) {
  assert(0 == mkdir_p(directory_path));

  const char* path = cask_path(directory_path, generation);
  int fmode = access_mode(mode);
  if (fmode == -1) {
    eprintf("Illegal cask mode: %d\n", mode);
    exit(-1);
  }

  int fd = open(path, fmode, S_IRWXU);
  if (fd == -1) {
    eprintf("Failed to open cask file: %s\n", path);
    perror("open");
    exit(-1);
  }
  free((char*)path);
  assert(fd > 2);

  pony_cask cask = {
      .fd = fd,
      .offset = 0,
      .mode = mode,
      .generation = generation,
  };

  return cask;
}

pony_cask pony_cask_writer_open(const char* directory_path, size_t generation) {
  return pony_cask_open(directory_path, generation, PONY_CASK_MODE_WRITE);
}

pony_cask pony_cask_reader_open(const char* directory_path, size_t generation) {
  return pony_cask_open(directory_path, generation, PONY_CASK_MODE_READ);
}

pony_cask_entry pony_cask_append(pony_cask* writer, pony_record* record) {
  assert(writer->mode == PONY_CASK_MODE_WRITE);
  assert(writer->fd > 2);

  size_t size = pony_record_size(record);
  assert(size > 0);

  pony_buffer buffer = pony_buffer_new();
  size_t bytes_serialized = pony_record_serialize(&buffer, record);
  assert(bytes_serialized == size);

  ssize_t written = write(writer->fd, &buffer.data, size);
  if (-1 == written) {
    eprintf("Failed to append %zu bytes to cask with fd %d\n", size, writer->fd);
    perror("write");
    return pony_cask_entry_empty();
  }

  assert((size_t)written == size);
  pony_cask_entry entry = {
      .offset = writer->offset,
      .size = size,
  };

  writer->offset += (size_t)written;
  return entry;
}

ssize_t pony_cask_read_record(pony_cask* reader,
                              pony_record* record,
                              size_t offset,
                              size_t size) {
  assert(reader->mode == PONY_CASK_MODE_READ);
  assert(reader->fd > 2);

  off_t pos = lseek(reader->fd, offset, SEEK_SET);
  if (pos != (off_t)offset) {
    eprintf("Failed to seek cask reader to offset %zu\n", offset);
    perror("lseek");
    return -1;
  }

  pony_buffer buffer = pony_buffer_new();
  ssize_t bytes_read = read(reader->fd, &buffer, size);
  if (bytes_read == -1) {
    eprintf("Failed to read from cask %zu bytes at offset %zu\n", size, offset);
    perror("read");
    return -1;
  }
  assert((size_t)bytes_read == size);
  buffer.length = bytes_read;
  reader->offset += bytes_read;
  return pony_record_deserialize(record, &buffer, 0);
}

ssize_t pony_cask_read_block(pony_cask* reader,
                             pony_buffer* buffer,
                             size_t size) {
  assert(reader->mode == PONY_CASK_MODE_READ);
  assert(reader->fd > 2);

  off_t pos = lseek(reader->fd, reader->offset, SEEK_SET);
  if ((size_t)pos != reader->offset) {
    eprintf("Failed to seek cask reader to offset %zu\n", reader->offset);
    perror("lseek");
    return -1;
  }

  ssize_t bytes_read = read(reader->fd, &buffer->data, size);
  if (bytes_read == -1) {
    eprintf("Failed to read block from cache at offset %zu with size %zu\n",
            reader->offset, size);
    perror("read");
    return -1;
  }
  buffer->length = bytes_read;
  reader->offset += bytes_read;
  return bytes_read;
}

pony_cask_buf_reader pony_cask_buf_reader_new(pony_cask* reader) {
  assert(reader->mode == PONY_CASK_MODE_READ);
  pony_buffer buffer = pony_buffer_new();
  pony_cask_buf_reader buf_reader = {
      .reader = reader,
      .buffer = buffer,
      .offset = 0,
      .page_size = PAGE_SIZE,
  };
  return buf_reader;
}

ssize_t pony_cask_buf_reader_next(pony_record* record,
                                  pony_cask_buf_reader* buf_reader) {
  size_t bytes_serialized =
      pony_record_deserialize(record, &buf_reader->buffer, buf_reader->offset);
  if (bytes_serialized != 0) {
    buf_reader->offset += bytes_serialized;
    return bytes_serialized;
  }

  buf_reader->offset = 0;
  ssize_t bytes_read = pony_cask_read_block(
      buf_reader->reader, &buf_reader->buffer, buf_reader->page_size);
  // Error
  if (bytes_read == -1)
    return -1;
  // EOF
  if (bytes_read == 0)
    return 0;

  return pony_cask_buf_reader_next(record, buf_reader);
}

size_t pony_record_serialize(pony_buffer* buffer, const pony_record* record) {
  const size_t body_size = record->key_size + record->value_size;

  pony_buffer_cursor cursor = pony_buffer_cursor_new(buffer, 0);

  pony_buffer_write_u16(&cursor, (uint16_t*)&record->key_size);
  pony_buffer_write_u16(&cursor, (uint16_t*)&record->value_size);
  pony_buffer_write_str(&cursor, record->key, record->key_size);
  pony_buffer_write_str(&cursor, record->value, record->value_size);

  const size_t record_size = PONY_RECORD_HEADER_SIZE + body_size;
  uint8_t* buffer_ptr = pony_buffer_cursor_ptr(&cursor);
  uint32_t crc = pony_crc32(buffer_ptr - record_size, record_size);
  pony_buffer_write_u32(&cursor, &crc);

  return cursor.pos;
}

size_t pony_record_deserialize(pony_record* record,
                               pony_buffer* buffer,
                               size_t position) {
  pony_buffer_cursor cursor = pony_buffer_cursor_new(buffer, position);
  if (pony_buffer_cursor_remaining(&cursor) < PONY_RECORD_HEADER_SIZE) {
    return 0;
  }

  record->key_size = pony_buffer_read_u16(&cursor);
  record->value_size = pony_buffer_read_u16(&cursor);
  size_t body_size = record->key_size + record->value_size;
  if (pony_buffer_cursor_remaining(&cursor) < body_size + sizeof(uint32_t)) {
    return 0;
  }

  record->key = pony_buffer_read_str(&cursor, record->key_size);
  record->value = pony_buffer_read_str(&cursor, record->value_size);

  uint8_t* buffer_ptr = pony_buffer_cursor_ptr(&cursor);
  size_t record_size = PONY_RECORD_HEADER_SIZE + body_size;
  uint32_t crc = pony_crc32(buffer_ptr - record_size, record_size);
  uint32_t check_crc = pony_buffer_read_u32(&cursor);
  assert(crc == check_crc);

  return cursor.pos - position;
}

pony_cask_entry pony_cask_entry_empty() {
  return (pony_cask_entry){
      .offset = 0,
      .size = 0,
  };
}

void pony_cask_close(pony_cask* cask) {
  close(cask->fd);
  cask->fd = -1;
}

int mkdir_p(const char* path) {
  if (0 == mkdir(path, S_IRWXU))
    return 0;
  if (errno == EEXIST)
    return 0;
  perror("mkdir_p");
  return errno;
}
