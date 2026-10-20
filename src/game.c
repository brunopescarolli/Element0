#include "game.h"
#include "player.h"
#include "enemy.h"
#include "elementos.h"

bool game_init(Game *game) {
    if (!al_init()) return false;
    if (!al_install_keyboard()) return false;
    if (!al_install_mouse()) return false;
    if (!al_init_primitives_addon()) return false;
    if (!al_init_image_addon()) return false;
    al_init_font_addon();
    al_install_joystick();

    al_set_new_display_flags(ALLEGRO_FULLSCREEN_WINDOW);
    game->tela = al_create_display(LARGURA_JANELA, ALTURA_JANELA);
    if (!game->tela) return false;

    al_set_blender(ALLEGRO_ADD, ALLEGRO_ALPHA, ALLEGRO_INVERSE_ALPHA);

    game->fonte = al_create_builtin_font();
    carregar_tabela_periodica("assets/elementos.csv");

    for (int i = 0; i < MAX_SALAS; i++) {
        char path[128];
        sprintf(path, "assets/sprites/background/fundo_%d.png", i);
        game->sprites.backgrounds[i] = al_load_bitmap(path);
    }

    game->sprites.sprite_porta_fechada = al_load_bitmap("assets/sprites/background/door_closed.png");
    game->sprites.sprite_porta_meio = al_load_bitmap("assets/sprites/background/door_open1.png");
    game->sprites.sprite_porta_aberta = al_load_bitmap("assets/sprites/background/door_open2.png");

    game->sprites.sprite_porta_exit_fechada = al_load_bitmap("assets/sprites/background/door_exit_closed.png");
    game->sprites.sprite_porta_exit_meio = al_load_bitmap("assets/sprites/background/door_exit_open1.png");
    game->sprites.sprite_porta_exit_aberta = al_load_bitmap("assets/sprites/background/door_exit_open2.png");

    game->sprites.sprite_chao = al_load_bitmap("assets/sprites/background/chao.png");
    game->sprites.sprite_escada = al_load_bitmap("assets/sprites/escada.png");
    game->sprites.sprite_prop1 = al_load_bitmap("assets/sprites/prop1.png");

    game->faxineiro = (Jogador){
        32.0, 127.0 - (52.0 / 2.0),
        10.0, 4.0,
        16.0, 52.0,
        2.5, 0.0, true, DIR_DIREITA, 0,
        {NULL, NULL, NULL, NULL},
        false, 0, 0, false
    };

    game->faxineiro.sprites[FRAME_IDLE]  = al_load_bitmap("assets/sprites/front_idle.png");
    game->faxineiro.sprites[FRAME_BRE]   = al_load_bitmap("assets/sprites/front_bre.png");
    game->faxineiro.sprites[FRAME_WALK1] = al_load_bitmap("assets/sprites/front_walk1.png");
    game->faxineiro.sprites[FRAME_WALK2] = al_load_bitmap("assets/sprites/front_walk2.png");

    Plataforma temp_plat[TOTAL_PLATAFORMAS] = {
        { 0, 127, 240.0, 8, 0, false },
        { 0, 254, LARGURA_MAPA, 16, 1, false },
        { 192, 190, 288, 8, 1, true },
        { 0, 126, 160, 8, 1, true },
        { 160, 158, 32, 8, 1, true },
        { 0, 254, LARGURA_MAPA, 16, 2, false },
        { 0, 254, LARGURA_MAPA, 16, 3, false },
        { 0, 254, LARGURA_MAPA, 16, 4, false },
        { 0, 254, LARGURA_MAPA, 16, 5, false },
        { 0, 254, LARGURA_MAPA, 16, 6, false },
        { 0, 254, LARGURA_MAPA, 16, 7, false },
        { 0, 254, LARGURA_MAPA, 16, 8, false },
        { 0, 254, LARGURA_MAPA, 16, 9, false },
        { 0, 254, LARGURA_MAPA, 16, 10, false },
        { 0, 254, LARGURA_MAPA, 16, 11, false },
        { 0, 254, LARGURA_MAPA, 16, 12, false },
        { 0, 254, LARGURA_MAPA, 16, 13, false },
        { 0, 254, LARGURA_MAPA, 16, 14, false },
        { 0, 254, LARGURA_MAPA, 16, 15, false },
        { 0, 254, LARGURA_MAPA, 16, 16, false },
        { 0, 254, LARGURA_MAPA, 16, 17, false },
        { 0, 254, LARGURA_MAPA, 16, 18, false },
        { 0, 254, LARGURA_MAPA, 16, 19, false },
        { 0, 254, LARGURA_MAPA, 16, 20, false },
        { 0, 254, LARGURA_MAPA, 16, 21, false },
        { 0, 254, LARGURA_MAPA, 16, 22, false },
        { 0, 254, LARGURA_MAPA, 16, 23, false },
        { 0, 254, LARGURA_MAPA, 16, 24, false },
        { 0, 254, LARGURA_MAPA, 16, 25, false },
        { 0, 254, LARGURA_MAPA, 16, 26, false },
        { 0, 254, LARGURA_MAPA, 16, 27, false },
        { 0, 254, LARGURA_MAPA, 16, 28, false },
        { 0, 254, LARGURA_MAPA, 16, 29, false },
        { 0, 254, LARGURA_MAPA, 16, 30, false },
        { 0, 254, LARGURA_MAPA, 16, 31, false }
    };
    for (int i = 0; i < TOTAL_PLATAFORMAS; i++) game->mundo_plataformas[i] = temp_plat[i];

    Porta temp_porta[TOTAL_PORTAS] = {
        // Sala 0: Limpeza (Andar 5)
        { 171, 31, 64, 96, 0, 1, false, 0, 0 },

        // Sala 1: Armazem geral (Andar 5)
        { 0, 158, 64, 96, 1, 0, false, 0, 0 },
        { 416, 94, 64, 96, 1, 2, false, 0, 0 },

        // Sala 2: Hall 5 (Andar 5) - 1 saida de emergencia para Escadaria 5 (30)
        { 0, 158, 64, 96, 2, 1, false, 0, 0 },
        { 320, 158, 64, 96, 2, 30, false, 0, 1 },
        { 416, 158, 64, 96, 2, 4, false, 0, 0 },

        // Sala 3: Hall 4 (Andar 4) - 1 saida de emergencia para Escadaria 4 (29)
        { 0, 158, 64, 96, 3, 7, false, 0, 0 },
        { 320, 158, 64, 96, 3, 29, false, 0, 1 },
        { 416, 158, 64, 96, 3, 8, false, 0, 0 },

        // Sala 4: Deposito quimico (Andar 5)
        { 0, 158, 64, 96, 4, 2, false, 0, 0 },
        { 416, 158, 64, 96, 4, 9, false, 0, 0 },

        // Sala 5: Hall 6 (Andar 6) - 1 saida de emergencia para Escadaria 6 (31)
        { 0, 158, 64, 96, 5, 10, false, 0, 0 },
        { 320, 158, 64, 96, 5, 31, false, 0, 1 },
        { 416, 158, 64, 96, 5, 11, false, 0, 0 },

        // Sala 6: Hall 3 (Andar 3) - 1 saida de emergencia para Escadaria 3 (28)
        { 0, 158, 64, 96, 6, 13, false, 0, 0 },
        { 320, 158, 64, 96, 6, 28, false, 0, 1 },
        { 416, 158, 64, 96, 6, 14, false, 0, 0 },

        // Sala 7: Lab solidos (Andar 4)
        { 416, 158, 64, 96, 7, 3, false, 0, 0 },

        // Sala 8: Contencao solidos (Andar 4)
        { 0, 158, 64, 96, 8, 3, false, 0, 0 },

        // Sala 9: Entrada/saida carga (Andar 5)
        { 0, 158, 64, 96, 9, 4, false, 0, 0 },

        // Sala 10: Estufa/reciclagem (Andar 6)
        { 416, 158, 64, 96, 10, 5, false, 0, 0 },

        // Sala 11: Eletrica (Andar 6)
        { 0, 158, 64, 96, 11, 5, false, 0, 0 },

        // Sala 12: Hall 2 (Andar 2) - 1 saida de emergencia para Escadaria 2 (27)
        { 0, 158, 64, 96, 12, 16, false, 0, 0 },
        { 320, 158, 64, 96, 12, 27, false, 0, 1 },
        { 416, 158, 64, 96, 12, 17, false, 0, 0 },

        // Sala 13: Lab liquidos (Andar 3)
        { 416, 158, 64, 96, 13, 6, false, 0, 0 },

        // Sala 14: Contencao liquidos (Andar 3)
        { 0, 158, 64, 96, 14, 6, false, 0, 0 },

        // Sala 15: Hall 1 (Andar 1) - 1 saida de emergencia para Escadaria 1 (26)
        { 0, 158, 64, 96, 15, 19, false, 0, 0 },
        { 96, 158, 64, 96, 15, 18, false, 0, 0 },
        { 208, 158, 64, 96, 15, 20, false, 0, 0 },
        { 320, 158, 64, 96, 15, 26, false, 0, 1 },
        { 416, 158, 64, 96, 15, 21, false, 0, 0 },

        // Sala 16: Lab gasosos (Andar 2)
        { 416, 158, 64, 96, 16, 12, false, 0, 0 },

        // Sala 17: Contencao gasosos (Andar 2)
        { 0, 158, 64, 96, 17, 12, false, 0, 0 },

        // Sala 18: Evacuacao (Andar 1)
        { 208, 158, 64, 96, 18, 15, false, 0, 0 },

        // Sala 19: Area comum (Andar 1)
        { 0, 158, 64, 96, 19, 22, false, 0, 0 },
        { 208, 158, 64, 96, 19, 23, false, 0, 0 },
        { 416, 158, 64, 96, 19, 15, false, 0, 0 },

        // Sala 20: Dormitorios (Andar 1)
        { 0, 158, 64, 96, 20, 15, false, 0, 0 },
        { 416, 158, 64, 96, 20, 24, false, 0, 0 },

        // Sala 21: Ala medica (Andar 1)
        { 0, 158, 64, 96, 21, 15, false, 0, 0 },
        { 416, 158, 64, 96, 21, 25, false, 0, 0 },

        // Sala 22: Central (Andar 1)
        { 416, 158, 64, 96, 22, 19, false, 0, 0 },

        // Sala 23: Refeitorio (Andar 1)
        { 208, 158, 64, 96, 23, 19, false, 0, 0 },

        // Sala 24: Banheiros (Andar 1)
        { 0, 158, 64, 96, 24, 20, false, 0, 0 },

        // Sala 25: Quarentena (Andar 1)
        { 0, 158, 64, 96, 25, 21, false, 0, 0 },

        // Sala 26: Escadaria 1 (Andar 1 - Superior: Descer e Voltar ao Hall 1)
        { 0, 158, 64, 96, 26, 27, false, 0, 1 },
        { 208, 158, 64, 96, 26, 15, false, 0, 1 },

        // Sala 27: Escadaria 2 (Andar 2 - Intermediario: Descer, Voltar ao Hall 2 e Subir)
        { 0, 158, 64, 96, 27, 28, false, 0, 1 },
        { 208, 158, 64, 96, 27, 12, false, 0, 1 },
        { 416, 158, 64, 96, 27, 26, false, 0, 1 },

        // Sala 28: Escadaria 3 (Andar 3 - Intermediario: Descer, Voltar ao Hall 3 e Subir)
        { 0, 158, 64, 96, 28, 29, false, 0, 1 },
        { 208, 158, 64, 96, 28, 6, false, 0, 1 },
        { 416, 158, 64, 96, 28, 27, false, 0, 1 },

        // Sala 29: Escadaria 4 (Andar 4 - Intermediario: Descer, Voltar ao Hall 4 e Subir)
        { 0, 158, 64, 96, 29, 30, false, 0, 1 },
        { 208, 158, 64, 96, 29, 3, false, 0, 1 },
        { 416, 158, 64, 96, 29, 28, false, 0, 1 },

        // Sala 30: Escadaria 5 (Andar 5 - Intermediario: Descer, Voltar ao Hall 5 e Subir)
        { 0, 158, 64, 96, 30, 31, false, 0, 1 },
        { 208, 158, 64, 96, 30, 2, false, 0, 1 },
        { 416, 158, 64, 96, 30, 29, false, 0, 1 },

        // Sala 31: Escadaria 6 (Andar 6 - Inferior: Voltar ao Hall 6 e Subir)
        { 208, 158, 64, 96, 31, 5, false, 0, 1 },
        { 416, 158, 64, 96, 31, 30, false, 0, 1 }
    };
    for (int i = 0; i < TOTAL_PORTAS; i++) game->mundo_portas[i] = temp_porta[i];

    Escada temp_escada[TOTAL_ESCADAS] = {
        { 192, 158, 32, 32, 1, -1 },
        { 160, 126, 32, 32, 1, -1 }
    };
    for (int i = 0; i < TOTAL_ESCADAS; i++) game->mundo_escadas[i] = temp_escada[i];

    game->arma_mop = (Item){ 62.0, 127.0, 0.0, 30.0, true, 0, NULL };
    game->arma_mop.sprite = al_load_bitmap("assets/sprites/mop.png");
    game->sprites.sprite_water = al_load_bitmap("assets/sprites/water.png");

    game->anomalia = (Inimigo){
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
        sprintf(path, "assets/sprites/galio_%d.png", i + 1);
        game->anomalia.sprites_normal[i] = al_load_bitmap(path);

        sprintf(path, "assets/sprites/galio_%d_bre.png", i + 1);
        game->anomalia.sprites_bre[i] = al_load_bitmap(path);
    }

    for (int i = 0; i < MAX_PROJETEIS; i++) game->projeteis[i].ativo = false;

    game->timer = al_create_timer(1.0 / FPS);
    game->fila_eventos = al_create_event_queue();
    al_register_event_source(game->fila_eventos, al_get_display_event_source(game->tela));
    al_register_event_source(game->fila_eventos, al_get_timer_event_source(game->timer));
    al_register_event_source(game->fila_eventos, al_get_keyboard_event_source());
    al_register_event_source(game->fila_eventos, al_get_mouse_event_source());

    game->rodando = true;
    game->redesenhar = true;
    game->mostrar_hitbox = false;
    game->estado_jogo = ESTADO_MENU_PRINCIPAL;
    game->opcao_menu = 0;
    game->opcao_confirma = 0;
    game->opcao_pause = 0;
    game->opcao_config = 0;
    game->slot_sendo_remapeado = -1;
    strcpy(game->nome_jogador, "");
    carregar_controles_padrao(&game->controles);
    game->sala_atual = 0;
    game->estado_fade = 0;
    game->alpha_fade = 0;
    game->timer_fade = 0;
    game->proxima_sala = 0;
    game->proximo_x = 0;
    game->proximo_chao_y = 0;
    game->prop_na_frente = false;
    game->input = (Input){0.0, false, false, false, false, false, false, false};

    al_start_timer(game->timer);
    return true;
}

