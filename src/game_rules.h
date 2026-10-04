#ifndef GAME_RULES_H
#define GAME_RULES_H

#define MAX_RULE_LEN 32

// - - - Data Structures

typedef struct {
    int birth[9];
    int survival[9];
} Rule;

typedef struct {
    int L;
    int C;
    int G;
    int R;
    Rule *rules;
    int *rule_matrix;
    unsigned char *initial_grid;
} SimulationConfig;

// - - - Functions Declarations
void parse_rule(
    const char *text,
    Rule *rule);

int count_neighbors(
    const unsigned char *grid,
    int rows,
    int cols,
    int row,
    int col);
    
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
