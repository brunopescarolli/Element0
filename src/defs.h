#ifndef DEFS_H
#define DEFS_H

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <math.h>
#include <time.h>
#include <allegro5/allegro.h>
#include <allegro5/allegro_primitives.h>
#include <allegro5/allegro_image.h>
#include <allegro5/allegro_font.h>

#define LARGURA_JANELA 1920
#define ALTURA_JANELA  1080
#define LARGURA_MAPA   480
#define ALTURA_MAPA    270
#define FPS          60.0
#define ESPESSURA_PAREDE 10.0

#define MAX_PROJETEIS 10
#define VELOCIDADE_PROJETIL 6.0
#define COOLDOWN_TIRO 60

#define GRAVIDADE 0.3
#define FORCA_PULO -5.2

#define MAX_SALAS 10

#define TEMP_ESTACAO 20.0
#define TEMP_JOGADOR 36.0
#define TEMP_INDETERMINADA 99999.0
#define MAX_ELEMENTOS 118

enum GameState {
    ESTADO_MENU_PRINCIPAL,
    ESTADO_CONFIRMAR_NOVO_JOGO,
    ESTADO_DIGITAR_NOME,
    ESTADO_CONFIGURACOES,
    ESTADO_RECORDES,
    ESTADO_JOGANDO,
    ESTADO_PAUSADO
};

enum ConfigSlot {
    SLOT_PULAR = 0,
    SLOT_ESQUERDA,
    SLOT_DIREITA,
    SLOT_ATIRAR,
    SLOT_PEGAR,
    SLOT_SOLTAR,
    SLOT_ENTRAR,
    TOTAL_SLOTS
};

enum EstadoFisico { SOLIDO, LIQUIDO, GASOSO };
enum Direcao { DIR_ESQUERDA = 0, DIR_DIREITA = 1 };
enum TipoFrame { FRAME_IDLE = 0, FRAME_BRE = 1, FRAME_WALK1 = 2, FRAME_WALK2 = 3 };

enum TipoProjetil { PROJETIL_AGUA, PROJETIL_FOGO, PROJETIL_NORMAL };

typedef struct {
    float eixo_x;
    bool pulo;
    bool atirando;
    bool acao_pegar;
    bool acao_soltar;
    bool acao_entrar;
    bool acao_descer;
    bool f3_pressionado;
} Input;

typedef struct {
    int tecla_pular;
    int tecla_esquerda;
    int tecla_direita;
    int tecla_atirar;
    int tecla_pegar;
    int tecla_soltar;
    int tecla_entrar;
} Controles;

typedef struct {
    float x, y;
    float w, h;
    float body_w, body_h;
    float velocidade;
    float vel_y;
    bool no_chao;
    int direcao_atual;
    int cooldown;

    ALLEGRO_BITMAP *sprites[4];
    bool esta_andando;
    int frame_atual;
    int tempo_animacao;

    bool possui_arma;
} Jogador;

typedef struct {
    float x, y;
    float w, h;
    int sala;
    bool atravessavel;
} Plataforma;

typedef struct {
    float x, y;
    float w, h;
    int sala;
    int direcao;
} Escada;

typedef struct {
    float x, y;
    float w, h;
    int sala_origem;
    int destino_sala;
    bool jogador_perto;
    int timer_animacao;
    int tipo;
} Porta;

typedef struct {
    float x, y;
    float vel_y;
    float raio_interacao;
    bool no_chao;
    int sala_origem;
    ALLEGRO_BITMAP *sprite;
} Item;

typedef struct {
    float x, y;
    float vel_x;
    bool ativo;
    int tipo;
} Projetil;

typedef struct {
    int numero_atomico;
    char simbolo[5];
    char nome[30];
    int periodo;
    char grupo_familia[50];
    char categoria[50];
    int estado_fisico_20C;
    bool magnetico;
    bool radioativo;
    float ponto_fusao;
    float ponto_ebulicao;
    float pontos_de_vida_dureza;
} DadosElemento;

extern DadosElemento tabela_periodica[MAX_ELEMENTOS + 1];

typedef struct {
    float x, y;
    float w, h;

    float velocidade;
    float vel_y;
    float vel_x_atual;
    bool no_chao;

    int sala_origem;
    float alvo_x;

    float raio_visao;

    int numero_atomico;

    float temperatura;
    bool recebendo_calor;
    int estado_atual;
    ALLEGRO_COLOR cor;

    ALLEGRO_BITMAP *sprites_normal[5];
    ALLEGRO_BITMAP *sprites_bre[5];
    int frame_atual;
    int tempo_animacao;
    int frame_animacao;
    int direcao_atual;
    int timer_resfriamento;

    int timer_agro;
    int timer_patrulha;
} Inimigo;

#endif
