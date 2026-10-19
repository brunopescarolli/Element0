#ifndef ENEMY_H
#define ENEMY_H

#include "defs.h"

void atualizar_inimigo(Inimigo *anomalia, Jogador *faxineiro,
                        Plataforma *plataformas, int total_plat, Escada *escadas, int total_escadas,
                        int sala_atual, int estado_fade, Projetil *projeteis, int max_proj);

#endif
