#ifndef SIMULATION_H
#define SIMULATION_H

#include "game_rules.h"

typedef struct {
    int L;
    int C;
    int G;
    int R;
    Rule *rules;
    int *rule_matrix;
    unsigned char *initial_grid;
} SimulationConfig;

void free_config(SimulationConfig *config);

#endif
