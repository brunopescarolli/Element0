#ifndef WEAPON_H
#define WEAPON_H

#include "defs.h"

void instanciar_texto_flutuante(TextoFlutuante *textos, int max_textos, float x, float y, const char *mensagem, ALLEGRO_COLOR cor);
void disparar_arma_jogador(Jogador *faxineiro, Projetil *projeteis, int max_proj, TextoFlutuante *textos, int max_textos);
void atualizar_soprador(Jogador *faxineiro, Soprador *soprador, bool atirando, TextoFlutuante *textos, int max_textos);
void atualizar_projeteis(Projetil *projeteis, int max_proj, int sala_atual);

#endif
