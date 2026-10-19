#ifndef WEAPON_H
#define WEAPON_H

#include "defs.h"

void disparar_arma_jogador(Jogador *faxineiro, Projetil *projeteis, int max_proj);
void atualizar_projeteis(Projetil *projeteis, int max_proj, int sala_atual);

#endif
