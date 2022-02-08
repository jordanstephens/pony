#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include "../include/minunit.h"
#include "../src/pony.h"

int tests_run = 0;

static char * test_open_close() {
  const char* dirname = "./tmp/test_open";
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

static char * test_put_get() {
  const char* dirname = "./tmp/test_put_get";
  pony_db db = pony_open(dirname);
  const char* key = "foo";
  const char* value = "bar";
  pony_put(&db, key, value);
  const char* returned_value = pony_get(&db, key);
  mu_assert(
    "get value is not the same as put value",
    strcmp(value, returned_value) == 0
  );
  pony_close(&db);
  return 0;
}

static char * test_get_missing() {
  const char* dirname = "./tmp/test_get_missing";
  pony_db db = pony_open(dirname);
  const char* key = "foo";
  const char* returned_value = pony_get(&db, key);
  mu_assert(
    "value for non-inserted key is not NULL",
    returned_value == NULL
  );
  pony_close(&db);
  return 0;
}

static char * test_put_rm_get() {
  const char* dirname = "./tmp/test_put_rm_get";
  pony_db db = pony_open(dirname);
  const char* key = "foo";
  const char* value = "bar";
  pony_put(&db, key, value);
  pony_rm(&db, key);
  const char* returned_value = pony_get(&db, key);
  mu_assert(
    "value for removed key is not NULL",
    returned_value == NULL
  );
  pony_close(&db);
  return 0;
}

static char * test_put_close_get() {
  const char* dirname = "./tmp/test_put_close_get";
  pony_db db = pony_open(dirname);
  const char* key = "foo";
  const char* value = "bar";
  pony_put(&db, key, value);
  pony_close(&db);
  db = pony_open(dirname);
  const char* returned_value = pony_get(&db, key);
  mu_assert(
    "value is not persisted after close",
    strcmp(value, returned_value) == 0
  );
  pony_close(&db);
  return 0;
}

static char * test_put_rm_close_get() {
  const char* dirname = "./tmp/test_put_rm_close_get";
  pony_db db = pony_open(dirname);
  const char* key = "foo";
  const char* value = "bar";
  pony_put(&db, key, value);
  pony_rm(&db, key);
  pony_close(&db);
  db = pony_open(dirname);
  const char* returned_value = pony_get(&db, key);
  mu_assert(
    "value for removed key is not NULL",
    returned_value == NULL
  );
  pony_close(&db);
  return 0;
}

static char * all_tests() {
  mu_run_test(test_open_close);
  mu_run_test(test_put_get);
  mu_run_test(test_get_missing);
  mu_run_test(test_put_rm_get);
  mu_run_test(test_put_close_get);
  mu_run_test(test_put_rm_close_get);
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
