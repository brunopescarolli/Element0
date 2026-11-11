#ifndef ENEMY_H
#define ENEMY_H

#include "defs.h"

void instanciar_particula(Particula *particulas, int max_particulas,
                          float x, float y, float vx, float vy,
                          float vida_maxima, int tipo, ALLEGRO_COLOR cor, float tamanho);
void instanciar_splash_agua(Particula *particulas, int max_particulas, float x, float y, float vel_x_tiro);
void atualizar_particulas(Particula *particulas, int max_particulas);

void atualizar_inimigo(Inimigo *anomalia, Jogador *faxineiro,
                        Plataforma *plataformas, int total_plat, Escada *escadas, int total_escadas,
                        int sala_atual, int estado_fade, Projetil *projeteis, int max_proj,
                        Particula *particulas, int max_particulas,
                        MarcaFuligem *marcas_fuligem, int max_marcas,
                        TextoFlutuante *textos, int max_textos);

#endif
