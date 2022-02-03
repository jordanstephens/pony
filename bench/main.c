#include <stdio.h>
#include <string.h>
#include <time.h>

#include "../src/pony.h"

unsigned char* random_bytes (size_t length);

int main() {
  srand(time(NULL));
  clock_t t0, t1;
  double dt;
  FILE* timings = fopen("./tmp/bench/timings.tsv", "a");
  pony_db db = pony_open("tmp/bench");
  size_t num_records = 1 << 20;
  size_t key_size = 64;
  size_t value_size = 64;
  for (int i = 0; i < num_records; i++) {
    const char* key = random_bytes(key_size);
    const char* value = random_bytes(value_size);
    t0 = clock();
    pony_put(&db, key, value);
    t1 = clock();
    dt = ((double) (t1 - t0)) / CLOCKS_PER_SEC;
    fprintf(timings, "%zu\t%f\n", i, dt);
  }
  return 0;
}

unsigned char* random_bytes (size_t length) {
  unsigned char* buffer = malloc(length);

  for (size_t i = 0; i < length; i++) {
    buffer[i] = rand();
  }

  return buffer;
}
