#include "grid.h"

/*
 * Conta os vizinhos vivos da célula (row, col).
 *
 * A grade não é toroidal:
 * posições fora da matriz são simplesmente ignoradas.
 */
static int count_neighbors(
    const unsigned char *grid, // a grade agora guarda 0 e 1
    int rows,
    int cols,
    int row,
    int col)
{
  int count = 0;

  for (int dr = -1; dr <= 1; dr++)
  {
    for (int dc = -1; dc <= 1; dc++)
    {

      if (dr == 0 && dc == 0)
      {
        continue;
      }

      int nr = row + dr;
      int nc = col + dc;

      if (nr >= 0 && nr < rows &&
          nc >= 0 && nc < cols)
      {

        count += grid[nr * cols + nc]; // soma direto, sem desvio
      }
    }
  }

  return count;
}

/*
 * Calcula as linhas do intervalo [inicio, fim).
 * Requer 0 <= inicio <= fim <= rows. rows/cols são as dimensões globais.
 * Vizinhos fora da faixa ainda são lidos de current, se pertencem à grade.
 *
 * current nunca é alterada durante o cálculo.
 * Todos os novos estados são escritos em next.
 *
 * Isso garante a evolução síncrona.
 */
void update_rows(
    const unsigned char *current,
    unsigned char *next,
    const int *rule_matrix,
    const Rule *rules,
    int rows,
    int cols,
    int inicio,
    int fim)
{
  for (int row = inicio; row < fim; row++)
  {

    int tem_cima = (row > 0);
    int tem_baixo = (row < rows - 1);

    /* up/dn so saem da propria linha quando a vizinha existe, para nao
       formar ponteiro fora do bloco alocado */
    const unsigned char *me = current + row * cols;
    const unsigned char *up = tem_cima ? me - cols : me;
    const unsigned char *dn = tem_baixo ? me + cols : me;

    for (int col = 0; col < cols; col++)
    {

      int pos = row * cols + col;
      int neighbors;

      if (tem_cima && tem_baixo && col > 0 && col < cols - 1)
      {
        /* MIOLO: os 8 vizinhos existem, soma direta sem checar nada */
        neighbors = up[col - 1] + up[col] + up[col + 1] + me[col - 1] + me[col + 1] + dn[col - 1] + dn[col] + dn[col + 1];
      }
      else
      {
        /* BORDA: caminho lento, com checagem de limites */
        neighbors = count_neighbors(current, rows, cols, row, col);
      }

      const Rule *regra = &rules[rule_matrix[pos]];

      if (me[col])
      {
        next[pos] = regra->survival[neighbors];
      }
      else
      {
        next[pos] = regra->birth[neighbors];
      }
    }
  }
}

/* Referência sequencial: calcula todas as linhas antes de trocar os buffers. */
void next_generation(
    const unsigned char *current,
    unsigned char *next,
    const int *rule_matrix,
    const Rule *rules,
    int rows,
    int cols)
{
  update_rows(current, next, rule_matrix, rules, rows, cols, 0, rows);
}

