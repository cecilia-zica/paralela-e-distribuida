#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include "io.h"

// - - - Data Structures
typedef struct {
    const unsigned char *current;
    unsigned char *next;
    const int *rule_matrix;
    const Rule *rules;
    int rows;
    int cols;
    int inicio;
    int fim;
} Thread;

// - - - Functions Declarations
void *thread_work(void *arg) {
    Thread *thread_data = (Thread *)arg;
    update_rows(thread_data->current, thread_data->next, thread_data->rule_matrix,
                thread_data->rules, thread_data->rows, thread_data->cols,
                thread_data->inicio, thread_data->fim);
    return NULL; 
}; 

int main(int argc, char *argv[]) {

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
  if (read_input(&config) != 0)
    return 1;

    
  // cast to size_t before multiplying, so the multiplication itself, can't overflow int range
  size_t total_cells = (size_t)config.L * config.C;

  // sizeof(*buffer) adapts automatically if buffer's type ever changes
  unsigned char *buffer = malloc(total_cells * sizeof(*buffer));
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

  // Divide the rows among the threads (atributes that doesn't change during the generations)
  int rows_per_thread = config.L / n_threads;
  int remaining_rows = config.L % n_threads;
  int cursor = 0;

  for (int i = 0; i < n_threads; i++) {
      int current_line = rows_per_thread;
      if (i >= n_threads - remaining_rows) {
          current_line = rows_per_thread + 1;
      }

      threads[i].rule_matrix = config.rule_matrix;
      threads[i].rules = config.rules;
      threads[i].rows = config.L;
      threads[i].cols = config.C;
      threads[i].inicio = cursor;
      threads[i].fim = cursor + current_line;
      cursor = threads[i].fim;
  }

  for (int generation = 0; generation < config.G; generation++) {

    for (int i = 0; i < n_threads; i++) {
        threads[i].current = current;   // current/next changes each generation 
        threads[i].next = next;
        pthread_create(&thread_ids[i], NULL, thread_work, &threads[i]);
    }

    for (int i = 0; i < n_threads; i++) {
        pthread_join(thread_ids[i], NULL);
    }

    unsigned char *temp = current;
    current = next;
    next = temp;
  }

  print_grid(current, config.L, config.C);

  free(buffer);
  free_config(&config);
  return 0;
}
