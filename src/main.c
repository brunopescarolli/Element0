#include "game.h"

int main(void) {
    srand(time(NULL));

    Game game = {0};
    if (!game_init(&game)) {
        return -1;
    }

    game_run(&game);
    game_cleanup(&game);

    return 0;
}
