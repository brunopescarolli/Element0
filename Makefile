CC = gcc
CFLAGS = -Wall -Wextra -Isrc $(shell pkg-config --cflags allegro-5 allegro_main-5 allegro_primitives-5 allegro_image-5 allegro_font-5)
LIBS = $(shell pkg-config --libs allegro-5 allegro_main-5 allegro_primitives-5 allegro_image-5 allegro_font-5)

SRC = src/main.c src/game.c src/player.c src/enemy.c src/weapon.c src/physics.c src/render.c src/elementos.c
OBJ = $(SRC:.c=.o)

jogo: $(OBJ)
	$(CC) $(OBJ) -o jogo $(LIBS) -static-libgcc -static-libstdc++

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

run: jogo
	./jogo

clean:
	rm -f src/*.o jogo jogo.exe

.PHONY: run clean
