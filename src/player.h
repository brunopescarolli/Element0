#ifndef PLAYER_H
#define PLAYER_H

#include "defs.h"

void atualizar_jogador(Jogador *faxineiro, Input *input, Item *arma_mop, Inimigo *anomalia,
                       Plataforma *plataformas, int total_plat, Escada *escadas, int total_escadas,
                       Porta *portas, int total_portas, Projetil *projeteis, int max_proj,
                       int *sala_atual, int *estado_fade, int *proxima_sala, float *proximo_x, float *proximo_chao_y);

#endif
