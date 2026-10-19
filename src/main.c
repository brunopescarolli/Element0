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
    int tipo; // NOVO: Define qual sprite carregar (0 = Padrão, 1 = Emergência)
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

DadosElemento tabela_periodica[MAX_ELEMENTOS + 1]; 

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

void carregar_controles_padrao(Controles *c) {
    c->tecla_pular = ALLEGRO_KEY_SPACE;
    c->tecla_esquerda = ALLEGRO_KEY_A;
    c->tecla_direita = ALLEGRO_KEY_D;
    c->tecla_atirar = ALLEGRO_KEY_F;
    c->tecla_pegar = ALLEGRO_KEY_E;
    c->tecla_soltar = ALLEGRO_KEY_Q;
    c->tecla_entrar = ALLEGRO_KEY_W;
}

void salvar_jogo(char *nome, int sala, float x, float y, bool possui_arma) {
    FILE *f = fopen("save.dat", "w");
    if (f) {
        fprintf(f, "%s\n%d\n%f\n%f\n%d\n", nome, sala, x, y, possui_arma ? 1 : 0);
        fclose(f);
    }
}

bool carregar_jogo(char *nome, int *sala, float *x, float *y, bool *possui_arma) {
    FILE *f = fopen("save.dat", "r");
    if (f) {
        int arma_temp = 0;
        if (fscanf(f, "%29s\n%d\n%f\n%f\n%d", nome, sala, x, y, &arma_temp) == 5) {
            *possui_arma = (arma_temp == 1);
            fclose(f);
            return true;
        }
        fclose(f);
    }
    return false;
}

char* proximo_campo_csv(char **cursor) {
    if (!*cursor) return NULL;
    char *inicio = *cursor;
    char *virgula = strchr(inicio, ',');
    if (virgula) {
        *virgula = '\0';
        *cursor = virgula + 1;
    } else {
        char *newline = strpbrk(inicio, "\r\n");
        if (newline) *newline = '\0';
        *cursor = NULL; 
    }
    return inicio;
}

void carregar_tabela_periodica(const char *caminho_arquivo) {
    FILE *file = fopen(caminho_arquivo, "r");
    if (!file) {
        printf("Aviso: 'elementos.csv' nao encontrado. Usando fallbacks.\n");
        tabela_periodica[31].numero_atomico = 31;
        tabela_periodica[31].ponto_fusao = 29.8;
        return;
    }

    char linha[1024];
    fgets(linha, sizeof(linha), file); 

    while (fgets(linha, sizeof(linha), file)) {
        char *cursor = linha;
        DadosElemento e;

        e.numero_atomico = atoi(proximo_campo_csv(&cursor));
        strcpy(e.simbolo, proximo_campo_csv(&cursor) ? "" : ""); 
        strcpy(e.nome, proximo_campo_csv(&cursor) ? "" : "");
        e.periodo = atoi(proximo_campo_csv(&cursor));
        strcpy(e.grupo_familia, proximo_campo_csv(&cursor) ? "" : "");
        strcpy(e.categoria, proximo_campo_csv(&cursor) ? "" : "");
        
        char *str_estado = proximo_campo_csv(&cursor);
        if (str_estado && (strcmp(str_estado, "Sólido") == 0 || strcmp(str_estado, "Solido") == 0)) e.estado_fisico_20C = SOLIDO;
        else if (str_estado && (strcmp(str_estado, "Líquido") == 0 || strcmp(str_estado, "Liquido") == 0)) e.estado_fisico_20C = LIQUIDO;
        else e.estado_fisico_20C = GASOSO;

        char *str_mag = proximo_campo_csv(&cursor);
        e.magnetico = (str_mag && strcmp(str_mag, "Sim") == 0);

        char *str_rad = proximo_campo_csv(&cursor);
        e.radioativo = (str_rad && strcmp(str_rad, "Sim") == 0);

        char *str_fusao = proximo_campo_csv(&cursor);
        e.ponto_fusao = (str_fusao && strlen(str_fusao) > 0) ? atof(str_fusao) : TEMP_INDETERMINADA;

        char *str_ebulicao = proximo_campo_csv(&cursor);
        e.ponto_ebulicao = (str_ebulicao && strlen(str_ebulicao) > 0) ? atof(str_ebulicao) : TEMP_INDETERMINADA;

        char *str_hp = proximo_campo_csv(&cursor);
        e.pontos_de_vida_dureza = (str_hp && strlen(str_hp) > 0) ? atof(str_hp) : 100.0;

        if (e.numero_atomico > 0 && e.numero_atomico <= MAX_ELEMENTOS) {
            tabela_periodica[e.numero_atomico] = e;
        }
    }
    fclose(file);
}

