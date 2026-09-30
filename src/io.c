#include <limits.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include "io.h"

void free_config(SimulationConfig *config)
{
  free(config->initial_grid);
  free(config->rule_matrix);
  free(config->rules);
  *config = (SimulationConfig){0};
}

int read_input(SimulationConfig *config)
{
  char *line = NULL;
  *config = (SimulationConfig){0};

  if (scanf("%d %d %d", &config->L, &config->C, &config->G) != 3)
  {
    fprintf(stderr, "Erro ao ler L, C e G.\n");
    goto fail;
  }

  if (config->L <= 0 || config->C <= 0 || config->G < 0 ||
      config->C > INT_MAX - 3 || config->L > INT_MAX / config->C)
  {
    fprintf(stderr, "Dimensoes ou numero de geracoes invalidos.\n");
    goto fail;
  }

  if (scanf("%d", &config->R) != 1 || config->R <= 0)
  {
    fprintf(stderr, "Numero de regras invalido.\n");
    goto fail;
  }

  size_t total_cells = (size_t)config->L * config->C;
  if ((size_t)config->R > SIZE_MAX / sizeof(Rule) ||
      total_cells > SIZE_MAX / sizeof(int))
  {
    fprintf(stderr, "Tamanho da entrada excede o limite suportado.\n");
    goto fail;
  }

  config->rules = malloc((size_t)config->R * sizeof(*config->rules));
  config->rule_matrix = malloc(total_cells * sizeof(*config->rule_matrix));
  config->initial_grid = malloc(total_cells * sizeof(*config->initial_grid));
  line = calloc((size_t)config->C + 3, 1);
  if (!config->rules || !config->rule_matrix || !config->initial_grid || !line)
  {
    fprintf(stderr, "Erro de alocacao de memoria.\n");
    goto fail;
  }

  for (int i = 0; i < config->R; i++)
  {
    char rule_text[MAX_RULE_LEN];
    if (scanf("%31s", rule_text) != 1)
    {
      fprintf(stderr, "Erro ao ler regra %d.\n", i);
      goto fail;
    }
    parse_rule(rule_text, &config->rules[i]);
  }

  for (size_t pos = 0; pos < total_cells; pos++)
  {
    if (scanf("%d", &config->rule_matrix[pos]) != 1)
    {
      fprintf(stderr, "Erro ao ler matriz de regras.\n");
      goto fail;
    }
    if (config->rule_matrix[pos] < 0 || config->rule_matrix[pos] >= config->R)
    {
      fprintf(stderr, "Identificador de regra invalido: %d\n",
              config->rule_matrix[pos]);
      goto fail;
    }
  }

  int ch;
  while ((ch = getchar()) != '\n' && ch != EOF)
  {
  }

  for (int row = 0; row < config->L; row++)
  {
    if (fgets(line, config->C + 3, stdin) == NULL)
    {
      fprintf(stderr, "Erro ao ler a grade inicial.\n");
      goto fail;
    }
    for (int col = 0; col < config->C; col++)
      config->initial_grid[row * config->C + col] =
          (line[col] == 'x' || line[col] == 'X');
  }

  free(line);
  return 0;

fail:
  free(line);
  free_config(config);
  return 1;
}

void print_grid(const unsigned char *grid, int rows, int cols)
{
  for (int row = 0; row < rows; row++)
  {
    for (int col = 0; col < cols; col++)
      putchar(grid[row * cols + col] ? 'x' : ' ');
    putchar('\n');
  }
}
