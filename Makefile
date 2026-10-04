CC=cc
FLAGS=-O3

all: life life_par parallel_static
LIFE_PAR_SOURCES=src/life_par.c src/io.c src/game_rules.c src/grid.c src/simulation.c
LIFE_PAR_HEADERS=src/io.h src/game_rules.h src/grid.h src/simulation.h

all: life life_par

life: life.c
	$(CC) $(FLAGS) life.c -o life


parallel_static: src/parallel_static.c src/io.c src/grid.c src/io.h src/game_rules.h
	$(CC) $(FLAGS) -pthread src/parallel_static.c src/io.c src/grid.c -o parallel_static
life_par: $(LIFE_PAR_SOURCES) $(LIFE_PAR_HEADERS)
	$(CC) $(FLAGS) $(LIFE_PAR_SOURCES) -o life_par

clean:
	rm -f life life_par parallel_static

.PHONY: all clean test

test: life_par
	python3 tests/test_life.py ./life_par
