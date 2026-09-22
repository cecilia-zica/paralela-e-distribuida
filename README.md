# T1 - INE5645 - Simulacao de automatos celulares

## Integrantes (Grupo 05238A C)

- Cecilia Zica Camargo - 24208346
- Joao Paulo Decker - 24202528
- Marina Benvenuti Cardeal - 23103131

## Compilacao

    make

Gera dois executaveis:

- `life` - versao sequencial de referencia (Professor)
- `life_par` - versao paralela desenvolvida pelo grupo

## Execucao

## Estrutura do projeto

    .
    ├── life.c              # referencia sequencial do professor (raiz, nao mover/mexer)
    ├── run.sh              # script de execucao do professor (raiz, nao mover/mexer)
    ├── life-*.in / .out    # massa de teste do professor (raiz, nao mover/mexer)
    ├── Makefile
    ├── src/
    │   └── life_par.c      # versao paralela do grupo (evolui aqui)
    ├── include/            # headers .h, conforme life_par.c for modularizado
    ├── docs/               # enunciado (PDF) e notas do grupo
    └── build/              # binarios/objetos gerados (git-ignorado)

Os arquivos do professor ficam na raiz de propósito: `run.sh` assume que o
binario `life` e os `.in` estao no mesmo diretorio de onde ele e chamado, e
seu conteudo nao pode ser alterado. Mover esses arquivos quebraria esse
contrato sem necessidade.
