
#include "pony_index.h"

#include <assert.h>

#include "pony_cask.h"
#include "pony_cask_set.h"
#include "pony_record.h"

void pony_index_load_entry(pony_index* self, const char* key, pony_index_entry entry) {
  pony_index_put(self, cstr_from(key), entry);
}

void pony_index_load_cask(pony_index* self, pony_cask_set* readers) {
  c_foreach (it, pony_cask_set, *readers) {
    const pony_cask_set_value* cask_set_entry = it.ref;
    pony_cask reader = cask_set_entry->second;
    assert(reader.offset == 0);
    pony_cask_buf_reader buf_reader = pony_cask_buf_reader_new(&reader);
    while (true) {
      pony_record record;
      ssize_t bytes_read = pony_cask_buf_reader_next(&record, &buf_reader);
      if (bytes_read == 0)
        break;

      if (!pony_record_is_tombstone(&record)) {
        pony_index_load_entry(self, record.key, (pony_index_entry){
          .offset = reader.offset - bytes_read,
          .size = bytes_read,
          .generation = reader.generation,
        });
        continue;
      }

      pony_index_erase(self, record.key);
    }
  }
}
