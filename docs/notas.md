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

obs: quando row == 0 o ponteiro up aponta pra fora do vetor, mas nesse caso o
tem_cima e falso e o ramo do miolo nunca roda, entao ninguem le aquilo.

obs: ainda nao tem nenhuma linha de paralelismo, continua 100% sequencial.

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

Ainda nao ha pthreads nem medicao de ganho de desempenho.

## Separacao em modulos

O codigo foi dividido por responsabilidade: `game_rules` interpreta as regras,
`grid` calcula a evolucao, `simulation` define a configuracao e libera seus recursos,
e `io` cuida da leitura e impressao. `life_par.c` ficou com o `main`, o loop das
geracoes e a troca dos buffers. `count_neighbors` ficou interna a `grid.c`;
`update_rows` continua publica para as futuras threads.

## Duvidas
