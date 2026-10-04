#ifndef GRID_H
#define GRID_H

#include "game_rules.h"

void update_rows(
    const unsigned char *current,
    unsigned char *next,
    const int *rule_matrix,
    const Rule *rules,
    int rows,
    int cols,
    int inicio,
    int fim);

void next_generation(
    const unsigned char *current,
    unsigned char *next,
    const int *rule_matrix,
    const Rule *rules,
    int rows,
    int cols);

#endif
