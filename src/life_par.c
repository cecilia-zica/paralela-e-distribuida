#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <errno.h>
#include <limits.h>
#include <stdint.h>

#define MAX_RULE_LEN 32

/* le um inteiro sem permitir overflow na conversao ou tokens incompletos */
static int read_int(int *value) {
    char token[64];
    size_t length = 0;
    int ch;
    do {
        ch = getchar();
    } while (ch != EOF && isspace((unsigned char)ch));

    if (ch == EOF) return 0;
    do {
        if (length == sizeof(token) - 1) return 0;
        token[length++] = (char)ch;
        ch = getchar();
    } while (ch != EOF && !isspace((unsigned char)ch));
    /* Preserva o separador para a leitura da grade apos a matriz de regras */
    if (ch != EOF) ungetc(ch, stdin);
    token[length] = '\0';

    char *end;
    errno = 0;
    long parsed = strtol(token, &end, 10);
    if (errno == ERANGE || end == token || *end != '\0' ||
        parsed < INT_MIN || parsed > INT_MAX) return 0;
    *value = (int)parsed;
    return 1;
}

static int checked_product(size_t a, size_t b, size_t *result) {
    if (b != 0 && a > SIZE_MAX / b) return 0;
    *result = a * b;
    return 1;
}

typedef struct {
    int birth[9];
    int survival[9];
} Rule;

/*
 * Converte uma regra no formato B3/S23, B36/S23, etc.
 * para dois vetores booleanos:
 *
 * birth[n]    = 1 se uma célula morta nasce com n vizinhos
 * survival[n] = 1 se uma célula viva sobrevive com n vizinhos
 */
void parse_rule(const char *text, Rule *rule) {
    int i;

    for (i = 0; i <= 8; i++) {
        rule->birth[i] = 0;
        rule->survival[i] = 0;
    }

    int mode = 0; /* 1 = B, 2 = S */

    for (i = 0; text[i] != '\0'; i++) {
        if (text[i] == 'B' || text[i] == 'b') {
            mode = 1;
        } else if (text[i] == 'S' || text[i] == 's') {
            mode = 2;
        } else if (text[i] >= '0' && text[i] <= '8') {
            int n = text[i] - '0';

            if (mode == 1) {
                rule->birth[n] = 1;
            } else if (mode == 2) {
                rule->survival[n] = 1;
            }
        }
    }
}


/*
 * Conta os vizinhos vivos da célula (row, col).
 *
 * A grade não é toroidal:
 * posições fora da matriz são simplesmente ignoradas.
 */
