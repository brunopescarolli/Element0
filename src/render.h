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
    ALLEGRO_BITMAP *sprite_bateria;
    ALLEGRO_BITMAP *sprite_refil;
    ALLEGRO_BITMAP *sprite_soprador;
    ALLEGRO_BITMAP *sprite_medkit;
    ALLEGRO_BITMAP *sprite_hud_vida;
    ALLEGRO_BITMAP *sprite_hud_agua;
    ALLEGRO_BITMAP *sprite_hud_bateria;
    ALLEGRO_BITMAP *sprite_hud_caveira;
    ALLEGRO_BITMAP *sprite_scanner;
} GameSprites;

bool scanner_posicao_dentro_painel(float mouse_x, float mouse_y);
bool scanner_clique_botao_voltar(float mouse_x, float mouse_y);
bool scanner_clique_botao_fechar(float mouse_x, float mouse_y);
int scanner_obter_item_clicado_menu(float mouse_x, float mouse_y, const bool *elemento_descoberto);
bool scanner_processar_clique_mouse(int *modo_scanner, bool *scanner_ativo, int *elemento_foco_id,
                                    const bool *elemento_descoberto, float mouse_x, float mouse_y);
void desenhar_visor_scanner(int modo_scanner, float tempo_scan, int elemento_foco_id,
                            const bool *elemento_descoberto, const Inimigo *anomalia,
                            ALLEGRO_FONT *fonte);

ALLEGRO_COLOR obter_cor_temperatura(float temp);
void obter_cores_temperatura(float temp, ALLEGRO_COLOR *cor_borda, ALLEGRO_COLOR *cor_centro);
void desenhar_sprite_quente(ALLEGRO_BITMAP *sprite, float centro_x, float centro_y, float larg_fallback, float alt_fallback, int flags, float temp, int frame_atual);
void desenhar_particulas(Particula *particulas, int max_particulas);
void desenhar_segmento_bateria(float x, float y, ALLEGRO_COLOR cor);
void desenhar_segmento_bateria_topo(float x, float y, ALLEGRO_COLOR cor);
void desenhar_icone_refil_agua(float x, float y, ALLEGRO_COLOR cor);
void desenhar_icone_bateria_guardada(float x, float y, ALLEGRO_COLOR cor);
void desenhar_hud_jogador(Jogador *faxineiro, GameSprites *sprites);

void desenhar_mundo(int sala_atual, Jogador *faxineiro, Inimigo *anomalia, Item *arma_mop, Item *arma_soprador,
                    Soprador *soprador, Coletavel *refil_agua, Coletavel *bateria, Coletavel *kit_medico,
                    Coletavel *item_scanner,
                    Projetil *projeteis, int max_proj, Particula *particulas, int max_particulas,
                    Plataforma *plataformas, int total_plat,
                    Escada *escadas, int total_escadas, Porta *portas, int total_portas,
                    Elevador *elevadores, int total_elevadores, TextoFlutuante *textos, int max_textos,
                    GameSprites *sprites, bool prop_na_frente, int estado_fade, int alpha_fade,
                    float alpha_game_over, int opcao_game_over,
                    int modo_scanner, float tempo_scan, int elemento_foco_id, const bool *elemento_descoberto,
                    bool mostrar_hitbox, ALLEGRO_FONT *fonte, float largura_camera, float altura_camera);

void desenhar_ui(int estado_jogo, int opcao_menu, int opcao_confirma, int opcao_pause,
                 int opcao_config, int slot_sendo_remapeado, Controles *controles,
                 const char *nome_jogador, ALLEGRO_FONT *fonte);

#endif
