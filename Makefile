CC = gcc
CFLAGS = -Wall -Isrc $(shell pkg-config --cflags allegro-5 allegro_main-5 allegro_primitives-5 allegro_image-5 allegro_font-5)
LIBS = $(shell pkg-config --libs allegro-5 allegro_main-5 allegro_primitives-5 allegro_image-5 allegro_font-5)

SRC = src/main.c src/elementos.c
OBJ = $(SRC:.c=.o)

jogo: $(OBJ)
	$(CC) $(OBJ) -o jogo $(LIBS) -static-libgcc -static-libstdc++

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

run: jogo
	./jogo

clean:
	rm -f src/*.o jogo

.PHONY: run clean
