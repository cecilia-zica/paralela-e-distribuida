#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <time.h>
#include "io.h"

// - - - Data Structures
typedef struct {
    unsigned char *current;   // sem const: as threads alternam os dois buffers
    unsigned char *next;
    const int *rule_matrix;
    const Rule *rules;
    int rows;
    int cols;
    int inicio;
    int fim;
    int generations;
    pthread_barrier_t *barreira;
} Thread;

static double elapsed_seconds(const struct timespec *start,
                              const struct timespec *end) {
    return (double)(end->tv_sec - start->tv_sec) +
           (double)(end->tv_nsec - start->tv_nsec) / 1000000000.0;
}

// - - - Functions Declarations
void *thread_work(void *arg) {
    Thread *thread_data = (Thread *)arg;

    // copias locais: cada thread alterna os seus proprios ponteiros
    unsigned char *current = thread_data->current;
    unsigned char *next = thread_data->next;

    for (int generation = 0; generation < thread_data->generations; generation++) {
        update_rows(current, next, thread_data->rule_matrix,
                    thread_data->rules, thread_data->rows, thread_data->cols,
                    thread_data->inicio, thread_data->fim);

        // evolucao sincrona: ninguem comeca t+1 antes de todas terminarem t
        pthread_barrier_wait(thread_data->barreira);

        unsigned char *temp = current;
        current = next;
        next = temp;
    }

    return NULL; 
};

int main(int argc, char *argv[]) {
  struct timespec total_start, allocation_start, allocation_end;
  struct timespec evolution_start, evolution_end, print_start, print_end, total_end;
  InputTiming input_timing = {0};
  int profiling = (getenv("LIFE_TIMING") != NULL);
  clock_gettime(CLOCK_MONOTONIC, &total_start);

  if (argc < 2) {
    fprintf(stderr, "Uso: %s <numero_de_threads> < entrada.in\n", argv[0]);
    return 1;
  }

  int n_threads = atoi(argv[1]);
  if (n_threads <= 0) {
    fprintf(stderr, "numero_de_threads precisa ser um inteiro positivo.\n");
    return 1;
  }

  SimulationConfig config = {0};
  if (read_input_profiled(&config, profiling ? &input_timing : NULL) != 0)
    return 1;

    
  // cast to size_t before multiplying, so the multiplication itself, can't overflow int range
  size_t total_cells = (size_t)config.L * config.C;

  // sizeof(*buffer) adapts automatically if buffer's type ever changes
  clock_gettime(CLOCK_MONOTONIC, &allocation_start);
  unsigned char *buffer = malloc(total_cells * sizeof(*buffer));
  clock_gettime(CLOCK_MONOTONIC, &allocation_end);
  double allocation_seconds = input_timing.allocation_seconds +
                              elapsed_seconds(&allocation_start, &allocation_end);
  if (buffer == NULL) {
    fprintf(stderr, "Erro de alocacao de memoria.\n");
    free_config(&config); // release what read_input already allocated
    return 1;
  }

  unsigned char *current = config.initial_grid;
  unsigned char *next = buffer;

  // One array for struct (threads data) and one array for pthread_t (ids)
  Thread threads[n_threads];
  pthread_t thread_ids[n_threads];

  pthread_barrier_t barreira;
  if (pthread_barrier_init(&barreira, NULL, n_threads) != 0) {
    fprintf(stderr, "Erro ao inicializar a barreira.\n");
    free(buffer);
    free_config(&config);
    return 1;
  }

  // Divide the rows among the threads (atributes that doesn't change during the generations)
  int rows_per_thread = config.L / n_threads;
  int remaining_rows = config.L % n_threads;
  int cursor = 0;

  for (int i = 0; i < n_threads; i++) {
      int current_line = rows_per_thread;
      if (i >= n_threads - remaining_rows) {
          current_line = rows_per_thread + 1;
      }
      threads[i].current = current;
      threads[i].next = next;
      threads[i].generations = config.G;
      threads[i].barreira = &barreira;
      threads[i].rule_matrix = config.rule_matrix;
      threads[i].rules = config.rules;
      threads[i].rows = config.L;
      threads[i].cols = config.C;
      threads[i].inicio = cursor;
      threads[i].fim = cursor + current_line;
      cursor = threads[i].fim;
  }

    // threads criadas uma vez, fora do laco de geracoes
  clock_gettime(CLOCK_MONOTONIC, &evolution_start);
  for (int i = 0; i < n_threads; i++)
      pthread_create(&thread_ids[i], NULL, thread_work, &threads[i]);

  for (int i = 0; i < n_threads; i++)
      pthread_join(thread_ids[i], NULL);
  clock_gettime(CLOCK_MONOTONIC, &evolution_end);

  // cada thread alternou os buffers G vezes: G par termina em current, impar em next
  clock_gettime(CLOCK_MONOTONIC, &print_start);
  print_grid((config.G % 2 == 0) ? current : next, config.L, config.C);
  clock_gettime(CLOCK_MONOTONIC, &print_end);
  clock_gettime(CLOCK_MONOTONIC, &total_end);

  if (profiling) {
    fprintf(stderr,
            "TIMING rows=%d cols=%d generations=%d threads=%d "
            "input_s=%.9f allocation_s=%.9f evolution_s=%.9f "
            "print_s=%.9f total_s=%.9f\n",
            config.L, config.C, config.G, n_threads,
            input_timing.input_seconds, allocation_seconds,
            elapsed_seconds(&evolution_start, &evolution_end),
            elapsed_seconds(&print_start, &print_end),
            elapsed_seconds(&total_start, &total_end));
  }

  pthread_barrier_destroy(&barreira);

  free(buffer);
  free_config(&config);
  return 0;
}
