#define _POSIX_C_SOURCE 200809L
#include <errno.h>
#include <pthread.h>
#include <stdatomic.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include "io.h"

typedef struct {
  const unsigned char *current;
  unsigned char *next;
  const int *rule_matrix;
  const Rule *rules;
  int rows, cols, generations, chunk_rows;
  atomic_size_t *next_row;
  pthread_barrier_t *finished;
  pthread_barrier_t *ready;
} Worker;

static double elapsed_seconds(const struct timespec *a, const struct timespec *b)
{
  return (double)(b->tv_sec - a->tv_sec) + (double)(b->tv_nsec - a->tv_nsec) / 1e9;
}

static void *worker_main(void *arg)
{
  Worker *w = arg;
  for (int generation = 0; generation < w->generations; ++generation) {
    for (;;) {
      size_t begin = atomic_fetch_add_explicit(w->next_row,
          (size_t)w->chunk_rows, memory_order_relaxed);
      if (begin >= (size_t)w->rows) break;
      size_t remaining = (size_t)w->rows - begin;
      int end = (int)(begin + ((size_t)w->chunk_rows < remaining
                                  ? (size_t)w->chunk_rows : remaining));
      update_rows(w->current, w->next, w->rule_matrix, w->rules,
                  w->rows, w->cols, (int)begin, end);
    }

    /* One worker resets the queue after all writes to this generation finish.
       The second barrier publishes the reset before anyone claims next-gen rows. */
    int rc = pthread_barrier_wait(w->finished);
    if (rc == PTHREAD_BARRIER_SERIAL_THREAD)
      atomic_store_explicit(w->next_row, 0, memory_order_relaxed);
    pthread_barrier_wait(w->ready);

    const unsigned char *tmp = w->current;
    w->current = w->next;
    w->next = (unsigned char *)tmp;
  }
  return NULL;
}

static int parse_positive(const char *text, int *value)
{
  char *end = NULL;
  errno = 0;
  long v = strtol(text, &end, 10);
  if (errno || end == text || *end || v < 1 || v > 2147483647L) return 0;
  *value = (int)v;
  return 1;
}

int main(int argc, char **argv)
{
  if (argc < 2 || argc > 3) {
    fprintf(stderr, "Uso: %s <threads> [linhas_por_bloco] < entrada.in\n", argv[0]);
    return 1;
  }
  int nthreads, chunk = 4;
  if (!parse_positive(argv[1], &nthreads) ||
      (argc == 3 && !parse_positive(argv[2], &chunk))) {
    fprintf(stderr, "Threads e linhas_por_bloco devem ser inteiros positivos.\n");
    return 1;
  }

  SimulationConfig config = {0};
  if (read_input(&config)) return 1;
  if (nthreads > config.L) nthreads = config.L;

  size_t cells = (size_t)config.L * (size_t)config.C;
  unsigned char *buffer = malloc(cells);
  pthread_t *ids = calloc((size_t)nthreads, sizeof(*ids));
  Worker *workers = calloc((size_t)nthreads, sizeof(*workers));
  if (!buffer || !ids || !workers) {
    fprintf(stderr, "Erro de alocacao de memoria.\n");
    free(buffer); free(ids); free(workers); free_config(&config); return 1;
  }

  atomic_size_t next_row;
  atomic_init(&next_row, 0);
  pthread_barrier_t finished, ready;
  if (pthread_barrier_init(&finished, NULL, (unsigned)nthreads) != 0 ||
      pthread_barrier_init(&ready, NULL, (unsigned)nthreads) != 0) {
    fprintf(stderr, "Erro ao inicializar barreiras.\n");
    free(buffer); free(ids); free(workers); free_config(&config); return 1;
  }
  unsigned char *current = config.initial_grid, *next = buffer;
  for (int i = 0; i < nthreads; ++i) {
    workers[i] = (Worker){current, next, config.rule_matrix, config.rules,
      config.L, config.C, config.G, chunk, &next_row, &finished, &ready};
  }

  struct timespec start, end;
  clock_gettime(CLOCK_MONOTONIC, &start);
  int created = 0;
  for (; created < nthreads; ++created) {
    int rc = pthread_create(&ids[created], NULL, worker_main, &workers[created]);
    if (rc != 0) {
      fprintf(stderr, "pthread_create falhou: %d\n", rc);
      /* Workers may be waiting at a barrier sized for nthreads. Exiting the
         process is safer than returning and stranding them. */
      exit(EXIT_FAILURE);
    }
  }
  for (int i = 0; i < nthreads; ++i) pthread_join(ids[i], NULL);
  clock_gettime(CLOCK_MONOTONIC, &end);

  print_grid((config.G % 2 == 0) ? config.initial_grid : buffer, config.L, config.C);
  if (getenv("LIFE_TIMING"))
    fprintf(stderr, "TIMING rows=%d cols=%d generations=%d threads=%d chunk=%d evolution_s=%.9f\n",
            config.L, config.C, config.G, nthreads, chunk, elapsed_seconds(&start, &end));

  pthread_barrier_destroy(&ready); pthread_barrier_destroy(&finished);
  free(buffer); free(ids); free(workers); free_config(&config);
  return 0;
}