int main(void) {
    srand(time(NULL)); 

    ALLEGRO_DISPLAY *tela = NULL;
    ALLEGRO_EVENT_QUEUE *fila_eventos = NULL;
    ALLEGRO_TIMER *timer = NULL;
    ALLEGRO_FONT *fonte = NULL;

    bool rodando = true;
    bool redesenhar = true;
    bool mostrar_hitbox = false; 

    int estado_jogo = ESTADO_MENU_PRINCIPAL;
    int opcao_menu = 0; 
    int opcao_confirma = 0; 
    int opcao_pause = 0; 
    int opcao_config = 0; 
    int slot_sendo_remapeado = -1; 

    char nome_jogador[30] = "";
    int recorde_fase = 3; 

    Controles controles;
    carregar_controles_padrao(&controles);

    int sala_atual = 0;
    int estado_fade = 0;   
    int alpha_fade = 0;    
    int timer_fade = 0;    
    int proxima_sala = 0;
    float proximo_x = 0, proximo_chao_y = 0;
    
    bool prop_na_frente = false;

    Input input = {0.0, false, false, false, false, false, false, false};

    if (!al_init()) return -1;
    if (!al_install_keyboard()) return -1;
    if (!al_install_mouse()) return -1; 
    if (!al_init_primitives_addon()) return -1;
    if (!al_init_image_addon()) return -1;
    al_init_font_addon();
    al_install_joystick(); 

    al_set_new_display_flags(ALLEGRO_FULLSCREEN_WINDOW);
    tela = al_create_display(LARGURA_JANELA, ALTURA_JANELA);
    al_set_blender(ALLEGRO_ADD, ALLEGRO_ALPHA, ALLEGRO_INVERSE_ALPHA);

    fonte = al_create_builtin_font();
    carregar_tabela_periodica("elementos.csv");

    ALLEGRO_BITMAP *backgrounds[MAX_SALAS];
    for (int i = 0; i < MAX_SALAS; i++) {
        char path[128];
        sprintf(path, "sprites/background/fundo_%d.png", i); 
        backgrounds[i] = al_load_bitmap(path);
    }

    ALLEGRO_BITMAP *sprite_porta_fechada = al_load_bitmap("sprites/background/door_closed.png");
    ALLEGRO_BITMAP *sprite_porta_meio = al_load_bitmap("sprites/background/door_open1.png");
    ALLEGRO_BITMAP *sprite_porta_aberta = al_load_bitmap("sprites/background/door_open2.png");
    
    // SPRITES DA PORTA DE EMERGÊNCIA
    ALLEGRO_BITMAP *sprite_porta_exit_fechada = al_load_bitmap("sprites/background/door_exit_closed.png");
    ALLEGRO_BITMAP *sprite_porta_exit_meio = al_load_bitmap("sprites/background/door_exit_open1.png");
    ALLEGRO_BITMAP *sprite_porta_exit_aberta = al_load_bitmap("sprites/background/door_exit_open2.png");
    
    ALLEGRO_BITMAP *sprite_chao = al_load_bitmap("sprites/background/chao.png");
    ALLEGRO_BITMAP *sprite_escada = al_load_bitmap("sprites/escada.png");
    ALLEGRO_BITMAP *sprite_prop1 = al_load_bitmap("sprites/prop1.png"); 

    Jogador faxineiro = { 
        32.0, 127.0 - (52.0 / 2.0), 
        10.0, 4.0,   
        16.0, 52.0,  
        2.5, 0.0, true, DIR_DIREITA, 0 
    };
    faxineiro.esta_andando = false;
    faxineiro.frame_atual = 0;
    faxineiro.tempo_animacao = 0;
    faxineiro.possui_arma = false; 

    faxineiro.sprites[FRAME_IDLE]  = al_load_bitmap("sprites/front_idle.png");
    faxineiro.sprites[FRAME_BRE]   = al_load_bitmap("sprites/front_bre.png");
    faxineiro.sprites[FRAME_WALK1] = al_load_bitmap("sprites/front_walk1.png");
    faxineiro.sprites[FRAME_WALK2] = al_load_bitmap("sprites/front_walk2.png");
    bool tem_sprites = (faxineiro.sprites[FRAME_IDLE] != NULL);

    #define TOTAL_PLATAFORMAS 7
    Plataforma mundo_plataformas[TOTAL_PLATAFORMAS] = {
        { 0, 127, 240.0, 8, 0, false },  
        { 0, 254, LARGURA_MAPA, 16, 1, false }, 
        { 192, 190, 288, 8, 1, true },
        { 0, 126, 160, 8, 1, true },
        { 160, 158, 32, 8, 1, true },    
        { 0, 254, LARGURA_MAPA, 16, 2, false }  
    };

    #define TOTAL_PORTAS 6
    Porta mundo_portas[TOTAL_PORTAS] = {
        // Formato: x, y, w, h, sala_origem, destino_sala, jogador_perto, timer_animacao, TIPO (0=Padrão, 1=Emergência)
        { 171, 31, 64, 96, 0, 1, false, 0, 0 },                
        { 0, 158, 64, 96, 1, 0, false, 0, 0 }, 
        { 416, 94, 64, 96, 1, 2, false, 0, 0 }, 
        { 0, 158, 64, 96, 2, 1, false, 0, 0 },
        { 416, 158, 64, 96, 2, 3, false, 0, 0 },
        // NOVO: Porta de Emergência no meio da Sala 2 (voltando pra sala 1)
        { 320, 158, 64, 96, 2, 1, false, 0, 1 }
    };

    #define TOTAL_ESCADAS 2
    Escada mundo_escadas[TOTAL_ESCADAS] = {
        { 192, 158, 32, 32, 1, -1 },
        { 160, 126, 32, 32, 1, -1 }
    };

    Item arma_mop = { 62.0, 127.0, 0.0, 30.0, true, 0, NULL }; 
    arma_mop.sprite = al_load_bitmap("sprites/mop.png");
    ALLEGRO_BITMAP *sprite_water = al_load_bitmap("sprites/water.png");

    Inimigo anomalia = { 
        100, 254.0 - (30.0 / 2.0), 40.0, 30.0, 
        0.5, 0.0, 0.0, true, 
        1, 0.0, 
        180.0, 
        31, 
        TEMP_ESTACAO, false, SOLIDO, al_map_rgb(210, 210, 220), 
        {NULL, NULL, NULL, NULL, NULL}, 
        {NULL, NULL, NULL, NULL, NULL}, 
        0, 0, 0,
        DIR_DIREITA, 0,
        0, 0 
    };
    
    for (int i = 0; i < 5; i++) {
        char path[64];
        sprintf(path, "sprites/galio_%d.png", i + 1);
        anomalia.sprites_normal[i] = al_load_bitmap(path);

        sprintf(path, "sprites/galio_%d_bre.png", i + 1);
        anomalia.sprites_bre[i] = al_load_bitmap(path);
    }

    Projetil projeteis[MAX_PROJETEIS];
    for (int i = 0; i < MAX_PROJETEIS; i++) projeteis[i].ativo = false;

    timer = al_create_timer(1.0 / FPS);

    fila_eventos = al_create_event_queue();
    al_register_event_source(fila_eventos, al_get_display_event_source(tela));
    al_register_event_source(fila_eventos, al_get_timer_event_source(timer));
    al_register_event_source(fila_eventos, al_get_keyboard_event_source());
    al_register_event_source(fila_eventos, al_get_mouse_event_source()); 

    al_start_timer(timer);

    while (rodando) {
        ALLEGRO_EVENT evento;
        al_wait_for_event(fila_eventos, &evento);

        float limite_mapa_x = (sala_atual == 0) ? (LARGURA_MAPA / 2.0) : LARGURA_MAPA;
        float limite_mapa_y = (sala_atual == 0) ? (ALTURA_MAPA / 2.0) : ALTURA_MAPA;

        if (evento.type == ALLEGRO_EVENT_DISPLAY_CLOSE) {
            rodando = false;
        }
        else if (evento.type == ALLEGRO_EVENT_KEY_CHAR) {
            if (estado_jogo == ESTADO_DIGITAR_NOME) {
                int keycode = evento.keyboard.unichar;
                if (keycode == 8) { 
                    int len = strlen(nome_jogador);
                    if (len > 0) nome_jogador[len - 1] = '\0';
                } else if (keycode == 13) { 
                    if (strlen(nome_jogador) > 0) {
                        sala_atual = 0;
                        
                        faxineiro.x = 32.0;
                        faxineiro.y = 127.0 - (faxineiro.h / 2.0); 
                        faxineiro.possui_arma = false;
                        
                        arma_mop.sala_origem = 0;
                        arma_mop.no_chao = true;
                        arma_mop.x = faxineiro.x + 30.0;
                        arma_mop.y = 127.0; 
                        arma_mop.vel_y = 0;
                        
                        salvar_jogo(nome_jogador, sala_atual, faxineiro.x, faxineiro.y, faxineiro.possui_arma);
                        estado_jogo = ESTADO_JOGANDO;
                    }
                } else if (keycode >= 32 && keycode <= 126) {
                    int len = strlen(nome_jogador);
                    if (len < 28) {
                        nome_jogador[len] = (char)keycode;
                        nome_jogador[len + 1] = '\0';
                    }
                }
                redesenhar = true;
            }
        }
        else if (evento.type == ALLEGRO_EVENT_KEY_DOWN) {
            if (slot_sendo_remapeado != -1) {
                int tecla = evento.keyboard.keycode;
                if (slot_sendo_remapeado == SLOT_PULAR) controles.tecla_pular = tecla;
                else if (slot_sendo_remapeado == SLOT_ESQUERDA) controles.tecla_esquerda = tecla;
                else if (slot_sendo_remapeado == SLOT_DIREITA) controles.tecla_direita = tecla;
                else if (slot_sendo_remapeado == SLOT_ATIRAR) controles.tecla_atirar = tecla;
                else if (slot_sendo_remapeado == SLOT_PEGAR) controles.tecla_pegar = tecla;
                else if (slot_sendo_remapeado == SLOT_SOLTAR) controles.tecla_soltar = tecla;
                else if (slot_sendo_remapeado == SLOT_ENTRAR) controles.tecla_entrar = tecla;
                slot_sendo_remapeado = -1; 
            }
            else if (estado_jogo == ESTADO_MENU_PRINCIPAL) {
                if (evento.keyboard.keycode == ALLEGRO_KEY_DOWN || evento.keyboard.keycode == ALLEGRO_KEY_S) {
                    opcao_menu = (opcao_menu + 1) % 4;
                }
                else if (evento.keyboard.keycode == ALLEGRO_KEY_UP || evento.keyboard.keycode == ALLEGRO_KEY_W) {
                    opcao_menu = (opcao_menu - 1 + 4) % 4;
                }
                else if (evento.keyboard.keycode == ALLEGRO_KEY_ENTER || evento.keyboard.keycode == ALLEGRO_KEY_SPACE) {
                    if (opcao_menu == 0) { 
                        FILE *f = fopen("save.dat", "r");
                        if (f) {
                            fclose(f);
                            estado_jogo = ESTADO_CONFIRMAR_NOVO_JOGO; 
                            opcao_confirma = 1;
                        } else {
                            strcpy(nome_jogador, "");
                            estado_jogo = ESTADO_DIGITAR_NOME;
                        }
                    } else if (opcao_menu == 1) { 
                        FILE *f_chk = fopen("save.dat", "r");
                        if (f_chk) {
                            fclose(f_chk);
                            if (carregar_jogo(nome_jogador, &sala_atual, &faxineiro.x, &faxineiro.y, &faxineiro.possui_arma)) {
                                if (faxineiro.possui_arma) arma_mop.no_chao = false;
                                estado_jogo = ESTADO_JOGANDO;
                            }
                        }
                    } else if (opcao_menu == 2) { 
                        estado_jogo = ESTADO_RECORDES;
                    } else if (opcao_menu == 3) { 
                        estado_jogo = ESTADO_CONFIGURACOES;
                        opcao_config = 0;
                    }
                }
            }
            else if (estado_jogo == ESTADO_CONFIRMAR_NOVO_JOGO) {
                if (evento.keyboard.keycode == ALLEGRO_KEY_LEFT || evento.keyboard.keycode == ALLEGRO_KEY_RIGHT ||
                    evento.keyboard.keycode == ALLEGRO_KEY_A || evento.keyboard.keycode == ALLEGRO_KEY_D) {
                    opcao_confirma = 1 - opcao_confirma; 
                }
                else if (evento.keyboard.keycode == ALLEGRO_KEY_ENTER || evento.keyboard.keycode == ALLEGRO_KEY_SPACE) {
                    if (opcao_confirma == 0) { 
                        strcpy(nome_jogador, "");
                        estado_jogo = ESTADO_DIGITAR_NOME;
                    } else { 
                        estado_jogo = ESTADO_MENU_PRINCIPAL;
                    }
                }
                else if (evento.keyboard.keycode == ALLEGRO_KEY_ESCAPE) {
                    estado_jogo = ESTADO_MENU_PRINCIPAL;
                }
            }
            else if (estado_jogo == ESTADO_RECORDES) {
                if (evento.keyboard.keycode == ALLEGRO_KEY_ESCAPE || evento.keyboard.keycode == ALLEGRO_KEY_ENTER) {
                    estado_jogo = ESTADO_MENU_PRINCIPAL;
                }
            }
            else if (estado_jogo == ESTADO_CONFIGURACOES) {
                if (evento.keyboard.keycode == ALLEGRO_KEY_ESCAPE) {
                    estado_jogo = ESTADO_MENU_PRINCIPAL;
                }
                else if (evento.keyboard.keycode == ALLEGRO_KEY_DOWN || evento.keyboard.keycode == ALLEGRO_KEY_S) {
                    opcao_config = (opcao_config + 1) % (TOTAL_SLOTS + 2); 
                }
                else if (evento.keyboard.keycode == ALLEGRO_KEY_UP || evento.keyboard.keycode == ALLEGRO_KEY_W) {
                    opcao_config = (opcao_config - 1 + (TOTAL_SLOTS + 2)) % (TOTAL_SLOTS + 2);
                }
                else if (evento.keyboard.keycode == ALLEGRO_KEY_ENTER || evento.keyboard.keycode == ALLEGRO_KEY_SPACE) {
                    if (opcao_config < TOTAL_SLOTS) {
                        slot_sendo_remapeado = opcao_config; 
                    } else if (opcao_config == TOTAL_SLOTS) { 
                        carregar_controles_padrao(&controles);
                    } else if (opcao_config == TOTAL_SLOTS + 1) { 
                        estado_jogo = ESTADO_MENU_PRINCIPAL;
                    }
                }
            }
            else if (estado_jogo == ESTADO_JOGANDO) {
                if (evento.keyboard.keycode == ALLEGRO_KEY_ESCAPE) {
                    estado_jogo = ESTADO_PAUSADO;
                    opcao_pause = 0;
                }
                else if (evento.keyboard.keycode == controles.tecla_pular) input.pulo = true; 
                else if (evento.keyboard.keycode == controles.tecla_esquerda) input.eixo_x = -1.0; 
                else if (evento.keyboard.keycode == controles.tecla_direita) input.eixo_x =  1.0; 
                else if (evento.keyboard.keycode == controles.tecla_pegar) input.acao_pegar = true;  
                else if (evento.keyboard.keycode == controles.tecla_soltar) input.acao_soltar = true; 
                else if (evento.keyboard.keycode == controles.tecla_entrar) input.acao_entrar = true; 
                else if (evento.keyboard.keycode == ALLEGRO_KEY_F3) input.f3_pressionado = true;
                else if (evento.keyboard.keycode == ALLEGRO_KEY_B) {
                    if (input.f3_pressionado) mostrar_hitbox = !mostrar_hitbox; 
                }
                else if (evento.keyboard.keycode == ALLEGRO_KEY_S) input.acao_descer = true;
            }
            else if (estado_jogo == ESTADO_PAUSADO) {
                if (evento.keyboard.keycode == ALLEGRO_KEY_ESCAPE || evento.keyboard.keycode == controles.tecla_pular) {
                    estado_jogo = ESTADO_JOGANDO;
                }
                else if (evento.keyboard.keycode == ALLEGRO_KEY_DOWN || evento.keyboard.keycode == ALLEGRO_KEY_S) {
                    opcao_pause = (opcao_pause + 1) % 3;
                }
                else if (evento.keyboard.keycode == ALLEGRO_KEY_UP || evento.keyboard.keycode == ALLEGRO_KEY_W) {
                    opcao_pause = (opcao_pause - 1 + 3) % 3;
                }
                else if (evento.keyboard.keycode == ALLEGRO_KEY_ENTER || evento.keyboard.keycode == ALLEGRO_KEY_SPACE) {
                    if (opcao_pause == 0) { 
                        estado_jogo = ESTADO_JOGANDO;
                    } else if (opcao_pause == 1) { 
                        estado_jogo = ESTADO_MENU_PRINCIPAL;
                    } else if (opcao_pause == 2) { 
                        rodando = false;
                    }
                }
            }
            redesenhar = true;
        } 
        else if (evento.type == ALLEGRO_EVENT_KEY_UP) {
            if (estado_jogo == ESTADO_JOGANDO) {
                if (evento.keyboard.keycode == controles.tecla_pular) input.pulo = false;
                else if (evento.keyboard.keycode == controles.tecla_esquerda && input.eixo_x < 0) input.eixo_x = 0.0;
                else if (evento.keyboard.keycode == controles.tecla_direita && input.eixo_x > 0) input.eixo_x = 0.0;
                else if (evento.keyboard.keycode == controles.tecla_pegar) input.acao_pegar = false;
                else if (evento.keyboard.keycode == controles.tecla_soltar) input.acao_soltar = false;
                else if (evento.keyboard.keycode == controles.tecla_entrar) input.acao_entrar = false; 
                else if (evento.keyboard.keycode == ALLEGRO_KEY_F3) input.f3_pressionado = false;
                else if (evento.keyboard.keycode == ALLEGRO_KEY_S) input.acao_descer = false;
            }
        }
        else if (evento.type == ALLEGRO_EVENT_MOUSE_AXES) {
            float mapa_mouse_x = evento.mouse.x * ((float)LARGURA_MAPA / LARGURA_JANELA);
            float mapa_mouse_y = evento.mouse.y * ((float)ALTURA_MAPA / ALTURA_JANELA);
            bool sobre_elemento = false;

            if (estado_jogo == ESTADO_MENU_PRINCIPAL) {
                for (int i = 0; i < 4; i++) {
                    float item_y = 90 + (i * 25);
                    if (mapa_mouse_x >= 100 && mapa_mouse_x <= 380 && mapa_mouse_y >= item_y && mapa_mouse_y <= item_y + 20) {
                        opcao_menu = i;
                        sobre_elemento = true;
                        break;
                    }
                }
            }
            else if (estado_jogo == ESTADO_CONFIRMAR_NOVO_JOGO) {
                if (mapa_mouse_y >= 150 && mapa_mouse_y <= 175) {
                    if (mapa_mouse_x >= 160 && mapa_mouse_x <= 220) { opcao_confirma = 0; sobre_elemento = true; }
                    else if (mapa_mouse_x >= 260 && mapa_mouse_x <= 320) { opcao_confirma = 1; sobre_elemento = true; }
                }
            }
            else if (estado_jogo == ESTADO_CONFIGURACOES) {
                for (int i = 0; i < TOTAL_SLOTS + 2; i++) {
                    float item_y = 50 + (i * 22) + (i >= TOTAL_SLOTS ? 10 : 0);
                    if (i == TOTAL_SLOTS + 1) item_y += 10;
                    if (mapa_mouse_x >= 100 && mapa_mouse_x <= 380 && mapa_mouse_y >= item_y && mapa_mouse_y <= item_y + 20) {
                        opcao_config = i;
                        sobre_elemento = true;
                        break;
                    }
                }
            }
            else if (estado_jogo == ESTADO_PAUSADO) {
                for (int i = 0; i < 3; i++) {
                    float item_y = 110 + (i * 25);
                    if (mapa_mouse_x >= 100 && mapa_mouse_x <= 380 && mapa_mouse_y >= item_y && mapa_mouse_y <= item_y + 20) {
                        opcao_pause = i;
                        sobre_elemento = true;
                        break;
                    }
                }
            }

            if (sobre_elemento) {
                al_set_system_mouse_cursor(tela, ALLEGRO_SYSTEM_MOUSE_CURSOR_LINK);
            } else {
                al_set_system_mouse_cursor(tela, ALLEGRO_SYSTEM_MOUSE_CURSOR_DEFAULT);
            }
            redesenhar = true;
        }
        else if (evento.type == ALLEGRO_EVENT_MOUSE_BUTTON_DOWN) {
            float mapa_mouse_x = evento.mouse.x * ((float)LARGURA_MAPA / LARGURA_JANELA);
            float mapa_mouse_y = evento.mouse.y * ((float)ALTURA_MAPA / ALTURA_JANELA);

            if (estado_jogo == ESTADO_JOGANDO && evento.mouse.button == 1) {
                input.atirando = true; 
            }
            else if (estado_jogo == ESTADO_MENU_PRINCIPAL && evento.mouse.button == 1) {
                for (int i = 0; i < 4; i++) {
                    float item_y = 90 + (i * 25);
                    if (mapa_mouse_x >= 100 && mapa_mouse_x <= 380 && mapa_mouse_y >= item_y && mapa_mouse_y <= item_y + 20) {
                        opcao_menu = i;
                        if (opcao_menu == 0) { 
                            FILE *f = fopen("save.dat", "r");
                            if (f) {
                                fclose(f);
                                estado_jogo = ESTADO_CONFIRMAR_NOVO_JOGO; 
                                opcao_confirma = 1;
                            } else {
                                strcpy(nome_jogador, "");
                                estado_jogo = ESTADO_DIGITAR_NOME;
                            }
                        } else if (opcao_menu == 1) { 
                            FILE *f_chk = fopen("save.dat", "r");
                            if (f_chk) {
                                fclose(f_chk);
                                if (carregar_jogo(nome_jogador, &sala_atual, &faxineiro.x, &faxineiro.y, &faxineiro.possui_arma)) {
                                    if (faxineiro.possui_arma) arma_mop.no_chao = false;
                                    estado_jogo = ESTADO_JOGANDO;
                                }
                            }
                        } else if (opcao_menu == 2) { 
                            estado_jogo = ESTADO_RECORDES;
                        } else if (opcao_menu == 3) { 
                            estado_jogo = ESTADO_CONFIGURACOES;
                            opcao_config = 0;
                        }
                        break;
                    }
                }
            }
            else if (estado_jogo == ESTADO_CONFIRMAR_NOVO_JOGO && evento.mouse.button == 1) {
                if (mapa_mouse_y >= 150 && mapa_mouse_y <= 175) {
                    if (mapa_mouse_x >= 160 && mapa_mouse_x <= 220) { 
                        strcpy(nome_jogador, "");
                        estado_jogo = ESTADO_DIGITAR_NOME;
                    }
                    else if (mapa_mouse_x >= 260 && mapa_mouse_x <= 320) { 
                        estado_jogo = ESTADO_MENU_PRINCIPAL;
                    }
                }
            }
            else if (estado_jogo == ESTADO_RECORDES && evento.mouse.button == 1) {
                estado_jogo = ESTADO_MENU_PRINCIPAL;
            }
            else if (estado_jogo == ESTADO_CONFIGURACOES && evento.mouse.button == 1) {
                for (int i = 0; i < TOTAL_SLOTS + 2; i++) {
                    float item_y = 50 + (i * 22) + (i >= TOTAL_SLOTS ? 10 : 0);
                    if (i == TOTAL_SLOTS + 1) item_y += 10;
                    if (mapa_mouse_x >= 100 && mapa_mouse_x <= 380 && mapa_mouse_y >= item_y && mapa_mouse_y <= item_y + 20) {
                        opcao_config = i;
                        if (opcao_config < TOTAL_SLOTS) {
                            slot_sendo_remapeado = opcao_config; 
                        } else if (opcao_config == TOTAL_SLOTS) { 
                            carregar_controles_padrao(&controles);
                        } else if (opcao_config == TOTAL_SLOTS + 1) { 
                            estado_jogo = ESTADO_MENU_PRINCIPAL;
                        }
                        break;
                    }
                }
            }
            else if (estado_jogo == ESTADO_PAUSADO && evento.mouse.button == 1) {
                for (int i = 0; i < 3; i++) {
                    float item_y = 110 + (i * 25);
                    if (mapa_mouse_x >= 100 && mapa_mouse_x <= 380 && mapa_mouse_y >= item_y && mapa_mouse_y <= item_y + 20) {
                        opcao_pause = i;
                        if (opcao_pause == 0) { 
                            estado_jogo = ESTADO_JOGANDO;
                        } else if (opcao_pause == 1) { 
                            estado_jogo = ESTADO_MENU_PRINCIPAL;
                        } else if (opcao_pause == 2) { 
                            rodando = false;
                        }
                        break;
                    }
                }
            }
            redesenhar = true;
        }
        else if (evento.type == ALLEGRO_EVENT_MOUSE_BUTTON_UP) {
            if (estado_jogo == ESTADO_JOGANDO && evento.mouse.button == 1) input.atirando = false;
        }

        else if (evento.type == ALLEGRO_EVENT_TIMER) {
            redesenhar = true;

            if (estado_jogo == ESTADO_JOGANDO) {
                
                anomalia.recebendo_calor = false; 

                if (sala_atual == 1) {
                    float player_left = faxineiro.x - faxineiro.w / 2.0;
                    float player_right = faxineiro.x + faxineiro.w / 2.0;
                    float player_top = faxineiro.y - faxineiro.h / 2.0;
                    float player_bottom = faxineiro.y + faxineiro.h / 2.0;

                    float prop_x = 322.0;
                    float prop_y = 161.0;
                    float prop_w = 73.0;
                    float prop_h = 42.0;

                    bool encostando_prop = (player_right > prop_x) && (player_left < prop_x + prop_w) &&
                                           (player_bottom > prop_y) && (player_top < prop_y + prop_h);

                    if (!encostando_prop) {
                        prop_na_frente = (player_bottom < 200.0);
                    }
                }

                if (estado_fade == 1) { 
                    alpha_fade += 15;
                    if (alpha_fade >= 255) { alpha_fade = 255; estado_fade = 2; timer_fade = 0; }
                } 
                else if (estado_fade == 2) { 
                    timer_fade++;
                    if (timer_fade == 30) {
                        sala_atual = proxima_sala;
                        faxineiro.x = proximo_x; faxineiro.y = proximo_chao_y - (faxineiro.h / 2.0); faxineiro.vel_y = 0; faxineiro.no_chao = true;
                        salvar_jogo(nome_jogador, sala_atual, faxineiro.x, faxineiro.y, faxineiro.possui_arma);
                    }
                    if (timer_fade >= 60) estado_fade = 3;
                }
                else if (estado_fade == 3) { 
                    alpha_fade -= 15;
                    if (alpha_fade <= 0) { alpha_fade = 0; estado_fade = 0; }
                }

                if (arma_mop.no_chao && arma_mop.sala_origem == sala_atual) {
                    arma_mop.vel_y += GRAVIDADE;
                    arma_mop.y += arma_mop.vel_y;
                    float largura_mop_sprite = arma_mop.sprite ? al_get_bitmap_width(arma_mop.sprite) : 10.0;
                    for (int p = 0; p < TOTAL_PLATAFORMAS; p++) {
                        if (mundo_plataformas[p].sala != sala_atual) continue;
                        Plataforma plat = mundo_plataformas[p];
                        if ((arma_mop.x + largura_mop_sprite/2 > plat.x) && (arma_mop.x - largura_mop_sprite/2 < plat.x + plat.w) && 
                            (arma_mop.y >= plat.y) && (arma_mop.y - arma_mop.vel_y <= plat.y + 5.0)) {
                            arma_mop.y = plat.y; arma_mop.vel_y = 0; break;
                        }
                    }
                }

                float limite_anomalia = (anomalia.sala_origem == 0) ? (LARGURA_MAPA / 2.0) : LARGURA_MAPA;
                
                bool anomalia_quer_descer = false;
                if (anomalia.timer_agro > 0 && faxineiro.y > anomalia.y + 15.0) {
                    anomalia_quer_descer = true;
                }

                if (sala_atual == anomalia.sala_origem && estado_fade == 0) {
                    float abs_dist_x = fabs(faxineiro.x - anomalia.x);
                    
                    float player_top = (faxineiro.y + faxineiro.h / 2.0) - faxineiro.body_h;
                    float player_bottom = faxineiro.y + faxineiro.h / 2.0;
                    float anomalia_top = anomalia.y - anomalia.h / 2.0;
                    float anomalia_bottom = anomalia.y + anomalia.h / 2.0;
                    
                    bool toque_vertical = (player_bottom >= anomalia_top) && (player_top <= anomalia_bottom);

                    if (abs_dist_x <= anomalia.raio_visao && toque_vertical) {
                        anomalia.timer_agro = 300; 
                        anomalia.alvo_x = faxineiro.x; 
                    }
                }

                if (anomalia.timer_agro > 0) {
                    anomalia.timer_agro--;
                    if (anomalia.x < anomalia.alvo_x - anomalia.velocidade) {
                        anomalia.vel_x_atual = anomalia.velocidade;
                        anomalia.direcao_atual = DIR_DIREITA;
                    } else if (anomalia.x > anomalia.alvo_x + anomalia.velocidade) {
                        anomalia.vel_x_atual = -anomalia.velocidade;
                        anomalia.direcao_atual = DIR_ESQUERDA;
                    } else {
                        anomalia.vel_x_atual = 0;
                    }

                    if (anomalia.timer_agro <= 0) {
                        anomalia.raio_visao = 180.0; 
                    }
                } else {
                    if (anomalia.timer_patrulha <= 0) {
                        anomalia.timer_patrulha = 60 + (rand() % 120); 
                        int acao = rand() % 3; 
                        if (acao == 0) {
                            anomalia.vel_x_atual = 0;
                        } else if (acao == 1) { 
                            anomalia.vel_x_atual = -(anomalia.velocidade * 0.5); 
                            anomalia.direcao_atual = DIR_ESQUERDA; 
                        } else { 
                            anomalia.vel_x_atual = (anomalia.velocidade * 0.5); 
                            anomalia.direcao_atual = DIR_DIREITA; 
                        }
                    } else {
                        anomalia.timer_patrulha--;
                    }
                }

                anomalia.vel_y += GRAVIDADE; 
                anomalia.y += anomalia.vel_y; 
                anomalia.no_chao = false;

                for (int p = 0; p < TOTAL_PLATAFORMAS; p++) {
                    if (mundo_plataformas[p].sala != anomalia.sala_origem) continue; 
                    Plataforma plat = mundo_plataformas[p];
                    
                    if (anomalia_quer_descer && plat.atravessavel) continue;

                    if ((anomalia.x + anomalia.w/2 > plat.x) && (anomalia.x - anomalia.w/2 < plat.x + plat.w) && 
                        (anomalia.y + anomalia.h/2 > plat.y) && (anomalia.y - anomalia.h/2 < plat.y + plat.h)) {
                        float pe_anterior = (anomalia.y - anomalia.vel_y) + anomalia.h/2;
                        if (anomalia.vel_y >= 0 && pe_anterior <= plat.y + 0.1) {
                            anomalia.y = plat.y - anomalia.h/2; 
                            anomalia.vel_y = 0; 
                            anomalia.no_chao = true;
                        }
                    }
                }

                for (int e = 0; e < TOTAL_ESCADAS; e++) {
                    if (mundo_escadas[e].sala != anomalia.sala_origem) continue;
                    Escada esc = mundo_escadas[e];
                    
                    bool colide_x = (anomalia.x + anomalia.w/2.0 >= esc.x) && (anomalia.x - anomalia.w/2.0 <= esc.x + esc.w);
                    if (colide_x) {
                        float calc_x = anomalia.x;
                        if (calc_x < esc.x) calc_x = esc.x;
                        if (calc_x > esc.x + esc.w) calc_x = esc.x + esc.w;
                        
                        float progresso = (calc_x - esc.x) / esc.w;
                        float chao_rampa_y = (esc.direcao == 1) ? (esc.y + esc.h - (progresso * esc.h)) : (esc.y + (progresso * esc.h));
                        
                        float pe_anomalia = anomalia.y + anomalia.h / 2.0;
                        float pe_anterior = (anomalia.y - anomalia.vel_y) + anomalia.h / 2.0;
                        
                        bool descendo_rampa = (esc.direcao == 1 && anomalia.vel_x_atual < 0) || (esc.direcao == -1 && anomalia.vel_x_atual > 0);
                        bool no_topo = (pe_anterior <= esc.y + 8.0);

                        if (no_topo && descendo_rampa && !anomalia_quer_descer) {
                            continue; 
                        }

                        if (anomalia.vel_y >= 0 && pe_anomalia >= chao_rampa_y - 15.0 && pe_anterior <= chao_rampa_y + 15.0) {
                            anomalia.y = chao_rampa_y - anomalia.h / 2.0;
                            anomalia.vel_y = 0;
                            anomalia.no_chao = true;
                        }
                    }
                }

                if (anomalia.vel_x_atual != 0) {
                    float futuro_x = anomalia.x + anomalia.vel_x_atual;
                    bool bateu_parede = (futuro_x - anomalia.w/2 < 0 || futuro_x + anomalia.w/2 > limite_anomalia);
                    
                    for (int e = 0; e < TOTAL_ESCADAS; e++) {
                        if (mundo_escadas[e].sala != anomalia.sala_origem) continue;
                        Escada esc = mundo_escadas[e];
                        float pe_anomalia = anomalia.y + anomalia.h/2.0;
                        
                        if (pe_anomalia > esc.y + 4.0 && pe_anomalia - anomalia.h < esc.y + esc.h) {
                            if (esc.direcao == 1 && anomalia.vel_x_atual < 0 && anomalia.x - anomalia.w/2.0 >= esc.x + esc.w - 5.0 && futuro_x - anomalia.w/2.0 < esc.x + esc.w) {
                                bateu_parede = true;
                            }
                            else if (esc.direcao == -1 && anomalia.vel_x_atual > 0 && anomalia.x + anomalia.w/2.0 <= esc.x + 5.0 && futuro_x + anomalia.w/2.0 > esc.x) {
                                bateu_parede = true;
                            }
                        }
                    }

                    bool chao_frente = false;
                    if (anomalia.no_chao) {
                        float borda_x = anomalia.x + (anomalia.vel_x_atual > 0 ? anomalia.w/2 : -anomalia.w/2) + anomalia.vel_x_atual;
                        float base_y = anomalia.y + anomalia.h/2.0;
                        
                        for (int p = 0; p < TOTAL_PLATAFORMAS; p++) {
                            if (mundo_plataformas[p].sala != anomalia.sala_origem) continue;
                            Plataforma plat = mundo_plataformas[p];
                            if (borda_x >= plat.x && borda_x <= plat.x + plat.w) {
                                if (fabs(base_y - plat.y) <= 45.0) { 
                                    chao_frente = true; break; 
                                }
                            }
                        }

                        if (!chao_frente) {
                            for (int e = 0; e < TOTAL_ESCADAS; e++) {
                                if (mundo_escadas[e].sala != anomalia.sala_origem) continue;
                                Escada esc = mundo_escadas[e];
                                if (borda_x >= esc.x - 5.0 && borda_x <= esc.x + esc.w + 5.0) {
                                    float calc_x = borda_x;
                                    if (calc_x < esc.x) calc_x = esc.x;
                                    if (calc_x > esc.x + esc.w) calc_x = esc.x + esc.w;
                                    float progresso = (calc_x - esc.x) / esc.w;
                                    float chao_rampa_y = (esc.direcao == 1) ? (esc.y + esc.h - (progresso * esc.h)) : (esc.y + (progresso * esc.h));
                                    if (fabs(base_y - chao_rampa_y) <= 45.0) { 
                                        chao_frente = true; break;
                                    }
                                }
                            }
                        }
                    } else {
                        chao_frente = true; 
                    }

                    if (bateu_parede || (anomalia.no_chao && !chao_frente && !anomalia_quer_descer)) {
                        if (anomalia.timer_agro <= 0) {
                            anomalia.vel_x_atual = -anomalia.vel_x_atual;
                            anomalia.direcao_atual = (anomalia.vel_x_atual > 0) ? DIR_DIREITA : DIR_ESQUERDA;
                            anomalia.timer_patrulha = 60; 
                        } else {
                            anomalia.vel_x_atual = 0; 
                        }
                    } else {
                        anomalia.x = futuro_x;
                    }
                }

                if (estado_fade == 0) {
                    
                    for (int i = 0; i < TOTAL_PORTAS; i++) {
                        if (mundo_portas[i].sala_origem != sala_atual) continue; 
                        
                        bool prox_x = (faxineiro.x + faxineiro.w/2 > mundo_portas[i].x - 15) && (faxineiro.x - faxineiro.w/2 < mundo_portas[i].x + mundo_portas[i].w + 15);
                        
                        float chao_porta = mundo_portas[i].y + mundo_portas[i].h;
                        float pe_jogador = faxineiro.y + faxineiro.h/2.0;
                        bool mesmo_nivel = (fabs(pe_jogador - chao_porta) <= 5.0);

                        if (prox_x && mesmo_nivel) {
                            mundo_portas[i].jogador_perto = true;
                            if (mundo_portas[i].timer_animacao < 30) mundo_portas[i].timer_animacao++;
                        } else {
                            mundo_portas[i].jogador_perto = false;
                            if (mundo_portas[i].timer_animacao > 0) mundo_portas[i].timer_animacao--;
                        }
                    }

                    if (input.acao_entrar) {
                        for (int i = 0; i < TOTAL_PORTAS; i++) {
                            Porta p = mundo_portas[i];
                            if (p.sala_origem != sala_atual) continue; 

                            float chao_porta = p.y + p.h;
                            float pe_jogador = faxineiro.y + faxineiro.h/2.0;

                            if (faxineiro.x > p.x && faxineiro.x < p.x + p.w && fabs(pe_jogador - chao_porta) <= 5.0) {
                                estado_fade = 1; 
                                proxima_sala = p.destino_sala; 
                                
                                for (int j = 0; j < TOTAL_PORTAS; j++) {
                                    if (mundo_portas[j].sala_origem == p.destino_sala && mundo_portas[j].destino_sala == sala_atual) {
                                        proximo_x = mundo_portas[j].x + (mundo_portas[j].w / 2.0);
                                        proximo_chao_y = mundo_portas[j].y + mundo_portas[j].h;
                                        break;
                                    }
                                }

                                input.acao_entrar = false; 
                                if (sala_atual == anomalia.sala_origem) {
                                    float abs_dist_x = fabs(faxineiro.x - anomalia.x);
                                    
                                    float player_top = (faxineiro.y + faxineiro.h / 2.0) - faxineiro.body_h;
                                    float player_bottom = faxineiro.y + faxineiro.h / 2.0;
                                    float anomalia_top = anomalia.y - anomalia.h / 2.0;
                                    float anomalia_bottom = anomalia.y + anomalia.h / 2.0;
                                    
                                    bool toque_vertical = (player_bottom >= anomalia_top) && (player_top <= anomalia_bottom);

                                    if (abs_dist_x <= anomalia.raio_visao && toque_vertical) {
                                        anomalia.timer_agro = 300; anomalia.alvo_x = p.x + p.w/2; 
                                    }
                                }
                                break;
                            }
                        }
                    }

                    bool preso_na_poca = false;
                    if (sala_atual == anomalia.sala_origem && anomalia.estado_atual == LIQUIDO) {
                        bool col_poca_x = (faxineiro.x + faxineiro.w/2 > anomalia.x - anomalia.w/2) && (faxineiro.x - faxineiro.w/2 < anomalia.x + anomalia.w/2);
                        bool col_poca_y = (faxineiro.y + faxineiro.h/2 > anomalia.y - anomalia.h/2) && (faxineiro.y - faxineiro.h/2 < anomalia.y + anomalia.h/2);
                        if (col_poca_x && col_poca_y) preso_na_poca = true;
                    }

                    float velocidade_atual = preso_na_poca ? (faxineiro.velocidade * 0.4) : faxineiro.velocidade;
                    float vel_x_aplicada = input.eixo_x * velocidade_atual;
                    float futuro_x = faxineiro.x + vel_x_aplicada;

                    for (int e = 0; e < TOTAL_ESCADAS; e++) {
                        if (mundo_escadas[e].sala != sala_atual) continue;
                        Escada esc = mundo_escadas[e];
                        float pe_jogador = faxineiro.y + faxineiro.h/2.0;
                        
                        if (pe_jogador > esc.y + 4.0 && pe_jogador - faxineiro.h < esc.y + esc.h) {
                            if (esc.direcao == 1) { 
                                if (vel_x_aplicada < 0 && faxineiro.x - faxineiro.w/2.0 >= esc.x + esc.w - 5.0 && futuro_x - faxineiro.w/2.0 < esc.x + esc.w) {
                                    futuro_x = esc.x + esc.w + faxineiro.w/2.0;
                                }
                            } else if (esc.direcao == -1) { 
                                if (vel_x_aplicada > 0 && faxineiro.x + faxineiro.w/2.0 <= esc.x + 5.0 && futuro_x + faxineiro.w/2.0 > esc.x) {
                                    futuro_x = esc.x - faxineiro.w/2.0;
                                }
                            }
                        }
                    }

                    faxineiro.x = futuro_x;
                    if (input.eixo_x > 0) faxineiro.direcao_atual = DIR_DIREITA; else if (input.eixo_x < 0) faxineiro.direcao_atual = DIR_ESQUERDA;

                    if (faxineiro.x - faxineiro.body_w/2 < 0) faxineiro.x = faxineiro.body_w/2;
                    if (faxineiro.x + faxineiro.body_w/2 > limite_mapa_x) faxineiro.x = limite_mapa_x - faxineiro.body_w/2;

                    faxineiro.vel_y += GRAVIDADE; faxineiro.y += faxineiro.vel_y; faxineiro.no_chao = false;

                    for (int p = 0; p < TOTAL_PLATAFORMAS; p++) {
                        if (mundo_plataformas[p].sala != sala_atual) continue;
                        Plataforma plat = mundo_plataformas[p];
                        
                        if (input.acao_descer && plat.atravessavel) continue;

                        bool colide_x = (faxineiro.x + faxineiro.w/2 > plat.x) && (faxineiro.x - faxineiro.w/2 < plat.x + plat.w);
                        bool colide_y = (faxineiro.y + faxineiro.h/2 > plat.y) && (faxineiro.y - faxineiro.h/2 < plat.y + plat.h);

                        if (colide_x && colide_y) {
                            float pe_anterior = (faxineiro.y - faxineiro.vel_y) + faxineiro.h/2;
                            if (faxineiro.vel_y >= 0 && pe_anterior <= plat.y + 0.1) {
                                faxineiro.y = plat.y - faxineiro.h/2; faxineiro.vel_y = 0; faxineiro.no_chao = true;
                            }
                        }
                    }

                    for (int e = 0; e < TOTAL_ESCADAS; e++) {
                        if (mundo_escadas[e].sala != sala_atual) continue;
                        Escada esc = mundo_escadas[e];
                        
                        bool colide_x = (faxineiro.x + faxineiro.w/2.0 >= esc.x) && (faxineiro.x - faxineiro.w/2.0 <= esc.x + esc.w);
                        if (colide_x) {
                            float calc_x = faxineiro.x;
                            if (calc_x < esc.x) calc_x = esc.x;
                            if (calc_x > esc.x + esc.w) calc_x = esc.x + esc.w;
                            
                            float progresso = (calc_x - esc.x) / esc.w;
                            float chao_rampa_y = (esc.direcao == 1) ? (esc.y + esc.h - (progresso * esc.h)) : (esc.y + (progresso * esc.h));
                            
                            float pe_jogador = faxineiro.y + faxineiro.h / 2.0;
                            float pe_anterior = (faxineiro.y - faxineiro.vel_y) + faxineiro.h / 2.0;
                            
                            bool descendo_rampa = (esc.direcao == 1 && input.eixo_x < 0) || (esc.direcao == -1 && input.eixo_x > 0);
                            bool no_topo = (pe_anterior <= esc.y + 8.0);

                            if (no_topo && descendo_rampa && !input.acao_descer) {
                                continue; 
                            }
                            
                            if (faxineiro.vel_y >= 0 && pe_jogador >= chao_rampa_y - 15.0 && pe_anterior <= chao_rampa_y + 15.0) {
                                faxineiro.y = chao_rampa_y - faxineiro.h / 2.0;
                                faxineiro.vel_y = 0;
                                faxineiro.no_chao = true;
                            }
                        }
                    }

                    if (sala_atual == anomalia.sala_origem) {
                        bool colide_inimigo_x = (faxineiro.x + faxineiro.w/2 > anomalia.x - anomalia.w/2) && (faxineiro.x - faxineiro.w/2 < anomalia.x + anomalia.w/2);
                        bool colide_inimigo_y = (faxineiro.y + faxineiro.h/2 > anomalia.y - anomalia.h/2) && (faxineiro.y - faxineiro.h/2 < anomalia.y + anomalia.h/2);
                        
                        if (colide_inimigo_x && colide_inimigo_y) {
                            anomalia.recebendo_calor = true; 
                            float pe_anterior = (faxineiro.y - faxineiro.vel_y) + faxineiro.h/2;
                            float topo_inimigo = anomalia.y - anomalia.h/2;
                            if (faxineiro.vel_y >= 0 && pe_anterior <= topo_inimigo + 0.1 && anomalia.estado_atual == SOLIDO) {
                                faxineiro.y = topo_inimigo - faxineiro.h/2; faxineiro.vel_y = 0; faxineiro.no_chao = true;
                            }
                        }
                    }

                    if (input.pulo && faxineiro.no_chao) { faxineiro.vel_y = preso_na_poca ? (FORCA_PULO * 0.7) : FORCA_PULO; faxineiro.no_chao = false; }

                    bool movendo_agora = (input.eixo_x != 0); 
                    if (movendo_agora != faxineiro.esta_andando) { faxineiro.esta_andando = movendo_agora; faxineiro.frame_atual = 0; faxineiro.tempo_animacao = 0; }
                    faxineiro.tempo_animacao++;
                    int limite_tempo = faxineiro.esta_andando ? 10 : 30; 
                    if (faxineiro.tempo_animacao >= limite_tempo) {
                        faxineiro.tempo_animacao = 0; faxineiro.frame_atual++;      
                        if (faxineiro.esta_andando) { if (faxineiro.frame_atual > 3) faxineiro.frame_atual = 0; } 
                        else { if (faxineiro.frame_atual > 1) faxineiro.frame_atual = 0; }
                    }

                    if (arma_mop.no_chao && arma_mop.sala_origem == sala_atual) {
                        float dx_item = faxineiro.x - arma_mop.x; float dy_item = faxineiro.y - arma_mop.y;
                        if (input.acao_pegar && !faxineiro.possui_arma) {
                            if ((dx_item * dx_item) + (dy_item * dy_item) <= (arma_mop.raio_interacao * arma_mop.raio_interacao)) {
                                arma_mop.no_chao = false; faxineiro.possui_arma = true;   
                            }
                        }
                    }
                    if (input.acao_soltar && faxineiro.possui_arma && !arma_mop.no_chao) {
                        arma_mop.no_chao = true; arma_mop.x = faxineiro.x; arma_mop.y = faxineiro.y; arma_mop.vel_y = 0.0; arma_mop.sala_origem = sala_atual; faxineiro.possui_arma = false;  
                    }

                    if (faxineiro.cooldown > 0) faxineiro.cooldown--;

                    if (input.atirando && faxineiro.cooldown == 0 && faxineiro.possui_arma) {
                        for (int i = 0; i < MAX_PROJETEIS; i++) {
                            if (!projeteis[i].ativo) {
                                projeteis[i].ativo = true;
                                projeteis[i].tipo = PROJETIL_AGUA; 

                                float ponta_x = faxineiro.x; float ponta_y = faxineiro.y - (faxineiro.body_h / 2.0) + 4; 
                                if (faxineiro.direcao_atual == DIR_DIREITA) ponta_x -= 6; else ponta_x += 6;

                                int tipo_frame_atual = faxineiro.esta_andando ? (faxineiro.frame_atual == 0 || faxineiro.frame_atual == 2 ? FRAME_WALK1 : FRAME_IDLE) : (faxineiro.frame_atual == 0 ? FRAME_IDLE : FRAME_BRE);
                                if (tipo_frame_atual == FRAME_BRE) ponta_y += 1; else if (tipo_frame_atual == FRAME_WALK1) ponta_y -= 2; 
                                
                                float tamanho_cabo = 16.0; 
                                if (faxineiro.direcao_atual == DIR_ESQUERDA) { projeteis[i].vel_x = -VELOCIDADE_PROJETIL; ponta_x -= tamanho_cabo; } 
                                else { projeteis[i].vel_x = VELOCIDADE_PROJETIL; ponta_x += tamanho_cabo; }

                                projeteis[i].x = ponta_x; projeteis[i].y = ponta_y; faxineiro.cooldown = COOLDOWN_TIRO; break; 
                            }
                        }
                    }

                    for (int i = 0; i < MAX_PROJETEIS; i++) {
                        if (projeteis[i].ativo) {
                            projeteis[i].x += projeteis[i].vel_x;
                            if (sala_atual == anomalia.sala_origem) {
                                bool acertou_monstro_x = (projeteis[i].x > anomalia.x - anomalia.w/2) && (projeteis[i].x < anomalia.x + anomalia.w/2);
                                bool acertou_monstro_y = (projeteis[i].y > anomalia.y - anomalia.h/2) && (projeteis[i].y < anomalia.y + anomalia.h/2);

                                if (acertou_monstro_x && acertou_monstro_y) {
                                    projeteis[i].ativo = false;      
                                    
                                    if (projeteis[i].tipo == PROJETIL_AGUA) {
                                        anomalia.timer_resfriamento = 300; 
                                    }
                                    
                                    anomalia.raio_visao = 180.0 * 5.0;
                                    anomalia.timer_agro = 300; 
                                    anomalia.alvo_x = faxineiro.x;
                                }
                            }
                            if (projeteis[i].x < 0 || projeteis[i].x > limite_mapa_x) {
                                projeteis[i].ativo = false;
                            }
                        }
                    }
                }

                DadosElemento dados_anomalia = tabela_periodica[anomalia.numero_atomico];

                if (anomalia.timer_resfriamento > 0) {
                    anomalia.timer_resfriamento--;
                }

                if (anomalia.recebendo_calor) {
                    anomalia.temperatura += (4.0 / FPS); 
                } else {
                    float taxa_resfriamento = (2.0 / FPS); 
                    
                    if (anomalia.timer_resfriamento > 0) {
                        taxa_resfriamento *= 2.0; 
                    }
                    
                    anomalia.temperatura -= taxa_resfriamento; 
                }

                if (anomalia.temperatura > TEMP_JOGADOR) anomalia.temperatura = TEMP_JOGADOR;
                if (anomalia.temperatura < TEMP_ESTACAO) anomalia.temperatura = TEMP_ESTACAO;

                if (anomalia.temperatura >= dados_anomalia.ponto_fusao && anomalia.estado_atual == SOLIDO) {
                    anomalia.estado_atual = LIQUIDO;
                } else if (anomalia.temperatura < dados_anomalia.ponto_fusao && anomalia.estado_atual == LIQUIDO) {
                    anomalia.estado_atual = SOLIDO;
                }

                if (anomalia.estado_atual == LIQUIDO) {
                    anomalia.frame_atual = 4; 
                } else {
                    float range = dados_anomalia.ponto_fusao - TEMP_ESTACAO;
                    if (range <= 0) range = 1.0; 
                    anomalia.frame_atual = (int)(((anomalia.temperatura - TEMP_ESTACAO) / range) * 4.0);
                    
                    if (anomalia.frame_atual > 3) anomalia.frame_atual = 3; 
                    if (anomalia.frame_atual < 0) anomalia.frame_atual = 0; 
                }

                anomalia.tempo_animacao++;
                if (anomalia.tempo_animacao >= 30) {
                    anomalia.tempo_animacao = 0;
                    anomalia.frame_animacao = 1 - anomalia.frame_animacao;
                }
            }
        }

        if (redesenhar && al_is_event_queue_empty(fila_eventos)) {
            redesenhar = false;
            
            ALLEGRO_TRANSFORM camera_ui; al_identity_transform(&camera_ui); al_scale_transform(&camera_ui, (float)LARGURA_JANELA / LARGURA_MAPA, (float)ALTURA_JANELA / ALTURA_MAPA);
            float largura_camera = (estado_jogo == ESTADO_JOGANDO || estado_jogo == ESTADO_PAUSADO) && (sala_atual == 0) ? (LARGURA_MAPA / 2.0) : LARGURA_MAPA;
            float altura_camera = (estado_jogo == ESTADO_JOGANDO || estado_jogo == ESTADO_PAUSADO) && (sala_atual == 0) ? (ALTURA_MAPA / 2.0) : ALTURA_MAPA;
            ALLEGRO_TRANSFORM camera_jogo; al_identity_transform(&camera_jogo); al_scale_transform(&camera_jogo, (float)LARGURA_JANELA / largura_camera, (float)ALTURA_JANELA / altura_camera);

            al_clear_to_color(al_map_rgb(30, 30, 30)); 

            if (estado_jogo == ESTADO_MENU_PRINCIPAL || estado_jogo == ESTADO_CONFIRMAR_NOVO_JOGO || estado_jogo == ESTADO_DIGITAR_NOME || estado_jogo == ESTADO_RECORDES || estado_jogo == ESTADO_CONFIGURACOES) {
                al_use_transform(&camera_ui);
                if (estado_jogo == ESTADO_MENU_PRINCIPAL) {
                    al_draw_text(fonte, al_map_rgb(255, 255, 255), LARGURA_MAPA / 2, 40, ALLEGRO_ALIGN_CENTRE, "ELEMENT 0 - GALE");
                    bool save_existe = false; FILE *f_chk = fopen("save.dat", "r"); if (f_chk) { save_existe = true; fclose(f_chk); }
                    const char *itens[] = {"Novo Jogo", "Continuar", "Recorde", "Configuracao"};
                    for (int i = 0; i < 4; i++) {
                        ALLEGRO_COLOR cor;
                        if (i == 1 && !save_existe) cor = al_map_rgb(80, 80, 80); 
                        else cor = (i == opcao_menu) ? al_map_rgb(100, 255, 100) : al_map_rgb(200, 200, 200); 
                        al_draw_text(fonte, cor, LARGURA_MAPA / 2, 90 + (i * 25), ALLEGRO_ALIGN_CENTRE, itens[i]);
                    }
                }
                else if (estado_jogo == ESTADO_CONFIRMAR_NOVO_JOGO) {
                    al_draw_text(fonte, al_map_rgb(255, 255, 255), LARGURA_MAPA / 2, 80, ALLEGRO_ALIGN_CENTRE, "O progresso salvo sera apagado,");
                    al_draw_text(fonte, al_map_rgb(255, 255, 255), LARGURA_MAPA / 2, 100, ALLEGRO_ALIGN_CENTRE, "deseja iniciar um novo jogo?");
                    ALLEGRO_COLOR cor_sim = (opcao_confirma == 0) ? al_map_rgb(100, 255, 100) : al_map_rgb(200, 200, 200);
                    al_draw_rectangle(160, 150, 220, 175, cor_sim, 1.5); al_draw_text(fonte, cor_sim, 190, 157, ALLEGRO_ALIGN_CENTRE, "SIM");
                    ALLEGRO_COLOR cor_nao = (opcao_confirma == 1) ? al_map_rgb(255, 100, 100) : al_map_rgb(200, 200, 200);
                    al_draw_rectangle(260, 150, 320, 175, cor_nao, 1.5); al_draw_text(fonte, cor_nao, 290, 157, ALLEGRO_ALIGN_CENTRE, "NAO");
                }
                else if (estado_jogo == ESTADO_DIGITAR_NOME) {
                    al_draw_text(fonte, al_map_rgb(255, 255, 255), LARGURA_MAPA / 2, 90, ALLEGRO_ALIGN_CENTRE, "DIGITE SEU NOME:");
                    al_draw_text(fonte, al_map_rgb(100, 255, 100), LARGURA_MAPA / 2, 120, ALLEGRO_ALIGN_CENTRE, nome_jogador);
                    al_draw_text(fonte, al_map_rgb(150, 150, 150), LARGURA_MAPA / 2, 160, ALLEGRO_ALIGN_CENTRE, "Pressione ENTER para confirmar");
                }
                else if (estado_jogo == ESTADO_RECORDES) {
                    al_draw_text(fonte, al_map_rgb(255, 255, 255), LARGURA_MAPA / 2, 50, ALLEGRO_ALIGN_CENTRE, "TEMPO MAIS RAPIDO AO ZERAR");
                    al_draw_text(fonte, al_map_rgb(150, 150, 150), LARGURA_MAPA / 2, 110, ALLEGRO_ALIGN_CENTRE, "[ Nenhum recorde registrado ainda ]");
                    al_draw_text(fonte, al_map_rgb(200, 200, 200), LARGURA_MAPA / 2, 180, ALLEGRO_ALIGN_CENTRE, "Clique ou ENTER para voltar");
                }
                else if (estado_jogo == ESTADO_CONFIGURACOES) {
                    al_draw_text(fonte, al_map_rgb(255, 255, 255), LARGURA_MAPA / 2, 20, ALLEGRO_ALIGN_CENTRE, "CONFIGURACAO DE CONTROLES");
                    const char *nomes_slots[TOTAL_SLOTS] = { "Pular", "Esquerda", "Direita", "Atirar", "Pegar", "Soltar", "Entrar" };
                    int teclas[TOTAL_SLOTS] = { controles.tecla_pular, controles.tecla_esquerda, controles.tecla_direita, controles.tecla_atirar, controles.tecla_pegar, controles.tecla_soltar, controles.tecla_entrar };
                    for (int i = 0; i < TOTAL_SLOTS; i++) {
                        char linha[64]; const char *nome_tecla = al_keycode_to_name(teclas[i]);
                        sprintf(linha, "%s: [%s]", nomes_slots[i], nome_tecla ? nome_tecla : "DESCONHECIDA");
                        ALLEGRO_COLOR cor = (i == opcao_config) ? al_map_rgb(100, 255, 100) : al_map_rgb(200, 200, 200);
                        if (slot_sendo_remapeado == i) { sprintf(linha, "%s: [ Pressione Nova Tecla ]", nomes_slots[i]); cor = al_map_rgb(255, 255, 0); }
                        al_draw_text(fonte, cor, LARGURA_MAPA / 2, 50 + (i * 22), ALLEGRO_ALIGN_CENTRE, linha);
                    }
                    ALLEGRO_COLOR cor_padrao = (opcao_config == TOTAL_SLOTS) ? al_map_rgb(100, 255, 100) : al_map_rgb(200, 200, 200);
                    al_draw_text(fonte, cor_padrao, LARGURA_MAPA / 2, 50 + (TOTAL_SLOTS * 22) + 10, ALLEGRO_ALIGN_CENTRE, "Configuracao Padrao");
                    ALLEGRO_COLOR cor_voltar = (opcao_config == TOTAL_SLOTS + 1) ? al_map_rgb(100, 255, 100) : al_map_rgb(200, 200, 200);
                    al_draw_text(fonte, cor_voltar, LARGURA_MAPA / 2, 50 + (TOTAL_SLOTS * 22) + 32, ALLEGRO_ALIGN_CENTRE, "Voltar ao Menu");
                }
            }
            else if (estado_jogo == ESTADO_JOGANDO || estado_jogo == ESTADO_PAUSADO) {
                al_use_transform(&camera_jogo);

                ALLEGRO_BITMAP *bg_ativo = backgrounds[sala_atual];
                if (bg_ativo) {
                    float sw = al_get_bitmap_width(bg_ativo); float sh = al_get_bitmap_height(bg_ativo);
                    al_draw_scaled_bitmap(bg_ativo, 0, 0, sw, sh, 0, 0, largura_camera, altura_camera, 0);
                } 

                ALLEGRO_COLOR cor_porta = al_map_rgb(40, 50, 60);
                for (int p = 0; p < TOTAL_PORTAS; p++) {
                    if (mundo_portas[p].sala_origem != sala_atual) continue;
                    Porta porta = mundo_portas[p];
                    
                    ALLEGRO_BITMAP *sprite_atual = NULL;
                    
                    if (porta.tipo == 0) {
                        sprite_atual = sprite_porta_fechada;
                        if (porta.timer_animacao > 0 && porta.timer_animacao < 30) sprite_atual = sprite_porta_meio;
                        else if (porta.timer_animacao >= 30) sprite_atual = sprite_porta_aberta;
                    } else if (porta.tipo == 1) {
                        sprite_atual = sprite_porta_exit_fechada;
                        if (porta.timer_animacao > 0 && porta.timer_animacao < 30) sprite_atual = sprite_porta_exit_meio;
                        else if (porta.timer_animacao >= 30) sprite_atual = sprite_porta_exit_aberta;
                    }

                    if (sprite_atual) {
                        float sw = al_get_bitmap_width(sprite_atual); float sh = al_get_bitmap_height(sprite_atual);
                        float draw_x = porta.x + (porta.w / 2.0) - (sw / 2.0);
                        float draw_y = (porta.y + porta.h) - sh;
                        al_draw_bitmap(sprite_atual, draw_x, draw_y, 0);
                    } else {
                        al_draw_filled_rectangle(porta.x, porta.y, porta.x + porta.w, porta.y + porta.h, cor_porta);
                        al_draw_rectangle(porta.x, porta.y, porta.x + porta.w, porta.y + porta.h, al_map_rgb(20, 20, 30), 2.0);
                    }
                }

                ALLEGRO_COLOR cor_plat = al_map_rgb(60, 60, 75);
                ALLEGRO_COLOR cor_borda = al_map_rgb(80, 120, 100); 
                for (int p = 0; p < TOTAL_PLATAFORMAS; p++) {
                    if (mundo_plataformas[p].sala != sala_atual) continue;
                    Plataforma plat = mundo_plataformas[p];
                    
                    if (sprite_chao) {
                        float sw = al_get_bitmap_width(sprite_chao);
                        float sh = al_get_bitmap_height(sprite_chao);
                        for(float tx = 0; tx < plat.w; tx += sw) {
                            float draw_w = (tx + sw > plat.w) ? (plat.w - tx) : sw;
                            for(float ty = 0; ty < plat.h; ty += sh) {
                                float draw_h = (ty + sh > plat.h) ? (plat.h - ty) : sh;
                                al_draw_bitmap_region(sprite_chao, 0, 0, draw_w, draw_h, plat.x + tx, plat.y + ty, 0);
                            }
                        }
                    } else {
                        al_draw_filled_rectangle(plat.x, plat.y, plat.x + plat.w, plat.y + plat.h, cor_plat);
                        al_draw_line(plat.x, plat.y, plat.x + plat.w, plat.y, cor_borda, 1.0);
                    }
                }

                for (int e = 0; e < TOTAL_ESCADAS; e++) {
                    if (mundo_escadas[e].sala != sala_atual) continue;
                    Escada esc = mundo_escadas[e];
                    
                    if (sprite_escada) {
                        int flag = (esc.direcao == -1) ? ALLEGRO_FLIP_HORIZONTAL : 0;
                        al_draw_bitmap(sprite_escada, esc.x, esc.y, flag);
                    } else {
                        if (esc.direcao == 1) {
                            al_draw_filled_triangle(esc.x, esc.y + esc.h, esc.x + esc.w, esc.y + esc.h, esc.x + esc.w, esc.y, al_map_rgb(200, 200, 50));
                        } else {
                            al_draw_filled_triangle(esc.x, esc.y, esc.x, esc.y + esc.h, esc.x + esc.w, esc.y + esc.h, al_map_rgb(200, 200, 50));
                        }
                    }
                }

                if (sala_atual == 1 && !prop_na_frente) {
                    if (sprite_prop1) {
                        float pw = al_get_bitmap_width(sprite_prop1);
                        float ph = al_get_bitmap_height(sprite_prop1);
                        float novo_w = 73.0;
                        float novo_h = 42.0;
                        al_draw_scaled_bitmap(sprite_prop1, 0, 0, pw, ph, 322.0, 161.0, novo_w, novo_h, 0); 
                    }
                }

                if (sala_atual == anomalia.sala_origem) {
                    ALLEGRO_BITMAP *sprite_galio = (anomalia.frame_animacao == 1) ? anomalia.sprites_bre[anomalia.frame_atual] : anomalia.sprites_normal[anomalia.frame_atual];
                    int flag_galio_flip = (anomalia.direcao_atual == DIR_ESQUERDA) ? ALLEGRO_FLIP_HORIZONTAL : 0;
                    if (sprite_galio != NULL) {
                        float larg_img = al_get_bitmap_width(sprite_galio); float alt_img = al_get_bitmap_height(sprite_galio);
                        al_draw_bitmap(sprite_galio, anomalia.x - (larg_img / 2), anomalia.y - (alt_img / 2), flag_galio_flip);
                    } else {
                        int color_val = 200 - (anomalia.frame_atual * 25);
                        al_draw_filled_rectangle(anomalia.x - anomalia.w/2, anomalia.y - anomalia.h/2, anomalia.x + anomalia.w/2, anomalia.y + anomalia.h/2, al_map_rgb(color_val, color_val, color_val + 55));
                    }
                }

                if (arma_mop.no_chao && arma_mop.sala_origem == sala_atual && arma_mop.sprite) {
                    float larg_mop = al_get_bitmap_width(arma_mop.sprite); float alt_mop = al_get_bitmap_height(arma_mop.sprite);
                    al_draw_bitmap(arma_mop.sprite, arma_mop.x - (larg_mop/2), arma_mop.y - alt_mop, 0);
                }

                for (int i = 0; i < MAX_PROJETEIS; i++) {
                    if (projeteis[i].ativo) {
                        if (projeteis[i].tipo == PROJETIL_AGUA && sprite_water) {
                            float larg_w = al_get_bitmap_width(sprite_water); float alt_w = al_get_bitmap_height(sprite_water);
                            float angulo_water = (projeteis[i].vel_x > 0) ? (ALLEGRO_PI / 2.0) : (-ALLEGRO_PI / 2.0);
                            al_draw_rotated_bitmap(sprite_water, larg_w / 2.0, alt_w / 2.0, projeteis[i].x, projeteis[i].y, angulo_water, 0);
                        } else {
                            al_draw_filled_circle(projeteis[i].x, projeteis[i].y, 2, al_map_rgb(100, 200, 255)); 
                        }
                    }
                }

                int tipo_frame_desenhar = FRAME_IDLE;
                if (tem_sprites) {
                    if (faxineiro.esta_andando) {
                        int sequencia_andar[] = {FRAME_WALK1, FRAME_IDLE, FRAME_WALK2, FRAME_IDLE};
                        tipo_frame_desenhar = sequencia_andar[faxineiro.frame_atual];
                    } else {
                        int sequencia_parado[] = {FRAME_IDLE, FRAME_BRE};
                        tipo_frame_desenhar = sequencia_parado[faxineiro.frame_atual];
                    }
                }

                int flag_espelhamento = (faxineiro.direcao_atual == DIR_ESQUERDA) ? ALLEGRO_FLIP_HORIZONTAL : 0;

                if (tem_sprites) {
                    ALLEGRO_BITMAP *sprite_final = faxineiro.sprites[tipo_frame_desenhar];
                    if(sprite_final != NULL) {
                        float larg_img = al_get_bitmap_width(sprite_final); float alt_img = al_get_bitmap_height(sprite_final);
                        float draw_x = faxineiro.x - (larg_img / 2.0); float draw_y = (faxineiro.y + faxineiro.h / 2.0) - alt_img;
                        al_draw_bitmap(sprite_final, draw_x, draw_y, flag_espelhamento);
                    }
                }

                if (faxineiro.possui_arma && arma_mop.sprite) {
                    float mao_x = faxineiro.x; float mao_y = faxineiro.y - (faxineiro.body_h / 2.0) + 4; 
                    if (tipo_frame_desenhar == FRAME_BRE) mao_y += 1; else if (tipo_frame_desenhar == FRAME_WALK1) mao_y -= 2; 

                    if (faxineiro.direcao_atual == DIR_DIREITA) mao_x -= 6; else mao_x += 6;

                    float larg_mop = al_get_bitmap_width(arma_mop.sprite); float alt_mop = al_get_bitmap_height(arma_mop.sprite);
                    float angulo_rotacao = (faxineiro.direcao_atual == DIR_DIREITA) ? (-ALLEGRO_PI / 2.0) : (ALLEGRO_PI / 2.0);
                    al_draw_rotated_bitmap(arma_mop.sprite, larg_mop/2.0, alt_mop/2.0, mao_x, mao_y, angulo_rotacao, flag_espelhamento);
                }

                if (sala_atual == 1 && prop_na_frente) {
                    if (sprite_prop1) {
                        float pw = al_get_bitmap_width(sprite_prop1);
                        float ph = al_get_bitmap_height(sprite_prop1);
                        float novo_w = 73.0;
                        float novo_h = 42.0;
                        al_draw_scaled_bitmap(sprite_prop1, 0, 0, pw, ph, 322.0, 161.0, novo_w, novo_h, 0); 
                    }
                }

                if (estado_fade > 0) al_draw_filled_rectangle(0, 0, largura_camera, altura_camera, al_map_rgba(0, 0, 0, alpha_fade));

                if (mostrar_hitbox && estado_fade == 0) {
                    al_draw_rectangle(faxineiro.x - faxineiro.w/2, faxineiro.y - faxineiro.h/2, faxineiro.x + faxineiro.w/2, faxineiro.y + faxineiro.h/2, al_map_rgb(0, 255, 0), 1.0);
                    float body_top = (faxineiro.y + faxineiro.h/2) - faxineiro.body_h;
                    al_draw_rectangle(faxineiro.x - faxineiro.body_w/2, body_top, faxineiro.x + faxineiro.body_w/2, faxineiro.y + faxineiro.h/2, al_map_rgb(0, 255, 255), 1.0);

                    al_draw_text(fonte, al_map_rgb(255, 255, 0), largura_camera / 2.0, 10, ALLEGRO_ALIGN_CENTRE, "DEBUG MODE");
                    al_draw_textf(fonte, al_map_rgb(0, 255, 255), largura_camera - 10, 10, ALLEGRO_ALIGN_RIGHT, "Ambiente: %.1f C", TEMP_ESTACAO);
                    
                    al_draw_textf(fonte, al_map_rgb(255, 150, 150), faxineiro.x, faxineiro.y - faxineiro.h/2 - 15, ALLEGRO_ALIGN_CENTRE, "%.1f C", TEMP_JOGADOR);

                    if (sala_atual == anomalia.sala_origem) {
                        al_draw_textf(fonte, al_map_rgb(255, 150, 150), anomalia.x, anomalia.y - anomalia.h/2 - 15, ALLEGRO_ALIGN_CENTRE, "%.1f C", anomalia.temperatura);
                    }
                }

                if (estado_jogo == ESTADO_PAUSADO) {
                    al_use_transform(&camera_ui); 
                    al_draw_filled_rectangle(0, 0, LARGURA_MAPA, ALTURA_MAPA, al_map_rgba(0, 0, 0, 180));
                    al_draw_text(fonte, al_map_rgb(255, 255, 255), LARGURA_MAPA / 2, 70, ALLEGRO_ALIGN_CENTRE, "JOGO PAUSADO");

                    const char *itens_pause[] = {"Continuar", "Menu Principal", "Sair do Jogo"};
                    for (int i = 0; i < 3; i++) {
                        ALLEGRO_COLOR cor = (i == opcao_pause) ? al_map_rgb(100, 255, 100) : al_map_rgb(200, 200, 200);
                        al_draw_text(fonte, cor, LARGURA_MAPA / 2, 110 + (i * 25), ALLEGRO_ALIGN_CENTRE, itens_pause[i]);
                    }
                    al_draw_text(fonte, al_map_rgb(255, 100, 100), LARGURA_MAPA / 2, 195, ALLEGRO_ALIGN_CENTRE, "Aviso: Qualquer progresso nao salvo sera perdido!");
                }
            }

            al_flip_display();
        }
    }

    if (sprite_prop1) al_destroy_bitmap(sprite_prop1);
    if (sprite_chao) al_destroy_bitmap(sprite_chao);
    if (sprite_escada) al_destroy_bitmap(sprite_escada);
    if (sprite_porta_fechada) al_destroy_bitmap(sprite_porta_fechada);
    if (sprite_porta_meio) al_destroy_bitmap(sprite_porta_meio);
    if (sprite_porta_aberta) al_destroy_bitmap(sprite_porta_aberta);
    if (sprite_porta_exit_fechada) al_destroy_bitmap(sprite_porta_exit_fechada);
    if (sprite_porta_exit_meio) al_destroy_bitmap(sprite_porta_exit_meio);
    if (sprite_porta_exit_aberta) al_destroy_bitmap(sprite_porta_exit_aberta);
    for (int f = 0; f < 4; f++) if (faxineiro.sprites[f]) al_destroy_bitmap(faxineiro.sprites[f]);
    for (int i = 0; i < 5; i++) {
        if (anomalia.sprites_normal[i]) al_destroy_bitmap(anomalia.sprites_normal[i]);
        if (anomalia.sprites_bre[i]) al_destroy_bitmap(anomalia.sprites_bre[i]);
    }
    for (int i = 0; i < MAX_SALAS; i++) if (backgrounds[i]) al_destroy_bitmap(backgrounds[i]);
    if (arma_mop.sprite) al_destroy_bitmap(arma_mop.sprite);
    if (sprite_water) al_destroy_bitmap(sprite_water); 
    if (fonte) al_destroy_font(fonte);
    
    al_destroy_timer(timer);
    al_destroy_display(tela);
    al_destroy_event_queue(fila_eventos);

    return 0;
}