#ifndef IO_H
#define IO_H

#include "game_rules.h"

typedef struct {
  double input_seconds;
  double allocation_seconds;
} InputTiming;

int read_input(SimulationConfig *config);
int read_input_profiled(SimulationConfig *config, InputTiming *timing);
void print_grid(const unsigned char *grid, int rows, int cols);
void free_config(SimulationConfig *config);

#endif
