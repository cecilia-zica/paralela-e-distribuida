#include "game_rules.h"

/*
 * Converte uma regra no formato B3/S23, B36/S23, etc.
 * para dois vetores booleanos:
 *
 * birth[n]    = 1 se uma célula morta nasce com n vizinhos
 * survival[n] = 1 se uma célula viva sobrevive com n vizinhos
 */
void parse_rule(const char *text, Rule *rule)
{
  int i;

  for (i = 0; i <= 8; i++)
  {
    rule->birth[i] = 0;
    rule->survival[i] = 0;
  }

  int mode = 0; /* 1 = B, 2 = S */

  for (i = 0; text[i] != '\0'; i++)
  {
    if (text[i] == 'B' || text[i] == 'b')
    {
      mode = 1;
    }
    else if (text[i] == 'S' || text[i] == 's')
    {
      mode = 2;
    }
    else if (text[i] >= '0' && text[i] <= '8')
    {
      int n = text[i] - '0';

      if (mode == 1)
      {
        rule->birth[n] = 1;
      }
      else if (mode == 2)
      {
        rule->survival[n] = 1;
      }
    }
  }
}

