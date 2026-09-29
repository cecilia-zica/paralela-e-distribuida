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

## Duvidas

## Validacao da entrada e tamanhos (sem modularizacao)

As alteracoes continuam em src/life_par.c. read_int usa strtol com verificacao
de limites para evitar overflow na conversao de numeros de entrada, inclusive
nos identificadores de regra. Exige L/C positivos, G nao negativo e R positivo.
Como fgets recebe tamanho int, C deve ser no maximo INT_MAX - 3.

Tamanhos e indices lineares usam size_t. checked_product verifica os limites
antes de multiplicar dimensoes ou calcular bytes para malloc. Falhas de alocacao
continuam retornando erro; tamanho representavel nao garante memoria disponivel.

Cada linha da grade deve ter exatamente C posicoes. Espacos finais contam como
celulas; LF e CRLF sao aceitos, assim como a ultima linha completa sem quebra.
Linhas curtas/longas sao rejeitadas antes de consultar estados incompletos.

OBS: com essas validações, o life-4 está dando erro, pois foram definidas 40 colunas e algumas linhas da grade contem 41 ou 42 caracteres. fiquei em duvida se é um erro proposital do professor ou nao
