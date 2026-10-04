#ifndef IO_H
#define IO_H

#include "simulation.h"

int read_input(SimulationConfig *config);
void print_grid(const unsigned char *grid, int rows, int cols);

#endif
