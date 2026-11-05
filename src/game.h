#ifndef GAME_H
#define GAME_H

#include "defs.h"
#include "render.h"

#define TOTAL_PLATAFORMAS 35
#define TOTAL_PORTAS 62
#define TOTAL_ESCADAS 8
#define TOTAL_ELEVADORES 12
#define MAX_TEXTOS_FLUTUANTES 8

typedef struct {
    ALLEGRO_DISPLAY *tela;
    ALLEGRO_EVENT_QUEUE *fila_eventos;
    ALLEGRO_TIMER *timer;
    ALLEGRO_FONT *fonte;

    bool rodando;
    bool redesenhar;
    bool mostrar_hitbox;

    int estado_jogo;
    int opcao_menu;
    int opcao_confirma;
    int opcao_pause;
    int opcao_config;
    int opcao_game_over;
    int slot_sendo_remapeado;
    float alpha_game_over;

    char nome_jogador[30];
    Controles controles;

    int sala_atual;
    int estado_fade;
    int alpha_fade;
    int timer_fade;
    int proxima_sala;
    float proximo_x;
    float proximo_chao_y;
    bool prop_na_frente;

    Input input;
    Jogador faxineiro;
    Item arma_mop;
    Item arma_soprador;
    Soprador soprador;
    Coletavel refil_agua;
    Coletavel bateria;
    Coletavel kit_medico;
    Inimigo anomalia;
    Projetil projeteis[MAX_PROJETEIS];

    Plataforma mundo_plataformas[TOTAL_PLATAFORMAS];
    Porta mundo_portas[TOTAL_PORTAS];
    Escada mundo_escadas[TOTAL_ESCADAS];
    Elevador mundo_elevadores[TOTAL_ELEVADORES];
    TextoFlutuante textos_flutuantes[MAX_TEXTOS_FLUTUANTES];

    GameSprites sprites;
} Game;

bool game_init(Game *game);
void game_run(Game *game);
void game_cleanup(Game *game);

#endif
