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

## Medicao por fase no SDumont

Nova rodada com instrumentacao em `parallel_static`, GCC 13.2 (`gcc/13.2_sequana`),
`-O3`, fila `sequana_cpu_dev`. Cada configuracao foi executada cinco vezes e os
valores abaixo sao medianas. Os inputs sinteticos usam uma regra B3/S23, grade
quadrada com 30% de celulas vivas e a matriz de regras preenchida com a regra 0.
A saida foi redirecionada para `/dev/null`. Os tempos totais desta rodada nao sao
diretamente comparaveis ao teste anterior, que usou outro input.

Melhor tempo total observado em cada caso (comparado ao caso de uma thread do
mesmo executavel/input):

| grade | G | melhor threads | tempo total | ganho total vs 1 thread |
|---|---:|---:|---:|---:|
| 500x500 | 10 | 16 | 0,02245 s | 1,49x |
| 500x500 | 100 | 24 | 0,03043 s | 4,04x |
| 1000x1000 | 10 | 24 | 0,08136 s | 1,56x |
| 1000x1000 | 100 | 24 | 0,10224 s | 4,77x |
| 2000x2000 | 10 | 24 | 0,31003 s | 1,61x |
| 2000x2000 | 100 | 48 | 0,35970 s | 5,39x |

No caso 2000x2000 com G=100, a mediana da evolucao caiu de 1,64343 s (1 thread)
para 0,08798 s (24) e 0,06100 s (48). O tempo total caiu de 1,93876 s para
0,38578 s e 0,35970 s, respectivamente. Assim, 48 threads aceleram bastante o
kernel, mas reduzem o total em apenas cerca de 6,8% em relacao a 24 threads.
Com 48 threads, leitura/interpretacao fica em torno de 0,237 s e impressao em
torno de 0,065 s; juntas, essas fases representam a maior parte do tempo total.
Para G=10, a simulacao e curta e 48 threads nao foram a melhor configuracao em
nenhum dos tres tamanhos.

`allocation_s` mede somente o tempo dentro das chamadas `malloc`/`calloc` e
ficou na faixa de microssegundos. A inicializacao das paginas de memoria ocorre
depois, enquanto as estruturas sao preenchidas, e esta contabilizada em
`input_s` neste programa. `evolution_s` inclui criacao e join das threads, alem
do calculo e das barreiras. Os tempos por fase sao medianas independentes, logo
nao se deve esperar que a soma delas seja exatamente a mediana de `total_s`.

A comparacao com `life.c` foi feita na secao seguinte. A varredura mostra que
priorizar somente mais threads nao resolve o limite observado no tempo total.
Com a linha de base registrada, a proxima otimizacao a investigar e a leitura
sequencial da matriz de regras (atualmente feita com milhoes de chamadas a
`scanf`); a impressao pode ser investigada separadamente.

## Speedup contra life.c nos mesmos inputs

Comparacao realizada no SDumont usando os mesmos seis inputs sinteticos da
medicao por fase. `life.c` e `parallel_static` foram compilados com GCC 13.2 e
`-O3`; cada programa/configuracao foi aquecido uma vez e medido cinco vezes,
intercalando a ordem das configuracoes. O tempo foi medido externamente com
`time.perf_counter()` em ambos, incluindo a inicializacao do processo e a E/S;
speedup = mediana(life.c) / mediana(parallel_static).

| grade | G | melhor threads | tempo life.c | tempo paralelo | speedup |
|---|---:|---:|---:|---:|---:|
| 500x500 | 10 | 16 | 0,05205 s | 0,02388 s | 2,18x |
| 500x500 | 100 | 24 | 0,25502 s | 0,03126 s | 8,16x |
| 1000x1000 | 10 | 16 | 0,19550 s | 0,08287 s | 2,36x |
| 1000x1000 | 100 | 24 | 1,00440 s | 0,10291 s | 9,76x |
| 2000x2000 | 10 | 24 | 0,77317 s | 0,31358 s | 2,47x |
| 2000x2000 | 100 | 48 | 3,99144 s | 0,36970 s | 10,80x |

O maior speedup medido nesta rodada foi 10,80x. Para 2000x2000 e G=100, passar
de 24 para 48 threads reduziu o tempo paralelo de 0,39204 s para 0,36970 s,
ganho de aproximadamente 5,7%. Nos outros cinco casos, 48 threads nao foram a
configuracao mais rapida; os melhores resultados ficaram entre 16 e 24 threads.
Portanto, a escolha otima depende do tamanho da grade e de G. O valor 10,80x e
valido para estes inputs sinteticos com uma regra B3/S23 e nao garante o mesmo
resultado em instancias com outra distribuicao de regras ou dimensoes.

Com esta comparacao, a etapa de speedup contra a referencia sequencial esta
concluida. A proxima otimizacao a investigar e a leitura da matriz de regras,
mantendo estes mesmos inputs e repeticoes para comparar o efeito sem misturar
mudancas no kernel paralelo.

## Duvidas