void game_run(Game *game) {
    while (game->rodando) {
        ALLEGRO_EVENT evento;
        al_wait_for_event(game->fila_eventos, &evento);

        if (evento.type == ALLEGRO_EVENT_DISPLAY_CLOSE) {
            game->rodando = false;
        }
        else if (evento.type == ALLEGRO_EVENT_KEY_CHAR) {
            if (game->estado_jogo == ESTADO_DIGITAR_NOME) {
                int keycode = evento.keyboard.unichar;
                if (keycode == 8) {
                    int len = strlen(game->nome_jogador);
                    if (len > 0) game->nome_jogador[len - 1] = '\0';
                } else if (keycode == 13) {
                    if (strlen(game->nome_jogador) > 0) {
                        game->sala_atual = 0;
                        game->faxineiro.x = 32.0;
                        game->faxineiro.y = 127.0 - (game->faxineiro.h / 2.0);
                        game->faxineiro.possui_arma = false;

                        game->arma_mop.sala_origem = 0;
                        game->arma_mop.no_chao = true;
                        game->arma_mop.x = game->faxineiro.x + 30.0;
                        game->arma_mop.y = 127.0;
                        game->arma_mop.vel_y = 0;

                        salvar_jogo(game->nome_jogador, game->sala_atual, game->faxineiro.x, game->faxineiro.y, game->faxineiro.possui_arma);
                        game->estado_jogo = ESTADO_JOGANDO;
                    }
                } else if (keycode >= 32 && keycode <= 126) {
                    int len = strlen(game->nome_jogador);
                    if (len < 28) {
                        game->nome_jogador[len] = (char)keycode;
                        game->nome_jogador[len + 1] = '\0';
                    }
                }
                game->redesenhar = true;
            }
        }
        else if (evento.type == ALLEGRO_EVENT_KEY_DOWN) {
            if (game->slot_sendo_remapeado != -1) {
                int tecla = evento.keyboard.keycode;
                if (game->slot_sendo_remapeado == SLOT_PULAR) game->controles.tecla_pular = tecla;
                else if (game->slot_sendo_remapeado == SLOT_ESQUERDA) game->controles.tecla_esquerda = tecla;
                else if (game->slot_sendo_remapeado == SLOT_DIREITA) game->controles.tecla_direita = tecla;
                else if (game->slot_sendo_remapeado == SLOT_ATIRAR) game->controles.tecla_atirar = tecla;
                else if (game->slot_sendo_remapeado == SLOT_PEGAR) game->controles.tecla_pegar = tecla;
                else if (game->slot_sendo_remapeado == SLOT_SOLTAR) game->controles.tecla_soltar = tecla;
                else if (game->slot_sendo_remapeado == SLOT_ENTRAR) game->controles.tecla_entrar = tecla;
                game->slot_sendo_remapeado = -1;
            }
            else if (game->estado_jogo == ESTADO_MENU_PRINCIPAL) {
                if (evento.keyboard.keycode == ALLEGRO_KEY_DOWN || evento.keyboard.keycode == ALLEGRO_KEY_S) {
                    game->opcao_menu = (game->opcao_menu + 1) % 4;
                }
                else if (evento.keyboard.keycode == ALLEGRO_KEY_UP || evento.keyboard.keycode == ALLEGRO_KEY_W) {
                    game->opcao_menu = (game->opcao_menu - 1 + 4) % 4;
                }
                else if (evento.keyboard.keycode == ALLEGRO_KEY_ENTER || evento.keyboard.keycode == ALLEGRO_KEY_SPACE) {
                    if (game->opcao_menu == 0) {
                        FILE *f = fopen("save.dat", "r");
                        if (f) {
                            fclose(f);
                            game->estado_jogo = ESTADO_CONFIRMAR_NOVO_JOGO;
                            game->opcao_confirma = 1;
                        } else {
                            strcpy(game->nome_jogador, "");
                            game->estado_jogo = ESTADO_DIGITAR_NOME;
                        }
                    } else if (game->opcao_menu == 1) {
                        FILE *f_chk = fopen("save.dat", "r");
                        if (f_chk) {
                            fclose(f_chk);
                            if (carregar_jogo(game->nome_jogador, &game->sala_atual, &game->faxineiro.x, &game->faxineiro.y, &game->faxineiro.possui_arma)) {
                                if (game->faxineiro.possui_arma) game->arma_mop.no_chao = false;
                                game->estado_jogo = ESTADO_JOGANDO;
                            }
                        }
                    } else if (game->opcao_menu == 2) {
                        game->estado_jogo = ESTADO_RECORDES;
                    } else if (game->opcao_menu == 3) {
                        game->estado_jogo = ESTADO_CONFIGURACOES;
                        game->opcao_config = 0;
                    }
                }
            }
            else if (game->estado_jogo == ESTADO_CONFIRMAR_NOVO_JOGO) {
                if (evento.keyboard.keycode == ALLEGRO_KEY_LEFT || evento.keyboard.keycode == ALLEGRO_KEY_RIGHT ||
                    evento.keyboard.keycode == ALLEGRO_KEY_A || evento.keyboard.keycode == ALLEGRO_KEY_D) {
                    game->opcao_confirma = 1 - game->opcao_confirma;
                }
                else if (evento.keyboard.keycode == ALLEGRO_KEY_ENTER || evento.keyboard.keycode == ALLEGRO_KEY_SPACE) {
                    if (game->opcao_confirma == 0) {
                        strcpy(game->nome_jogador, "");
                        game->estado_jogo = ESTADO_DIGITAR_NOME;
                    } else {
                        game->estado_jogo = ESTADO_MENU_PRINCIPAL;
                    }
                }
                else if (evento.keyboard.keycode == ALLEGRO_KEY_ESCAPE) {
                    game->estado_jogo = ESTADO_MENU_PRINCIPAL;
                }
            }
            else if (game->estado_jogo == ESTADO_RECORDES) {
                if (evento.keyboard.keycode == ALLEGRO_KEY_ESCAPE || evento.keyboard.keycode == ALLEGRO_KEY_ENTER) {
                    game->estado_jogo = ESTADO_MENU_PRINCIPAL;
                }
            }
            else if (game->estado_jogo == ESTADO_CONFIGURACOES) {
                if (evento.keyboard.keycode == ALLEGRO_KEY_ESCAPE) {
                    game->estado_jogo = ESTADO_MENU_PRINCIPAL;
                }
                else if (evento.keyboard.keycode == ALLEGRO_KEY_DOWN || evento.keyboard.keycode == ALLEGRO_KEY_S) {
                    game->opcao_config = (game->opcao_config + 1) % (TOTAL_SLOTS + 2);
                }
                else if (evento.keyboard.keycode == ALLEGRO_KEY_UP || evento.keyboard.keycode == ALLEGRO_KEY_W) {
                    game->opcao_config = (game->opcao_config - 1 + (TOTAL_SLOTS + 2)) % (TOTAL_SLOTS + 2);
                }
                else if (evento.keyboard.keycode == ALLEGRO_KEY_ENTER || evento.keyboard.keycode == ALLEGRO_KEY_SPACE) {
                    if (game->opcao_config < TOTAL_SLOTS) {
                        game->slot_sendo_remapeado = game->opcao_config;
                    } else if (game->opcao_config == TOTAL_SLOTS) {
                        carregar_controles_padrao(&game->controles);
                    } else if (game->opcao_config == TOTAL_SLOTS + 1) {
                        game->estado_jogo = ESTADO_MENU_PRINCIPAL;
                    }
                }
            }
            else if (game->estado_jogo == ESTADO_JOGANDO) {
                if (evento.keyboard.keycode == ALLEGRO_KEY_ESCAPE) {
                    game->estado_jogo = ESTADO_PAUSADO;
                    game->opcao_pause = 0;
                }
                else if (evento.keyboard.keycode == game->controles.tecla_pular) game->input.pulo = true;
                else if (evento.keyboard.keycode == game->controles.tecla_esquerda) game->input.eixo_x = -1.0;
                else if (evento.keyboard.keycode == game->controles.tecla_direita) game->input.eixo_x =  1.0;
                else if (evento.keyboard.keycode == game->controles.tecla_pegar) game->input.acao_pegar = true;
                else if (evento.keyboard.keycode == game->controles.tecla_soltar) game->input.acao_soltar = true;
                else if (evento.keyboard.keycode == game->controles.tecla_entrar) game->input.acao_entrar = true;
                else if (evento.keyboard.keycode == ALLEGRO_KEY_F3) game->input.f3_pressionado = true;
                else if (evento.keyboard.keycode == ALLEGRO_KEY_B) {
                    if (game->input.f3_pressionado) game->mostrar_hitbox = !game->mostrar_hitbox;
                }
                else if (evento.keyboard.keycode == ALLEGRO_KEY_S) game->input.acao_descer = true;
            }
            else if (game->estado_jogo == ESTADO_PAUSADO) {
                if (evento.keyboard.keycode == ALLEGRO_KEY_ESCAPE || evento.keyboard.keycode == game->controles.tecla_pular) {
                    game->estado_jogo = ESTADO_JOGANDO;
                }
                else if (evento.keyboard.keycode == ALLEGRO_KEY_DOWN || evento.keyboard.keycode == ALLEGRO_KEY_S) {
                    game->opcao_pause = (game->opcao_pause + 1) % 3;
                }
                else if (evento.keyboard.keycode == ALLEGRO_KEY_UP || evento.keyboard.keycode == ALLEGRO_KEY_W) {
                    game->opcao_pause = (game->opcao_pause - 1 + 3) % 3;
                }
                else if (evento.keyboard.keycode == ALLEGRO_KEY_ENTER || evento.keyboard.keycode == ALLEGRO_KEY_SPACE) {
                    if (game->opcao_pause == 0) {
                        game->estado_jogo = ESTADO_JOGANDO;
                    } else if (game->opcao_pause == 1) {
                        game->estado_jogo = ESTADO_MENU_PRINCIPAL;
                    } else if (game->opcao_pause == 2) {
                        game->rodando = false;
                    }
                }
            }
            game->redesenhar = true;
        }
        else if (evento.type == ALLEGRO_EVENT_KEY_UP) {
            if (game->estado_jogo == ESTADO_JOGANDO) {
                if (evento.keyboard.keycode == game->controles.tecla_pular) game->input.pulo = false;
                else if (evento.keyboard.keycode == game->controles.tecla_esquerda && game->input.eixo_x < 0) game->input.eixo_x = 0.0;
                else if (evento.keyboard.keycode == game->controles.tecla_direita && game->input.eixo_x > 0) game->input.eixo_x = 0.0;
                else if (evento.keyboard.keycode == game->controles.tecla_pegar) game->input.acao_pegar = false;
                else if (evento.keyboard.keycode == game->controles.tecla_soltar) game->input.acao_soltar = false;
                else if (evento.keyboard.keycode == game->controles.tecla_entrar) game->input.acao_entrar = false;
                else if (evento.keyboard.keycode == ALLEGRO_KEY_F3) game->input.f3_pressionado = false;
                else if (evento.keyboard.keycode == ALLEGRO_KEY_S) game->input.acao_descer = false;
            }
        }
        else if (evento.type == ALLEGRO_EVENT_MOUSE_AXES) {
            float mapa_mouse_x = evento.mouse.x * ((float)LARGURA_MAPA / LARGURA_JANELA);
            float mapa_mouse_y = evento.mouse.y * ((float)ALTURA_MAPA / ALTURA_JANELA);
            bool sobre_elemento = false;

            if (game->estado_jogo == ESTADO_MENU_PRINCIPAL) {
                for (int i = 0; i < 4; i++) {
                    float item_y = 90 + (i * 25);
                    if (mapa_mouse_x >= 100 && mapa_mouse_x <= 380 && mapa_mouse_y >= item_y && mapa_mouse_y <= item_y + 20) {
                        game->opcao_menu = i;
                        sobre_elemento = true;
                        break;
                    }
                }
            }
            else if (game->estado_jogo == ESTADO_CONFIRMAR_NOVO_JOGO) {
                if (mapa_mouse_y >= 150 && mapa_mouse_y <= 175) {
                    if (mapa_mouse_x >= 160 && mapa_mouse_x <= 220) { game->opcao_confirma = 0; sobre_elemento = true; }
                    else if (mapa_mouse_x >= 260 && mapa_mouse_x <= 320) { game->opcao_confirma = 1; sobre_elemento = true; }
                }
            }
            else if (game->estado_jogo == ESTADO_CONFIGURACOES) {
                for (int i = 0; i < TOTAL_SLOTS + 2; i++) {
                    float item_y = 50 + (i * 22) + (i >= TOTAL_SLOTS ? 10 : 0);
                    if (i == TOTAL_SLOTS + 1) item_y += 10;
                    if (mapa_mouse_x >= 100 && mapa_mouse_x <= 380 && mapa_mouse_y >= item_y && mapa_mouse_y <= item_y + 20) {
                        game->opcao_config = i;
                        sobre_elemento = true;
                        break;
                    }
                }
            }
            else if (game->estado_jogo == ESTADO_PAUSADO) {
                for (int i = 0; i < 3; i++) {
                    float item_y = 110 + (i * 25);
                    if (mapa_mouse_x >= 100 && mapa_mouse_x <= 380 && mapa_mouse_y >= item_y && mapa_mouse_y <= item_y + 20) {
                        game->opcao_pause = i;
                        sobre_elemento = true;
                        break;
                    }
                }
            }

            if (sobre_elemento) {
                al_set_system_mouse_cursor(game->tela, ALLEGRO_SYSTEM_MOUSE_CURSOR_LINK);
            } else {
                al_set_system_mouse_cursor(game->tela, ALLEGRO_SYSTEM_MOUSE_CURSOR_DEFAULT);
            }
            game->redesenhar = true;
        }
        else if (evento.type == ALLEGRO_EVENT_MOUSE_BUTTON_DOWN) {
            float mapa_mouse_x = evento.mouse.x * ((float)LARGURA_MAPA / LARGURA_JANELA);
            float mapa_mouse_y = evento.mouse.y * ((float)ALTURA_MAPA / ALTURA_JANELA);

            if (game->estado_jogo == ESTADO_JOGANDO && evento.mouse.button == 1) {
                game->input.atirando = true;
            }
            else if (game->estado_jogo == ESTADO_MENU_PRINCIPAL && evento.mouse.button == 1) {
                for (int i = 0; i < 4; i++) {
                    float item_y = 90 + (i * 25);
                    if (mapa_mouse_x >= 100 && mapa_mouse_x <= 380 && mapa_mouse_y >= item_y && mapa_mouse_y <= item_y + 20) {
                        game->opcao_menu = i;
                        if (game->opcao_menu == 0) {
                            FILE *f = fopen("save.dat", "r");
                            if (f) {
                                fclose(f);
                                game->estado_jogo = ESTADO_CONFIRMAR_NOVO_JOGO;
                                game->opcao_confirma = 1;
                            } else {
                                strcpy(game->nome_jogador, "");
                                game->estado_jogo = ESTADO_DIGITAR_NOME;
                            }
                        } else if (game->opcao_menu == 1) {
                            FILE *f_chk = fopen("save.dat", "r");
                            if (f_chk) {
                                fclose(f_chk);
                                if (carregar_jogo(game->nome_jogador, &game->sala_atual, &game->faxineiro.x, &game->faxineiro.y, &game->faxineiro.possui_arma)) {
                                    if (game->faxineiro.possui_arma) game->arma_mop.no_chao = false;
                                    game->estado_jogo = ESTADO_JOGANDO;
                                }
                            }
                        } else if (game->opcao_menu == 2) {
                            game->estado_jogo = ESTADO_RECORDES;
                        } else if (game->opcao_menu == 3) {
                            game->estado_jogo = ESTADO_CONFIGURACOES;
                            game->opcao_config = 0;
                        }
                        break;
                    }
                }
            }
            else if (game->estado_jogo == ESTADO_CONFIRMAR_NOVO_JOGO && evento.mouse.button == 1) {
                if (mapa_mouse_y >= 150 && mapa_mouse_y <= 175) {
                    if (mapa_mouse_x >= 160 && mapa_mouse_x <= 220) {
                        strcpy(game->nome_jogador, "");
                        game->estado_jogo = ESTADO_DIGITAR_NOME;
                    }
                    else if (mapa_mouse_x >= 260 && mapa_mouse_x <= 320) {
                        game->estado_jogo = ESTADO_MENU_PRINCIPAL;
                    }
                }
            }
            else if (game->estado_jogo == ESTADO_RECORDES && evento.mouse.button == 1) {
                game->estado_jogo = ESTADO_MENU_PRINCIPAL;
            }
            else if (game->estado_jogo == ESTADO_CONFIGURACOES && evento.mouse.button == 1) {
                for (int i = 0; i < TOTAL_SLOTS + 2; i++) {
                    float item_y = 50 + (i * 22) + (i >= TOTAL_SLOTS ? 10 : 0);
                    if (i == TOTAL_SLOTS + 1) item_y += 10;
                    if (mapa_mouse_x >= 100 && mapa_mouse_x <= 380 && mapa_mouse_y >= item_y && mapa_mouse_y <= item_y + 20) {
                        game->opcao_config = i;
                        if (game->opcao_config < TOTAL_SLOTS) {
                            game->slot_sendo_remapeado = game->opcao_config;
                        } else if (game->opcao_config == TOTAL_SLOTS) {
                            carregar_controles_padrao(&game->controles);
                        } else if (game->opcao_config == TOTAL_SLOTS + 1) {
                            game->estado_jogo = ESTADO_MENU_PRINCIPAL;
                        }
                        break;
                    }
                }
            }
            else if (game->estado_jogo == ESTADO_PAUSADO && evento.mouse.button == 1) {
                for (int i = 0; i < 3; i++) {
                    float item_y = 110 + (i * 25);
                    if (mapa_mouse_x >= 100 && mapa_mouse_x <= 380 && mapa_mouse_y >= item_y && mapa_mouse_y <= item_y + 20) {
                        game->opcao_pause = i;
                        if (game->opcao_pause == 0) {
                            game->estado_jogo = ESTADO_JOGANDO;
                        } else if (game->opcao_pause == 1) {
                            game->estado_jogo = ESTADO_MENU_PRINCIPAL;
                        } else if (game->opcao_pause == 2) {
                            game->rodando = false;
                        }
                        break;
                    }
                }
            }
            game->redesenhar = true;
        }
        else if (evento.type == ALLEGRO_EVENT_MOUSE_BUTTON_UP) {
            if (game->estado_jogo == ESTADO_JOGANDO && evento.mouse.button == 1) game->input.atirando = false;
        }
        else if (evento.type == ALLEGRO_EVENT_TIMER) {
            game->redesenhar = true;

            if (game->estado_jogo == ESTADO_JOGANDO) {
                game->anomalia.recebendo_calor = false;

                if (game->sala_atual == 1) {
                    float player_left = game->faxineiro.x - game->faxineiro.w / 2.0;
                    float player_right = game->faxineiro.x + game->faxineiro.w / 2.0;
                    float player_top = game->faxineiro.y - game->faxineiro.h / 2.0;
                    float player_bottom = game->faxineiro.y + game->faxineiro.h / 2.0;

                    float prop_x = 322.0;
                    float prop_y = 161.0;
                    float prop_w = 73.0;
                    float prop_h = 42.0;

                    bool encostando_prop = (player_right > prop_x) && (player_left < prop_x + prop_w) &&
                                           (player_bottom > prop_y) && (player_top < prop_y + prop_h);

                    if (!encostando_prop) {
                        game->prop_na_frente = (player_bottom < 200.0);
                    }
                }

                if (game->estado_fade == 1) {
                    game->alpha_fade += 15;
                    if (game->alpha_fade >= 255) { game->alpha_fade = 255; game->estado_fade = 2; game->timer_fade = 0; }
                }
                else if (game->estado_fade == 2) {
                    game->timer_fade++;
                    if (game->timer_fade == 30) {
                        game->sala_atual = game->proxima_sala;
                        game->faxineiro.x = game->proximo_x;
                        game->faxineiro.y = game->proximo_chao_y - (game->faxineiro.h / 2.0);
                        game->faxineiro.vel_y = 0;
                        game->faxineiro.no_chao = true;
                        salvar_jogo(game->nome_jogador, game->sala_atual, game->faxineiro.x, game->faxineiro.y, game->faxineiro.possui_arma);
                    }
                    if (game->timer_fade >= 60) game->estado_fade = 3;
                }
                else if (game->estado_fade == 3) {
                    game->alpha_fade -= 15;
                    if (game->alpha_fade <= 0) { game->alpha_fade = 0; game->estado_fade = 0; }
                }

                if (game->arma_mop.no_chao && game->arma_mop.sala_origem == game->sala_atual) {
                    game->arma_mop.vel_y += GRAVIDADE;
                    game->arma_mop.y += game->arma_mop.vel_y;
                    float largura_mop_sprite = game->arma_mop.sprite ? al_get_bitmap_width(game->arma_mop.sprite) : 10.0;
                    for (int p = 0; p < TOTAL_PLATAFORMAS; p++) {
                        if (game->mundo_plataformas[p].sala != game->sala_atual) continue;
                        Plataforma plat = game->mundo_plataformas[p];
                        if ((game->arma_mop.x + largura_mop_sprite / 2 > plat.x) && (game->arma_mop.x - largura_mop_sprite / 2 < plat.x + plat.w) &&
                            (game->arma_mop.y >= plat.y) && (game->arma_mop.y - game->arma_mop.vel_y <= plat.y + 5.0)) {
                            game->arma_mop.y = plat.y; game->arma_mop.vel_y = 0; break;
                        }
                    }
                }

                atualizar_jogador(&game->faxineiro, &game->input, &game->arma_mop, &game->anomalia,
                                  game->mundo_plataformas, TOTAL_PLATAFORMAS, game->mundo_escadas, TOTAL_ESCADAS,
                                  game->mundo_portas, TOTAL_PORTAS, game->projeteis, MAX_PROJETEIS,
                                  &game->sala_atual, &game->estado_fade, &game->proxima_sala, &game->proximo_x, &game->proximo_chao_y);

                atualizar_inimigo(&game->anomalia, &game->faxineiro,
                                  game->mundo_plataformas, TOTAL_PLATAFORMAS, game->mundo_escadas, TOTAL_ESCADAS,
                                  game->sala_atual, game->estado_fade, game->projeteis, MAX_PROJETEIS);
            }
        }

        if (game->redesenhar && al_is_event_queue_empty(game->fila_eventos)) {
            game->redesenhar = false;

            ALLEGRO_TRANSFORM camera_ui; al_identity_transform(&camera_ui); al_scale_transform(&camera_ui, (float)LARGURA_JANELA / LARGURA_MAPA, (float)ALTURA_JANELA / ALTURA_MAPA);
            float largura_camera = (game->estado_jogo == ESTADO_JOGANDO || game->estado_jogo == ESTADO_PAUSADO) && (game->sala_atual == 0) ? (LARGURA_MAPA / 2.0) : LARGURA_MAPA;
            float altura_camera = (game->estado_jogo == ESTADO_JOGANDO || game->estado_jogo == ESTADO_PAUSADO) && (game->sala_atual == 0) ? (ALTURA_MAPA / 2.0) : ALTURA_MAPA;
            ALLEGRO_TRANSFORM camera_jogo; al_identity_transform(&camera_jogo); al_scale_transform(&camera_jogo, (float)LARGURA_JANELA / largura_camera, (float)ALTURA_JANELA / altura_camera);

            al_clear_to_color(al_map_rgb(30, 30, 30));

            if (game->estado_jogo == ESTADO_MENU_PRINCIPAL || game->estado_jogo == ESTADO_CONFIRMAR_NOVO_JOGO || game->estado_jogo == ESTADO_DIGITAR_NOME || game->estado_jogo == ESTADO_RECORDES || game->estado_jogo == ESTADO_CONFIGURACOES) {
                al_use_transform(&camera_ui);
                desenhar_ui(game->estado_jogo, game->opcao_menu, game->opcao_confirma, game->opcao_pause,
                            game->opcao_config, game->slot_sendo_remapeado, &game->controles,
                            game->nome_jogador, game->fonte);
            }
            else if (game->estado_jogo == ESTADO_JOGANDO || game->estado_jogo == ESTADO_PAUSADO) {
                al_use_transform(&camera_jogo);
                desenhar_mundo(game->sala_atual, &game->faxineiro, &game->anomalia, &game->arma_mop,
                               game->projeteis, MAX_PROJETEIS, game->mundo_plataformas, TOTAL_PLATAFORMAS,
                               game->mundo_escadas, TOTAL_ESCADAS, game->mundo_portas, TOTAL_PORTAS,
                               &game->sprites, game->prop_na_frente, game->estado_fade, game->alpha_fade,
                               game->mostrar_hitbox, game->fonte, largura_camera, altura_camera);

                if (game->estado_jogo == ESTADO_PAUSADO) {
                    al_use_transform(&camera_ui);
                    desenhar_ui(game->estado_jogo, game->opcao_menu, game->opcao_confirma, game->opcao_pause,
                                game->opcao_config, game->slot_sendo_remapeado, &game->controles,
                                game->nome_jogador, game->fonte);
                }
            }

            al_flip_display();
        }
    }
}

