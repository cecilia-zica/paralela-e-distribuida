#ifndef IO_H
#define IO_H

#include "game_rules.h"

int read_input(SimulationConfig *config);
void print_grid(const unsigned char *grid, int rows, int cols);
void free_config(SimulationConfig *config);

#endif
