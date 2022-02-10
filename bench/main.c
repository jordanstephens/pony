#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <errno.h>
#include <unistd.h>
#include <fcntl.h>

#include <sys/resource.h>

#define eprintf(...) fprintf(stderr, __VA_ARGS__)

static bool enable_core_dumps(void) {
  struct rlimit limit;

  limit.rlim_cur = RLIM_INFINITY;
  limit.rlim_max = RLIM_INFINITY;
  return setrlimit(RLIMIT_CORE, &limit) == 0;
}

#include "../src/pony.h"

typedef enum {
  PUT,
  GET,
  RM
} action;

typedef struct {
  action action;
  const char* key;
  const char* value;
} task;

int task_cmp(const task* a, const task* b) {
  if (a->action != b->action) return a->action - b->action;
  int keycmp = strcmp(a->key, b->key);
  if (0 != keycmp) return keycmp;
  int valcmp = strcmp(a->value, b->value);
  if (0 != valcmp) return valcmp;
  return 0;
}

task task_clone(task src) {
  task dst = {src.action, strdup(src.key), strdup(src.value)};
  return dst;
}

void task_drop(task* self) {
  free((void*)self->key);
  free((void*)self->value);
}

#define _i_prefix bench_
#define i_val task
#define i_cmp task_cmp
#define i_from task_clone
#define i_drop task_drop
#define i_tag tasks

#include "../include/cvec.h"

typedef struct {
  size_t count;
  unsigned seed;
  u_int16_t key_size;
  u_int16_t value_size;
} options;

void generate_workload(bench_tasks* tasks, options* opts);
void run(bench_tasks* tasks, options* opts);
char* random_bytes (size_t length);

options parse_args(int argc, char** argv) {
  options opts = {
    .count = 1 << 13,
    .seed = time(NULL),
    .key_size = 256,
    .value_size = 256
  };

  char opt;
  while ((opt = getopt(argc, argv, "c:s:h")) != -1) {
    switch (opt) {
    case 'c':
      opts.count = atol(optarg);
      break;
    case 's':
      opts.seed = atol(optarg);
      break;
    case 'h':
      eprintf("Usage: %s -s SEED -c COUNT\n", argv[0]);
      exit(EXIT_FAILURE);
    }
  }
  argc -= optind;
  argv += optind;

  return opts;
}
int main(int argc, char** argv) {
  // enable_core_dumps();
  srand(time(NULL));

  if (argc == 0) exit(1);
  options opts = parse_args(argc, argv);

  eprintf("COUNT: %zu\n", opts.count);
  eprintf("SEED: %d\n", opts.seed);

  eprintf("Generating Workload...\n");
  bench_tasks tasks = bench_tasks_with_capacity(opts.count);
  generate_workload(&tasks, &opts);
  eprintf("Running Benchmark...\n");
  run(&tasks, &opts);
  eprintf("Finished...\n");

  bench_tasks_drop(&tasks);
  return 0;
}

void generate_workload(bench_tasks* tasks, options* opts) {
  for (size_t i = 0; i < opts->count; i++) {
    const char* key = random_bytes(opts->key_size);
    const char* value = random_bytes(opts->value_size);
    bench_tasks_push_back(tasks, (task){
      .action = PUT,
      .key = key,
      .value = value
    });
  }
}

void run(bench_tasks* tasks, options* opts) {
  clock_t t0, t1;
  double dt;

  pony_db db = pony_open("tmp/bench", (pony_options){});
  int fd0 = db.writer.fd;

  for (size_t i = 0; i < opts->count; i++) {
    const task* t = bench_tasks_at(tasks, i);
    t0 = clock();
    switch (t->action) {
      case PUT:
        pony_put(&db, t->key, t->value);
        break;
      case GET:
        pony_get(&db, t->key);
        break;
      case RM:
        pony_rm(&db, t->key);
        break;
      default:
        eprintf("Unknown action");
        exit(EXIT_FAILURE);
    }
    t1 = clock();
    dt = ((double) (t1 - t0)) / CLOCKS_PER_SEC;
    printf("%zu\t%f\n", i, dt);
  }

  pony_close(&db);
}

char* random_bytes (size_t length) {
  char* buffer = malloc(length);

  for (size_t i = 0; i < length; i++) {
    buffer[i] = rand() % 94 + 32;
  }

  buffer[length - 1] = '\0';

  return buffer;
}
