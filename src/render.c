#include "render.h"

void desenhar_mundo(int sala_atual, Jogador *faxineiro, Inimigo *anomalia, Item *arma_mop,
                    Projetil *projeteis, int max_proj, Plataforma *plataformas, int total_plat,
                    Escada *escadas, int total_escadas, Porta *portas, int total_portas,
                    GameSprites *sprites, bool prop_na_frente, int estado_fade, int alpha_fade,
                    bool mostrar_hitbox, ALLEGRO_FONT *fonte, float largura_camera, float altura_camera) {

    ALLEGRO_BITMAP *bg_ativo = sprites->backgrounds[sala_atual];
    if (bg_ativo) {
        float sw = al_get_bitmap_width(bg_ativo); float sh = al_get_bitmap_height(bg_ativo);
        al_draw_scaled_bitmap(bg_ativo, 0, 0, sw, sh, 0, 0, largura_camera, altura_camera, 0);
    }

    ALLEGRO_COLOR cor_porta = al_map_rgb(40, 50, 60);
    for (int p = 0; p < total_portas; p++) {
        if (portas[p].sala_origem != sala_atual) continue;
        Porta porta = portas[p];

        ALLEGRO_BITMAP *sprite_atual = NULL;

        if (porta.tipo == 0) {
            sprite_atual = sprites->sprite_porta_fechada;
            if (porta.timer_animacao > 0 && porta.timer_animacao < 30) sprite_atual = sprites->sprite_porta_meio;
            else if (porta.timer_animacao >= 30) sprite_atual = sprites->sprite_porta_aberta;
        } else if (porta.tipo == 1) {
            sprite_atual = sprites->sprite_porta_exit_fechada;
            if (porta.timer_animacao > 0 && porta.timer_animacao < 30) sprite_atual = sprites->sprite_porta_exit_meio;
            else if (porta.timer_animacao >= 30) sprite_atual = sprites->sprite_porta_exit_aberta;
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
    for (int p = 0; p < total_plat; p++) {
        if (plataformas[p].sala != sala_atual) continue;
        Plataforma plat = plataformas[p];

        if (sprites->sprite_chao) {
            float sw = al_get_bitmap_width(sprites->sprite_chao);
            float sh = al_get_bitmap_height(sprites->sprite_chao);
            for (float tx = 0; tx < plat.w; tx += sw) {
                float draw_w = (tx + sw > plat.w) ? (plat.w - tx) : sw;
                for (float ty = 0; ty < plat.h; ty += sh) {
                    float draw_h = (ty + sh > plat.h) ? (plat.h - ty) : sh;
                    al_draw_bitmap_region(sprites->sprite_chao, 0, 0, draw_w, draw_h, plat.x + tx, plat.y + ty, 0);
                }
            }
        } else {
            al_draw_filled_rectangle(plat.x, plat.y, plat.x + plat.w, plat.y + plat.h, cor_plat);
            al_draw_line(plat.x, plat.y, plat.x + plat.w, plat.y, cor_borda, 1.0);
        }
    }

    for (int e = 0; e < total_escadas; e++) {
        if (escadas[e].sala != sala_atual) continue;
        Escada esc = escadas[e];

        if (sprites->sprite_escada) {
            int flag = (esc.direcao == -1) ? ALLEGRO_FLIP_HORIZONTAL : 0;
            al_draw_bitmap(sprites->sprite_escada, esc.x, esc.y, flag);
        } else {
            if (esc.direcao == 1) {
                al_draw_filled_triangle(esc.x, esc.y + esc.h, esc.x + esc.w, esc.y + esc.h, esc.x + esc.w, esc.y, al_map_rgb(200, 200, 50));
            } else {
                al_draw_filled_triangle(esc.x, esc.y, esc.x, esc.y + esc.h, esc.x + esc.w, esc.y + esc.h, al_map_rgb(200, 200, 50));
            }
        }
    }

    if (sala_atual == 1 && !prop_na_frente) {
        if (sprites->sprite_prop1) {
            float pw = al_get_bitmap_width(sprites->sprite_prop1);
            float ph = al_get_bitmap_height(sprites->sprite_prop1);
            float novo_w = 73.0;
            float novo_h = 42.0;
            al_draw_scaled_bitmap(sprites->sprite_prop1, 0, 0, pw, ph, 322.0, 161.0, novo_w, novo_h, 0);
        }
    }

    if (sala_atual == anomalia->sala_origem) {
        ALLEGRO_BITMAP *sprite_galio = (anomalia->frame_animacao == 1) ? anomalia->sprites_bre[anomalia->frame_atual] : anomalia->sprites_normal[anomalia->frame_atual];
        int flag_galio_flip = (anomalia->direcao_atual == DIR_ESQUERDA) ? ALLEGRO_FLIP_HORIZONTAL : 0;
        if (sprite_galio != NULL) {
            float larg_img = al_get_bitmap_width(sprite_galio); float alt_img = al_get_bitmap_height(sprite_galio);
            al_draw_bitmap(sprite_galio, anomalia->x - (larg_img / 2), anomalia->y - (alt_img / 2), flag_galio_flip);
        } else {
            int color_val = 200 - (anomalia->frame_atual * 25);
            al_draw_filled_rectangle(anomalia->x - anomalia->w / 2, anomalia->y - anomalia->h / 2, anomalia->x + anomalia->w / 2, anomalia->y + anomalia->h / 2, al_map_rgb(color_val, color_val, color_val + 55));
        }
    }

    if (arma_mop->no_chao && arma_mop->sala_origem == sala_atual && arma_mop->sprite) {
        float larg_mop = al_get_bitmap_width(arma_mop->sprite); float alt_mop = al_get_bitmap_height(arma_mop->sprite);
        al_draw_bitmap(arma_mop->sprite, arma_mop->x - (larg_mop / 2), arma_mop->y - alt_mop, 0);
    }

    for (int i = 0; i < max_proj; i++) {
        if (projeteis[i].ativo) {
            if (projeteis[i].tipo == PROJETIL_AGUA && sprites->sprite_water) {
                float larg_w = al_get_bitmap_width(sprites->sprite_water); float alt_w = al_get_bitmap_height(sprites->sprite_water);
                float angulo_water = (projeteis[i].vel_x > 0) ? (ALLEGRO_PI / 2.0) : (-ALLEGRO_PI / 2.0);
                al_draw_rotated_bitmap(sprites->sprite_water, larg_w / 2.0, alt_w / 2.0, projeteis[i].x, projeteis[i].y, angulo_water, 0);
            } else {
                al_draw_filled_circle(projeteis[i].x, projeteis[i].y, 2, al_map_rgb(100, 200, 255));
            }
        }
    }

    int tipo_frame_desenhar = FRAME_IDLE;
    bool tem_sprites = (faxineiro->sprites[FRAME_IDLE] != NULL);
    if (tem_sprites) {
        if (faxineiro->esta_andando) {
            int sequencia_andar[] = {FRAME_WALK1, FRAME_IDLE, FRAME_WALK2, FRAME_IDLE};
            tipo_frame_desenhar = sequencia_andar[faxineiro->frame_atual];
        } else {
            int sequencia_parado[] = {FRAME_IDLE, FRAME_BRE};
            tipo_frame_desenhar = sequencia_parado[faxineiro->frame_atual];
        }
    }

    int flag_espelhamento = (faxineiro->direcao_atual == DIR_ESQUERDA) ? ALLEGRO_FLIP_HORIZONTAL : 0;

    if (tem_sprites) {
        ALLEGRO_BITMAP *sprite_final = faxineiro->sprites[tipo_frame_desenhar];
        if (sprite_final != NULL) {
            float larg_img = al_get_bitmap_width(sprite_final); float alt_img = al_get_bitmap_height(sprite_final);
            float draw_x = faxineiro->x - (larg_img / 2.0); float draw_y = (faxineiro->y + faxineiro->h / 2.0) - alt_img;
            al_draw_bitmap(sprite_final, draw_x, draw_y, flag_espelhamento);
        }
    }

    if (faxineiro->possui_arma && arma_mop->sprite) {
        float mao_x = faxineiro->x; float mao_y = faxineiro->y - (faxineiro->body_h / 2.0) + 4;
        if (tipo_frame_desenhar == FRAME_BRE) mao_y += 1; else if (tipo_frame_desenhar == FRAME_WALK1) mao_y -= 2;

        if (faxineiro->direcao_atual == DIR_DIREITA) mao_x -= 6; else mao_x += 6;

        float larg_mop = al_get_bitmap_width(arma_mop->sprite); float alt_mop = al_get_bitmap_height(arma_mop->sprite);
        float angulo_rotacao = (faxineiro->direcao_atual == DIR_DIREITA) ? (-ALLEGRO_PI / 2.0) : (ALLEGRO_PI / 2.0);
        al_draw_rotated_bitmap(arma_mop->sprite, larg_mop / 2.0, alt_mop / 2.0, mao_x, mao_y, angulo_rotacao, flag_espelhamento);
    }

    if (sala_atual == 1 && prop_na_frente) {
        if (sprites->sprite_prop1) {
            float pw = al_get_bitmap_width(sprites->sprite_prop1);
            float ph = al_get_bitmap_height(sprites->sprite_prop1);
            float novo_w = 73.0;
            float novo_h = 42.0;
            al_draw_scaled_bitmap(sprites->sprite_prop1, 0, 0, pw, ph, 322.0, 161.0, novo_w, novo_h, 0);
        }
    }

    if (estado_fade > 0) al_draw_filled_rectangle(0, 0, largura_camera, altura_camera, al_map_rgba(0, 0, 0, alpha_fade));

    if (mostrar_hitbox && estado_fade == 0) {
        al_draw_rectangle(faxineiro->x - faxineiro->w / 2, faxineiro->y - faxineiro->h / 2, faxineiro->x + faxineiro->w / 2, faxineiro->y + faxineiro->h / 2, al_map_rgb(0, 255, 0), 1.0);
        float body_top = (faxineiro->y + faxineiro->h / 2) - faxineiro->body_h;
        al_draw_rectangle(faxineiro->x - faxineiro->body_w / 2, body_top, faxineiro->x + faxineiro->body_w / 2, faxineiro->y + faxineiro->h / 2, al_map_rgb(0, 255, 255), 1.0);

        al_draw_text(fonte, al_map_rgb(255, 255, 0), largura_camera / 2.0, 10, ALLEGRO_ALIGN_CENTRE, "DEBUG MODE");
        al_draw_textf(fonte, al_map_rgb(0, 255, 255), largura_camera - 10, 10, ALLEGRO_ALIGN_RIGHT, "Ambiente: %.1f C", TEMP_ESTACAO);
        al_draw_textf(fonte, al_map_rgb(255, 150, 150), faxineiro->x, faxineiro->y - faxineiro->h / 2 - 15, ALLEGRO_ALIGN_CENTRE, "%.1f C", TEMP_JOGADOR);

        if (sala_atual == anomalia->sala_origem) {
            al_draw_textf(fonte, al_map_rgb(255, 150, 150), anomalia->x, anomalia->y - anomalia->h / 2 - 15, ALLEGRO_ALIGN_CENTRE, "%.1f C", anomalia->temperatura);
        }
    }
}

void desenhar_ui(int estado_jogo, int opcao_menu, int opcao_confirma, int opcao_pause,
                 int opcao_config, int slot_sendo_remapeado, Controles *controles,
                 const char *nome_jogador, ALLEGRO_FONT *fonte) {

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
        int teclas[TOTAL_SLOTS] = { controles->tecla_pular, controles->tecla_esquerda, controles->tecla_direita, controles->tecla_atirar, controles->tecla_pegar, controles->tecla_soltar, controles->tecla_entrar };
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
    else if (estado_jogo == ESTADO_PAUSADO) {
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