void game_cleanup(Game *game) {
    if (game->sprites.sprite_prop1) al_destroy_bitmap(game->sprites.sprite_prop1);
    if (game->sprites.sprite_chao) al_destroy_bitmap(game->sprites.sprite_chao);
    if (game->sprites.sprite_escada) al_destroy_bitmap(game->sprites.sprite_escada);
    if (game->sprites.sprite_porta_fechada) al_destroy_bitmap(game->sprites.sprite_porta_fechada);
    if (game->sprites.sprite_porta_meio) al_destroy_bitmap(game->sprites.sprite_porta_meio);
    if (game->sprites.sprite_porta_aberta) al_destroy_bitmap(game->sprites.sprite_porta_aberta);
    if (game->sprites.sprite_porta_exit_fechada) al_destroy_bitmap(game->sprites.sprite_porta_exit_fechada);
    if (game->sprites.sprite_porta_exit_meio) al_destroy_bitmap(game->sprites.sprite_porta_exit_meio);
    if (game->sprites.sprite_porta_exit_aberta) al_destroy_bitmap(game->sprites.sprite_porta_exit_aberta);

    for (int f = 0; f < 4; f++) {
        if (game->faxineiro.sprites[f]) al_destroy_bitmap(game->faxineiro.sprites[f]);
    }
    for (int i = 0; i < 5; i++) {
        if (game->anomalia.sprites_normal[i]) al_destroy_bitmap(game->anomalia.sprites_normal[i]);
        if (game->anomalia.sprites_bre[i]) al_destroy_bitmap(game->anomalia.sprites_bre[i]);
    }
    for (int i = 0; i < MAX_SALAS; i++) {
        if (game->sprites.backgrounds[i]) al_destroy_bitmap(game->sprites.backgrounds[i]);
    }
    if (game->arma_mop.sprite) al_destroy_bitmap(game->arma_mop.sprite);
    if (game->sprites.sprite_water) al_destroy_bitmap(game->sprites.sprite_water);
    if (game->fonte) al_destroy_font(game->fonte);
    if (game->timer) al_destroy_timer(game->timer);
    if (game->fila_eventos) al_destroy_event_queue(game->fila_eventos);
    if (game->tela) al_destroy_display(game->tela);
}
