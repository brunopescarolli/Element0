#ifndef RENDER_H
#define RENDER_H

#include "defs.h"

typedef struct {
    ALLEGRO_BITMAP *backgrounds[MAX_SALAS];
    ALLEGRO_BITMAP *bg_escadaria;
    ALLEGRO_BITMAP *sprite_porta_fechada;
    ALLEGRO_BITMAP *sprite_porta_meio;
    ALLEGRO_BITMAP *sprite_porta_aberta;
    ALLEGRO_BITMAP *sprite_porta_exit_fechada;
    ALLEGRO_BITMAP *sprite_porta_exit_meio;
    ALLEGRO_BITMAP *sprite_porta_exit_aberta;
    ALLEGRO_BITMAP *sprite_chao;
    ALLEGRO_BITMAP *sprite_escada;
    ALLEGRO_BITMAP *sprite_escada_vertical;
    ALLEGRO_BITMAP *sprite_prop1;
    ALLEGRO_BITMAP *sprite_water;
} GameSprites;

void desenhar_mundo(int sala_atual, Jogador *faxineiro, Inimigo *anomalia, Item *arma_mop, Item *arma_soprador,
                    Soprador *soprador, Coletavel *refil_agua, Coletavel *bateria, Coletavel *kit_medico,
                    Projetil *projeteis, int max_proj, Plataforma *plataformas, int total_plat,
                    Escada *escadas, int total_escadas, Porta *portas, int total_portas,
                    Elevador *elevadores, int total_elevadores, TextoFlutuante *textos, int max_textos,
                    GameSprites *sprites, bool prop_na_frente, int estado_fade, int alpha_fade,
                    bool mostrar_hitbox, ALLEGRO_FONT *fonte, float largura_camera, float altura_camera);

void desenhar_ui(int estado_jogo, int opcao_menu, int opcao_confirma, int opcao_pause,
                 int opcao_config, int slot_sendo_remapeado, Controles *controles,
                 const char *nome_jogador, ALLEGRO_FONT *fonte);

#endif
