#include "pony_buffer.h"

#include <string.h>

pony_buffer pony_buffer_new() {
  return (pony_buffer){
      .data = {0},
      .length = 0,
      .capacity = PAGE_SIZE,
  };
}

pony_buffer_cursor pony_buffer_cursor_new(pony_buffer* buffer, size_t pos) {
  return (pony_buffer_cursor){
      .buffer = buffer,
      .pos = pos,
  };
}

uint8_t* pony_buffer_cursor_ptr(pony_buffer_cursor* self) {
  return self->buffer->data + self->pos;
}

void pony_buffer_write_u16(pony_buffer_cursor* cursor, uint16_t* value) {
  size_t size = sizeof(uint16_t);
  memcpy(cursor->buffer->data + cursor->pos, value, size);
  cursor->pos += size;
  cursor->buffer->length += size;
}

void pony_buffer_write_u32(pony_buffer_cursor* cursor, uint32_t* value) {
  size_t size = sizeof(uint32_t);
  memcpy(cursor->buffer->data + cursor->pos, value, size);
  cursor->pos += size;
  cursor->buffer->length += size;
}

void pony_buffer_write_str(pony_buffer_cursor* cursor,
                           const char* value,
                           size_t size) {
  memcpy(cursor->buffer->data + cursor->pos, value, size);
  cursor->pos += size;
  cursor->buffer->length += size;
}

uint16_t pony_buffer_read_u16(pony_buffer_cursor* cursor) {
  size_t size = sizeof(uint16_t);
  uint16_t value;
  memcpy(&value, cursor->buffer->data + cursor->pos, size);
  cursor->pos += size;
  return value;
}

uint32_t pony_buffer_read_u32(pony_buffer_cursor* cursor) {
  size_t size = sizeof(uint32_t);
  uint32_t value;
  memcpy(&value, cursor->buffer->data + cursor->pos, size);
  cursor->pos += size;
  return value;
}

const char* pony_buffer_read_str(pony_buffer_cursor* cursor, size_t size) {
  const char* value = malloc(size);
  memcpy((void*)value, cursor->buffer->data + cursor->pos, size);
  cursor->pos += size;
  return value;
}

size_t pony_buffer_cursor_remaining(pony_buffer_cursor* cursor) {
  return cursor->buffer->length - cursor->pos;
}