int count_neighbors(
    const unsigned char *grid,  // a grade agora guarda 0 e 1
    int rows,
    int cols,
    int row,
    int col
) {
    int count = 0;

    for (int dr = -1; dr <= 1; dr++) {
        for (int dc = -1; dc <= 1; dc++) {

            if (dr == 0 && dc == 0) {
                continue;
            }

            int nr = row + dr;
            int nc = col + dc;

            if (nr >= 0 && nr < rows &&
                nc >= 0 && nc < cols) {

                count += grid[(size_t)nr * (size_t)cols + (size_t)nc]; //soma direto, sem desvio
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
    int fim
) {
    for (int row = inicio; row < fim; row++) {

        int tem_cima  = (row > 0);
        int tem_baixo = (row < rows - 1);

        /* up/dn so saem da propria linha quando a vizinha existe, para nao
           formar ponteiro fora do bloco alocado */
        const unsigned char *me = current + (size_t)row * (size_t)cols;
        const unsigned char *up = tem_cima  ? me - cols : me;
        const unsigned char *dn = tem_baixo ? me + cols : me;

        for (int col = 0; col < cols; col++) {

            size_t pos = (size_t)row * (size_t)cols + (size_t)col;
            int neighbors;

            if (tem_cima && tem_baixo && col > 0 && col < cols - 1) {
                /* MIOLO: os 8 vizinhos existem, soma direta sem checar nada */
                neighbors = up[col-1] + up[col] + up[col+1]
                          + me[col-1]           + me[col+1]
                          + dn[col-1] + dn[col] + dn[col+1];
            } else {
                /* BORDA: caminho lento, com checagem de limites */
                neighbors = count_neighbors(current, rows, cols, row, col);
            }

            const Rule *regra = &rules[rule_matrix[pos]];

            if (me[col]) {
                next[pos] = regra->survival[neighbors];
            } else {
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
    int cols
) {
    update_rows(current, next, rule_matrix, rules, rows, cols, 0, rows);
}


int main(void) {

    int L, C, G;

    if (!read_int(&L) || !read_int(&C) || !read_int(&G)) {
        fprintf(stderr, "Erro ao ler L, C e G.\n");
        return 1;
    }

    int R;

    if (!read_int(&R)) {
        fprintf(stderr, "Erro ao ler o numero de regras.\n");
        return 1;
    }

    if (L <= 0 || C <= 0 || C > INT_MAX - 3 || G < 0 || R <= 0) {
        fprintf(stderr, "Dimensões, gerações ou número de regras inválidos.\n");
        return 1;
    }

    size_t total_cells, matrix_bytes, rules_bytes, grid_bytes;
    if (!checked_product((size_t)L, (size_t)C, &total_cells) ||
        !checked_product(total_cells, sizeof(int), &matrix_bytes) ||
        !checked_product(total_cells, sizeof(unsigned char), &grid_bytes) ||
        !checked_product((size_t)R, sizeof(Rule), &rules_bytes)) {
        fprintf(stderr, "Tamanho da entrada excede o limite de memoria representavel.\n");
        return 1;
    }

    Rule *rules = malloc(rules_bytes);

    if (rules == NULL) {
        fprintf(stderr, "Erro de alocacao de memoria.\n");
        return 1;
    }

    /*
     * Leitura das regras.
     *
     * Exemplo:
     * B3/S23
     * B36/S23
     */
    for (int i = 0; i < R; i++) {
        char rule_text[MAX_RULE_LEN];

        if (scanf("%31s", rule_text) != 1) {
            fprintf(stderr, "Erro ao ler regra %d.\n", i);
            free(rules);
            return 1;
        }

        parse_rule(rule_text, &rules[i]);
    }

    int *rule_matrix = malloc(matrix_bytes);

    if (rule_matrix == NULL) {
        fprintf(stderr, "Erro de alocacao de memoria.\n");
        free(rules);
        return 1;
    }

    /*
     * Matriz contendo o identificador da regra
     * associada a cada posição.
     */
    for (int row = 0; row < L; row++) {
        for (int col = 0; col < C; col++) {

            size_t pos = (size_t)row * (size_t)C + (size_t)col;

            if (!read_int(&rule_matrix[pos])) {
                fprintf(stderr, "Erro ao ler matriz de regras.\n");
                free(rule_matrix);
                free(rules);
                return 1;
            }

            if (rule_matrix[pos] < 0 ||
                rule_matrix[pos] >= R) {

                fprintf(stderr,
                        "Identificador de regra invalido: %d\n",
                        rule_matrix[pos]);

                free(rule_matrix);
                free(rules);
                return 1;
            }
        }
    }

    /*
     * Remove o '\n' que ficou depois da leitura
     * da matriz de inteiros.
     */
    int ch;

    while ((ch = getchar()) != '\n' && ch != EOF) {
    }

    unsigned char *current = malloc(grid_bytes);
    unsigned char *next = malloc(grid_bytes);

    if (current == NULL || next == NULL) {
        fprintf(stderr, "Erro de alocacao de memoria.\n");

        free(current);
        free(next);
        free(rule_matrix);
        free(rules);

        return 1;
    }

    /*
     * Como espaços são significativos, não podemos usar scanf("%s").
     *
     * É necessário usar fgets.
     */
    char *line = malloc((size_t)C + 3);

    if (line == NULL) {
        fprintf(stderr, "Erro de alocacao de memoria.\n");

        free(current);
        free(next);
        free(rule_matrix);
        free(rules);

        return 1;
    }

    for (int row = 0; row < L; row++) {

        if (fgets(line, C + 3, stdin) == NULL) {
            fprintf(stderr, "Erro ao ler a grade inicial.\n");

            free(line);
            free(current);
            free(next);
            free(rule_matrix);
            free(rules);

            return 1;
        }

        /* espacos sao celulas logo nao devem ser removidos. Aceita LF, CRLF
           e EOF apos a ultima linha completa */
        size_t length = strcspn(line, "\r\n");
        int valid_end = line[length] == '\n' ||
            (line[length] == '\r' && line[length + 1] == '\n') ||
            (line[length] == '\0' && feof(stdin) && row == L - 1);
        if (length != (size_t)C || !valid_end) {
            fprintf(stderr, "Linha %d da grade deve conter exatamente %d posicoes.\n",
                    row + 1, C);
            free(line);
            free(current);
            free(next);
            free(rule_matrix);
            free(rules);
            return 1;
        }

        for (int col = 0; col < C; col++) {

            current[(size_t)row * (size_t)C + (size_t)col] =
            (line[col] == 'x' || line[col] == 'X') ? 1 : 0;
        }
    }

    free(line);

    /*
     * Executa as G gerações.
     *
     * Ao final de cada geração simplesmente trocamos
     * os ponteiros current e next.
     *
     * Assim não é necessário copiar toda a matriz.
     */
    for (int generation = 0; generation < G; generation++) {

        next_generation(
            current,
            next,
            rule_matrix,
            rules,
            L,
            C
        );

        unsigned char *temp = current;
        current = next;
        next = temp;
    }

    /*
     * Imprime exatamente L linhas,
     * cada uma contendo C posições.
     */
    for (int row = 0; row < L; row++) {

        for (int col = 0; col < C; col++) {
            putchar(current[(size_t)row * (size_t)C + (size_t)col] ? 'x' : ' ');
        }

        putchar('\n');
    }

    free(current);
    free(next);
    free(rule_matrix);
    free(rules);

    return 0;
}
