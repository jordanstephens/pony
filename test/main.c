#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include "../include/minunit.h"
#include "../src/pony.h"

int tests_run = 0;

static char * test_open_close() {
  char* dirname = "./tmp/test_open";
  pony_db db = pony_open(dirname);
  mu_assert(
    "db.path is not the same as dirname",
    strcmp(db.path, dirname) == 0
  );
  mu_assert(
    "db.path is not a copy of dirname",
    db.path != dirname
  );
  pony_close(&db);
  return 0;
}

static char * all_tests() {
  mu_run_test(test_open_close);
  return 0;
}

int main() {
  char *result = all_tests();
  printf("\n");
  if (result != 0) {
    printf("%s\n", result);
  } else {
    printf("ALL TESTS PASSED\n");
  }
  printf("Tests run: %d\n", tests_run);

  return result != 0;
}
