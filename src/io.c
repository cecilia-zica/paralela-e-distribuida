#include <limits.h>
#include <stdint.h>
#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include "io.h"

static double elapsed_seconds(const struct timespec *start,
                              const struct timespec *end)
{
  return (double)(end->tv_sec - start->tv_sec) +
         (double)(end->tv_nsec - start->tv_nsec) / 1000000000.0;
}

/* getchar_unlocked usa o buffer interno de stdin, mas evita o custo de scanf
 * para cada inteiro da matriz. O separador fica pendente para que, ao terminar
 * a matriz, possamos consumir somente o restante da linha antes de ler a grade. */
static int pending_input_char;
static int has_pending_input_char;

static int fast_getchar(void)
{
  if (has_pending_input_char)
  {
    has_pending_input_char = 0;
    return pending_input_char;
  }
  return getchar_unlocked();
}

/* Retorna 1 ao ler um int, 0 para entrada malformada/EOF e -1 em overflow. */
static int fast_read_int(int *value)
{
  int ch;
  do
  {
    ch = fast_getchar();
  } while (ch != EOF && isspace((unsigned char)ch));

  int negative = 0;
  if (ch == '+' || ch == '-')
  {
    negative = (ch == '-');
    ch = fast_getchar();
  }
  if (ch < '0' || ch > '9')
    return 0;

  unsigned int limit = negative ? (unsigned int)INT_MAX + 1U : (unsigned int)INT_MAX;
  unsigned int number = 0;
  do
  {
    unsigned int digit = (unsigned int)(ch - '0');
    if (number > (limit - digit) / 10U)
      return -1;
    number = number * 10U + digit;
    ch = fast_getchar();
  } while (ch >= '0' && ch <= '9');

  if (ch != EOF)
  {
    if (!isspace((unsigned char)ch))
      return 0;
    pending_input_char = ch;
    has_pending_input_char = 1;
  }

  if (negative)
    *value = (number == (unsigned int)INT_MAX + 1U) ? INT_MIN : -(int)number;
  else
    *value = (int)number;
  return 1;
}

int read_input_profiled(SimulationConfig *config, InputTiming *timing)
{
  struct timespec total_start, total_end, allocation_start, allocation_end;
  double allocation_seconds = 0.0;
  clock_gettime(CLOCK_MONOTONIC, &total_start);
  char *line = NULL;
  has_pending_input_char = 0;
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

  clock_gettime(CLOCK_MONOTONIC, &allocation_start);
  config->rules = malloc((size_t)config->R * sizeof(*config->rules));
  config->rule_matrix = malloc(total_cells * sizeof(*config->rule_matrix));
  config->initial_grid = malloc(total_cells * sizeof(*config->initial_grid));
  line = calloc((size_t)config->C + 3, 1);
  clock_gettime(CLOCK_MONOTONIC, &allocation_end);
  allocation_seconds += elapsed_seconds(&allocation_start, &allocation_end);
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
    if (fast_read_int(&config->rule_matrix[pos]) != 1)
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
  while ((ch = fast_getchar()) != '\n' && ch != EOF)
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
  clock_gettime(CLOCK_MONOTONIC, &total_end);
  if (timing != NULL)
  {
    double total_seconds = elapsed_seconds(&total_start, &total_end);
    timing->allocation_seconds = allocation_seconds;
    timing->input_seconds = total_seconds - allocation_seconds;
  }
  return 0;

fail:
  free(line);
  free_config(config);
  clock_gettime(CLOCK_MONOTONIC, &total_end);
  if (timing != NULL)
  {
    double total_seconds = elapsed_seconds(&total_start, &total_end);
    timing->allocation_seconds = allocation_seconds;
    timing->input_seconds = total_seconds - allocation_seconds;
  }
  return 1;
}

int read_input(SimulationConfig *config)
{
  return read_input_profiled(config, NULL);
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
