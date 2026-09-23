# Plano de Divisão de Tarefas — T1 (INE 5645) — Escopo 2 (Pthreads completo)

Sugestão de atualização do `plano-divisao.md` original

## 1. O que já está pronto

O arquivo enviado (atualmente monolítico, tudo em um único `.c`) já implementa:

- [x] `parse_rule` — conversão de `B.../S...` para vetores booleanos de birth/survival
- [x] Leitura de `L C G`, do número de regras `R` e das `R` regras
- [x] Leitura da matriz `L×C` de identificadores de regra, com validação de índice
- [x] Leitura do estado inicial via `fgets` (preserva espaços em branco corretamente)
- [x] `count_neighbors` — até 8 vizinhos, não toroidal, bordas tratadas por limite de índice
- [x] `next_generation` — evolução síncrona (lê só de `current`, escreve só em `next`,
      troca de ponteiros entre gerações — já evita cópia desnecessária da grade)
- [x] Impressão da grade final no formato exigido

**Pendente antes de considerar isso "fechado":**
- [ ] Validar a saída contra os dois exemplos do enunciado e os 4 arquivos `life-*.in`/`.out`
      fornecidos (ainda não foi conferido byte-a-byte)
- [ ] Separar o arquivo único na estrutura do projeto (`src/grid.c`, `src/io.c`, ver seção 2)

## 2. Ajuste na estrutura de arquivos

O que hoje é um arquivo só precisa virar (mantendo o mapeamento do plano original):

```
src/grid.c   ← struct Rule, parse_rule, alocação da matriz de regras/grade
src/io.c     ← leitura de L C G / R / regras / matriz / estado inicial, impressão final
include/*.h  ← protótipos compartilhados (Rule, parse_rule, count_neighbors, next_generation)
```

`count_neighbors` e `next_generation` continuam existindo como referência sequencial —
é a partir delas que o Escopo 2 é construído, não algo que é jogado fora.

## 3. Divisão por sprint

### Sprint 1 — Corretude

| Membro | Responsabilidade |
|---|---|
| **P1** | Separar o código existente em `grid.c`/`io.c`; validar contra os 2 exemplos do PDF e os 4 `life-*.in`/`.out` |
| **P2** | Escrever `parallel_static.c`: portar `count_neighbors`/`next_generation` pra versão com pthreads — particionamento fixo de linhas, `pthread_create`, `pthread_barrier` entre gerações |
| **P3** | Gerador de instâncias sintéticas (incluindo desbalanceadas, pra usar já no Sprint 2), script de comparação automática com os `.out`, esqueleto de `timing.c` |

**Meta do sprint (DoD):** igual ao plano original — compila sem warnings, bate com os 4
testes, sem erro de memória (rodar `valgrind` na versão sequencial já ajuda a pegar
qualquer problema de alocação antes de threads entrarem em cena).

### Sprint 2 — Desempenho

| Membro | Responsabilidade |
|---|---|
| **P2** | Lidera a extensão pra fila de blocos (`parallel_dynamic.c`) |
| **P1** | Pareia com P2 na fila dinâmica; testa granularidade de bloco |
| **P3** | Benchmarks estático vs. dinâmico vs. `life.c`, em instâncias uniformes e desbalanceadas |

### Sprint 3 — Entrega

P2 escreve a seção de estratégia/decisões de
implementação, P1 a de estrutura de dados/E-S + Makefile/README, P3 a de resultados
experimentais e discussão. Ensaio de defesa com todos explicando a versão final inteira.
