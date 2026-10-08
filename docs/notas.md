## Notas

# Otimizacao do sequencial (antes de paralelizar)

mexi no life_par.c pra deixar o sequencial mais rapido antes de botar thread.
faz sentido porque o speedup e tempo(life.c do prof) / tempo(nosso), entao o que
a gente ganha aqui ja conta.

duas mudancas:

1. **miolo x borda** — o count_neighbors original testava se cada um dos 8 vizinhos
   esta dentro da grade, pra toda celula. mas numa grade grande quase toda celula e
   do miolo e esse teste nunca falha. agora cada linha guarda 3 ponteiros (up/me/dn)
   e o miolo soma os 8 vizinhos direto. so a borda cai no count_neighbors antigo.

2. **grade em 0/1 em vez de 'x' e ' '** — assim contar vizinho virou soma em vez de
   8 comparacoes, e como as tabelas birth/survival ja sao 0/1 o valor da tabela ja e o
   novo estado (matou um if por celula). converte pra caractere so na leitura e na
   impressao, formato de entrada/saida continua igual.

as outras edicoes (declaracao das grades e o temp da troca de ponteiros) foram so
consequencia da mudanca de tipo pra unsigned char.

obs: up e dn so apontam pra outra linha quando ela existe (ternario com
tem_cima/tem_baixo). formar ponteiro antes do inicio do bloco alocado e
comportamento indefinido em C mesmo sem ler.

## Preparacao para threads

O calculo foi extraido para `update_rows`, que recebe `inicio` (inclusivo) e
`fim` (exclusivo). `next_generation` continua como referencia sequencial e chama
`update_rows(..., 0, rows)`. A troca dos buffers continua fora dessas funcoes.

As dimensoes `rows` e `cols` sao sempre as da grade inteira. Uma faixa pode ler
vizinhos de outra faixa em `current`, mas escreve apenas suas linhas em `next`.
O limite de uma faixa nao e uma borda da grade.

Para verificar: `make test`. O script compara
os quatro arquivos fornecidos byte a byte e mais 24 casos com um oraculo simples
em Python: 1x1, linha unica, coluna unica, grades pequenas, zero e multiplas
geracoes, regras distintas por posicao e nascimento com zero vizinhos.

## Separacao em modulos

O codigo foi dividido por responsabilidade: `game_rules` interpreta as regras,
`grid` calcula a evolucao, `simulation` define a configuracao e libera seus recursos,
e `io` cuida da leitura e impressao. `life_par.c` ficou com o `main`, o loop das
geracoes e a troca dos buffers. `count_neighbors` ficou interna a `grid.c`;
`update_rows` continua publica para as futuras threads.

## Threads criadas uma vez + barreira

a primeira versao criava e destruia as threads a cada geracao. com G grande isso
nao paga: em 200x200 com G=3000 a paralela ficava mais lenta que a sequencial.
agora as threads sao criadas uma vez e cada uma roda as G
geracoes, com pthread_barrier_wait no fim de cada uma garantindo a evolucao
sincrona.

uma barreira basta porque cada thread troca os proprios ponteiros depois dela —
nao tem ponteiro compartilhado sendo alternado. como as threads alternam G vezes,
o main refaz a conta: G par termina em current, G impar em next.

## Teste no SDumont

rodei a versao atual no Santos Dumont, fila sequana_cpu_dev, 1 no com 48 nucleos,
gcc 13.2 com -O3. instancia 2000x2000 com G=100

| threads  | tempo  | speedup vs life.c | eficiencia |
| -------- | ------ | ----------------- | ---------- |
| life.c   | 4,251s | 1,00              | -          |
| life_par | 1,951s | 2,18              | -          |
| 1        | 1,871s | 2,27              | 100%       |
| 2        | 1,096s | 3,88              | 85%        |
| 4        | 0,731s | 5,82              | 64%        |
| 8        | 0,523s | 8,13              | 45%        |
| 16       | 0,413s | 10,29             | 28%        |
| 24       | 0,384s | 11,07             | 20%        |
| 48       | 0,367s | 11,58             | 11%        |

melhor resultado: 11,58x com 48 threads. mas de 24 pra 48 so ganhamos 4% e a
eficiencia cai pra 11%, entao tem algo segurando (provavelmente a leitura, impresaao ou algo sequencial)

## Duvidas
