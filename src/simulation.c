#include <stdlib.h>
#include "game_rules.h"

void free_config(SimulationConfig *config)
{
  free(config->initial_grid);
  free(config->rule_matrix);
  free(config->rules);
  *config = (SimulationConfig){0};
}

