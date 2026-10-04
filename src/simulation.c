#include <stdlib.h>
#include "simulation.h"

void free_config(SimulationConfig *config)
{
  free(config->initial_grid);
  free(config->rule_matrix);
  free(config->rules);
  *config = (SimulationConfig){0};
}

