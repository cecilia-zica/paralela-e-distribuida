CC=cc
FLAGS=-O3

all: life life_par

life: life.c
	$(CC) $(FLAGS) life.c -o life

life_par: src/life_par.c
	$(CC) $(FLAGS) src/life_par.c -o life_par

clean:
	rm -f life life_par
