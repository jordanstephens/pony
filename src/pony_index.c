
#include "pony_index.h"

#include <assert.h>

#include "pony_cask.h"
#include "pony_record.h"

void pony_index_load_cask(pony_index* self, pony_cask* reader) {
  assert(reader->offset == 0);

  pony_cask_buf_reader buf_reader = pony_cask_buf_reader_new(reader);
  while (true) {
    pony_record record;
    ssize_t bytes_read = pony_cask_buf_reader_next(&record, &buf_reader);
    if (bytes_read == 0)
      break;
    pony_index_entry entry = {
        .offset = reader->offset - bytes_read,
        .size = bytes_read,
    };
    pony_index_result result =
        pony_index_put(self, cstr_from(record.key), entry);
    assert(result.inserted);
  }
}
