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

## Duvidas
