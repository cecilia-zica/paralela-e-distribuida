#include <stdio.h>
#include <stdlib.h>
#include "io.h"

int main(void)
{
  SimulationConfig config = {0};
  if (read_input(&config) != 0)
    return 1;

  size_t total_cells = (size_t)config.L * config.C;
  unsigned char *buffer = malloc(total_cells * sizeof(*buffer));
  if (buffer == NULL)
  {
    fprintf(stderr, "Erro de alocacao de memoria.\n");
    free_config(&config);
    return 1;
  }

  unsigned char *current = config.initial_grid;
  unsigned char *next = buffer;

  for (int generation = 0; generation < config.G; generation++)
  {
    next_generation(current, next, config.rule_matrix, config.rules,
                    config.L, config.C);

    unsigned char *temp = current;
    current = next;
    next = temp;
  }

  print_grid(current, config.L, config.C);

  free(buffer);
  free_config(&config);
  return 0;
}
