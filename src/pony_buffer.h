#pragma once

#include <stdlib.h>

#define PAGE_SIZE 1 << 12

typedef struct {
  uint8_t data[PAGE_SIZE];
  uint16_t length;
  uint16_t capacity;
} pony_buffer;

typedef struct {
  pony_buffer* buffer;
  size_t pos;
} pony_buffer_cursor;

pony_buffer pony_buffer_new();
pony_buffer_cursor pony_buffer_cursor_new(pony_buffer* buffer, size_t pos);
uint8_t* pony_buffer_cursor_ptr(pony_buffer_cursor* self);

void pony_buffer_write_u16(pony_buffer_cursor* cursor, uint16_t* value);
void pony_buffer_write_u32(pony_buffer_cursor* cursor, uint32_t* value);
void pony_buffer_write_str(pony_buffer_cursor* cursor,
                           const char* value,
                           size_t size);

uint16_t pony_buffer_read_u16(pony_buffer_cursor* cursor);
uint32_t pony_buffer_read_u32(pony_buffer_cursor* cursor);
const char* pony_buffer_read_str(pony_buffer_cursor* cursor, size_t size);
size_t pony_buffer_cursor_remaining(pony_buffer_cursor* cursor);
