CC=cc
FLAGS=-O3

all: life life_par parallel_static parallel_dynamic

life: life.c
	$(CC) $(FLAGS) life.c -o life

life_par: src/life_par.c src/io.c src/grid.c src/simulation.c src/io.h src/game_rules.h
	$(CC) $(FLAGS) src/life_par.c src/io.c src/grid.c src/simulation.c -o life_par

parallel_static: src/parallel_static.c src/io.c src/grid.c src/simulation.c src/io.h src/game_rules.h
	$(CC) $(FLAGS) -pthread src/parallel_static.c src/io.c src/grid.c src/simulation.c -o parallel_static

parallel_dynamic: src/parallel_dynamic.c src/io.c src/grid.c src/simulation.c src/io.h src/game_rules.h
	$(CC) $(FLAGS) -std=c11 -D_DEFAULT_SOURCE -pthread src/parallel_dynamic.c src/io.c src/grid.c src/simulation.c -o parallel_dynamic

clean:
	rm -f life life_par parallel_static parallel_dynamic

.PHONY: all clean test

test: life_par
	python3 tests/test_life.py ./life_par
