#ifndef PLAYER_H
#define PLAYER_H

#include "defs.h"

void atualizar_jogador(Jogador *faxineiro, Input *input, Item *arma_mop, Item *arma_soprador,
                       Soprador *soprador, Coletavel *refil_agua, Coletavel *bateria, Coletavel *kit_medico,
                       Inimigo *anomalia, Plataforma *plataformas, int total_plat, Escada *escadas, int total_escadas,
                       Porta *portas, int total_portas, Projetil *projeteis, int max_proj,
                       TextoFlutuante *textos, int max_textos,
                       int *sala_atual, int *estado_fade, int *proxima_sala, float *proximo_x, float *proximo_chao_y);

#endif
