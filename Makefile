CC = gcc
CFLAGS = -Wall $(shell pkg-config --cflags allegro-5 allegro_main-5 allegro_primitives-5)
LIBS = $(shell pkg-config --libs allegro-5 allegro_main-5 allegro_primitives-5)

jogo: main.c
	$(CC) main.c -o jogo $(CFLAGS) $(LIBS)

run: jogo
	./jogo

clean:
	rm -f jogo

.PHONY: run clean
