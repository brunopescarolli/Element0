#include "render.h"

static const char *obter_nome_sala(int sala) {
    static const char *nomes[MAX_SALAS] = {
        "Limpeza (Andar 5)",
        "Armazem geral (Andar 5)",
        "Hall 5 (Andar 5)",
        "Hall 4 (Andar 4)",
        "Deposito quimico (Andar 5)",
        "Hall 6 (Andar 6)",
        "Hall 3 (Andar 3)",
        "Lab solidos (Andar 4)",
        "Contencao solidos (Andar 4)",
        "Entrada/saida carga (Andar 5)",
        "Estufa/reciclagem (Andar 6)",
        "Eletrica (Andar 6)",
        "Hall 2 (Andar 2)",
        "Lab liquidos (Andar 3)",
        "Contencao liquidos (Andar 3)",
        "Hall 1 (Andar 1)",
        "Lab gasosos (Andar 2)",
        "Contencao gasosos (Andar 2)",
        "Evacuacao (Andar 1)",
        "Area comum (Andar 1)",
        "Dormitorios (Andar 1)",
        "Ala medica (Andar 1)",
        "Central (Andar 1)",
        "Refeitorio (Andar 1)",
        "Banheiros (Andar 1)",
        "Quarentena (Andar 1)",
        "Escadaria 1 (Andar 1)",
        "Escadaria 2 (Andar 2)",
        "Escadaria 3 (Andar 3)",
        "Escadaria 4 (Andar 4)",
        "Escadaria 5 (Andar 5)",
        "Escadaria 6 (Andar 6)"
    };
    if (sala >= 0 && sala < MAX_SALAS) return nomes[sala];
    return "Desconhecida";
}

static unsigned char clamp_canal_rgb(float valor) {
    if (valor < 0.0f) return 0;
    if (valor > 255.0f) return 255;
    return (unsigned char)(valor + 0.5f);
}

ALLEGRO_COLOR obter_cor_temperatura(float temp) {
    float r = 255.0f;
    float g = 255.0f;
    float b = 255.0f;

    if (temp < 400.0f) {
        r = 255.0f;
        g = 255.0f;
        b = 255.0f;
    } else if (temp < 525.0f) {
        // 400 °C a 525 °C: Branco (255, 255, 255) -> Vermelho Bem Escuro (100, 0, 0)
        float t = (temp - 400.0f) / (525.0f - 400.0f);
        r = 255.0f + t * (100.0f - 255.0f);
        g = 255.0f + t * (0.0f - 255.0f);
        b = 255.0f + t * (0.0f - 255.0f);
    } else if (temp < 700.0f) {
        // 525 °C a 700 °C: Vermelho Bem Escuro (100, 0, 0) -> Vermelho Vivo (255, 0, 0)
        float t = (temp - 525.0f) / (700.0f - 525.0f);
        r = 100.0f + t * (255.0f - 100.0f);
        g = 0.0f;
        b = 0.0f;
    } else if (temp < 900.0f) {
        // 700 °C a 900 °C: Vermelho Vivo (255, 0, 0) -> Laranja (255, 127, 0)
        float t = (temp - 700.0f) / (900.0f - 700.0f);
        r = 255.0f;
        g = 0.0f + t * (127.0f - 0.0f);
        b = 0.0f;
    } else if (temp < 1100.0f) {
        // 900 °C a 1100 °C: Laranja (255, 127, 0) -> Amarelo (255, 255, 0)
        float t = (temp - 900.0f) / (1100.0f - 900.0f);
        r = 255.0f;
        g = 127.0f + t * (255.0f - 127.0f);
        b = 0.0f;
    } else if (temp < 1300.0f) {
        // 1100 °C a 1300 °C: Amarelo (255, 255, 0) -> Branco Brilhante (255, 255, 255)
        float t = (temp - 1100.0f) / (1300.0f - 1100.0f);
        r = 255.0f;
        g = 255.0f;
        b = 0.0f + t * (255.0f - 0.0f);
    } else {
        // 1300 °C+: Estado de incandescência total (Branco Brilhante)
        r = 255.0f;
        g = 255.0f;
        b = 255.0f;
    }

    return al_map_rgb(clamp_canal_rgb(r), clamp_canal_rgb(g), clamp_canal_rgb(b));
}

void desenhar_mundo(int sala_atual, Jogador *faxineiro, Inimigo *anomalia, Item *arma_mop, Item *arma_soprador,
                    Soprador *soprador, Coletavel *refil_agua, Coletavel *bateria, Coletavel *kit_medico,
                    Projetil *projeteis, int max_proj, Plataforma *plataformas, int total_plat,
                    Escada *escadas, int total_escadas, Porta *portas, int total_portas,
                    Elevador *elevadores, int total_elevadores, TextoFlutuante *textos, int max_textos,
                    GameSprites *sprites, bool prop_na_frente, int estado_fade, int alpha_fade,
                    float alpha_game_over, int opcao_game_over,
                    bool mostrar_hitbox, ALLEGRO_FONT *fonte, float largura_camera, float altura_camera) {

    bool eh_escadaria = (sala_atual >= 26 && sala_atual <= 31);
    al_draw_filled_rectangle(0.0f, 0.0f, largura_camera, altura_camera, al_map_rgb(30, 30, 30));

    if (eh_escadaria) {
        if (sprites->bg_escadaria != NULL) {
            float sw = al_get_bitmap_width(sprites->bg_escadaria);
            float sh = al_get_bitmap_height(sprites->bg_escadaria);
            al_draw_scaled_bitmap(sprites->bg_escadaria, 0.0f, 0.0f, sw, sh, 0.0f, 0.0f, largura_camera, altura_camera, 0);
        }
    } else {
        ALLEGRO_BITMAP *bg_ativo = sprites->backgrounds[sala_atual];
        if (!bg_ativo && (sala_atual == 3 || sala_atual == 5 || sala_atual == 6 || sala_atual == 12 || sala_atual == 15)) {
            bg_ativo = sprites->backgrounds[2];
        }
        if (bg_ativo != NULL) {
            float sw = al_get_bitmap_width(bg_ativo);
            float sh = al_get_bitmap_height(bg_ativo);
            al_draw_scaled_bitmap(bg_ativo, 0.0f, 0.0f, sw, sh, 0.0f, 0.0f, largura_camera, altura_camera, 0);
        }
    }

    if (eh_escadaria && fonte) {
        int andar_atual = sala_atual - 25;
        float porta_x = 416.0f;
        float porta_y = 84.0f;
        for (int p = 0; p < total_portas; p++) {
            if (portas[p].sala_origem == sala_atual && portas[p].tipo != 2) {
                porta_x = portas[p].x;
                porta_y = portas[p].y;
                break;
            }
        }

        float texto_x = porta_x - 90.0f;
        float texto_y = porta_y + 22.0f;
        float escala_texto = 3.5f;

        ALLEGRO_TRANSFORM transform_anterior = *al_get_current_transform();
        ALLEGRO_TRANSFORM transform_texto;
        al_build_transform(&transform_texto, texto_x, texto_y, escala_texto, escala_texto, 0.0f);
        al_compose_transform(&transform_texto, &transform_anterior);
        al_use_transform(&transform_texto);

        al_draw_textf(fonte, al_map_rgba(210, 220, 230, 190), 0.0f, 0.0f, ALLEGRO_ALIGN_CENTRE, "%02d", andar_atual);

        al_use_transform(&transform_anterior);
    }

    for (int e = 0; e < total_elevadores; e++) {
        if (elevadores[e].sala != sala_atual) continue;
        Elevador elev = elevadores[e];
        ALLEGRO_COLOR cor_preenchimento = (elev.tipo == ELEVADOR_INDUSTRIAL)
            ? al_map_rgb(38, 32, 30)
            : al_map_rgb(24, 32, 44);
        ALLEGRO_COLOR cor_borda_elev = (elev.tipo == ELEVADOR_INDUSTRIAL)
            ? al_map_rgb(18, 15, 14)
            : al_map_rgb(14, 18, 26);

        al_draw_filled_rectangle(elev.x, elev.y, elev.x + elev.w, elev.y + elev.h, cor_preenchimento);
        al_draw_rectangle(elev.x, elev.y, elev.x + elev.w, elev.y + elev.h, cor_borda_elev, 2.0);
        al_draw_line(elev.x + elev.w / 2.0f, elev.y, elev.x + elev.w / 2.0f, elev.y + elev.h, cor_borda_elev, 1.0);
    }

    ALLEGRO_COLOR cor_porta = al_map_rgb(40, 50, 60);
    for (int p = 0; p < total_portas; p++) {
        if (portas[p].sala_origem != sala_atual) continue;
        Porta porta = portas[p];
        if (porta.tipo == 2) continue;

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
        if (eh_escadaria && sprites->bg_escadaria) continue;
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

        if (esc.direcao == 0) {
            if (sprites->sprite_escada_vertical) {
                float sw = al_get_bitmap_width(sprites->sprite_escada_vertical);
                float sh = al_get_bitmap_height(sprites->sprite_escada_vertical);
                for (float ty = 0; ty < esc.h; ty += sh) {
                    float draw_h = (ty + sh > esc.h) ? (esc.h - ty) : sh;
                    al_draw_scaled_bitmap(sprites->sprite_escada_vertical, 0, 0, sw, draw_h, esc.x, esc.y + ty, esc.w, draw_h, 0);
                }
            } else if (!eh_escadaria || !sprites->bg_escadaria) {
                ALLEGRO_COLOR cor_aste = al_map_rgb(130, 145, 160);
                ALLEGRO_COLOR cor_degrau = al_map_rgb(185, 200, 215);
                al_draw_filled_rectangle(esc.x, esc.y, esc.x + 3.0, esc.y + esc.h, cor_aste);
                al_draw_filled_rectangle(esc.x + esc.w - 3.0, esc.y, esc.x + esc.w, esc.y + esc.h, cor_aste);
                for (float ry = esc.y + 6.0; ry < esc.y + esc.h - 2.0; ry += 12.0) {
                    al_draw_filled_rectangle(esc.x + 3.0, ry, esc.x + esc.w - 3.0, ry + 2.0, cor_degrau);
                }
            }
        } else if (sprites->sprite_escada) {
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
        ALLEGRO_COLOR cor_temp = obter_cor_temperatura(anomalia->temperatura);

        if (sprite_galio != NULL) {
            float larg_img = al_get_bitmap_width(sprite_galio);
            float alt_img = al_get_bitmap_height(sprite_galio);
            al_draw_tinted_bitmap(sprite_galio, cor_temp,
                                  anomalia->x - (larg_img / 2.0f), anomalia->y - (alt_img / 2.0f), flag_galio_flip);
        } else {
            ALLEGRO_COLOR cor_prim = cor_temp;
            if (anomalia->temperatura < 400.0f) {
                unsigned char tom_azul = clamp_canal_rgb((float)(200 - (anomalia->frame_atual * 25)));
                unsigned char tom_azul_b = clamp_canal_rgb((float)(tom_azul + 55));
                cor_prim = al_map_rgb(tom_azul, tom_azul, tom_azul_b);
            }
            al_draw_filled_rectangle(anomalia->x - anomalia->w / 2.0f, anomalia->y - anomalia->h / 2.0f,
                                     anomalia->x + anomalia->w / 2.0f, anomalia->y + anomalia->h / 2.0f,
                                     cor_prim);
        }
    }

    if (arma_mop && arma_mop->no_chao && arma_mop->sala_origem == sala_atual && arma_mop->sprite) {
        float larg_mop = al_get_bitmap_width(arma_mop->sprite); float alt_mop = al_get_bitmap_height(arma_mop->sprite);
        al_draw_bitmap(arma_mop->sprite, arma_mop->x - (larg_mop / 2), arma_mop->y - alt_mop, 0);
    }

    if (arma_soprador && arma_soprador->no_chao && arma_soprador->sala_origem == sala_atual) {
        al_draw_filled_rectangle(arma_soprador->x - 7.0f, arma_soprador->y - 8.0f, arma_soprador->x + 5.0f, arma_soprador->y - 2.0f, al_map_rgb(220, 100, 20));
        al_draw_filled_rectangle(arma_soprador->x + 5.0f, arma_soprador->y - 7.0f, arma_soprador->x + 9.0f, arma_soprador->y - 3.0f, al_map_rgb(180, 180, 180));
        al_draw_filled_rectangle(arma_soprador->x - 5.0f, arma_soprador->y - 2.0f, arma_soprador->x - 2.0f, arma_soprador->y, al_map_rgb(50, 50, 50));
        al_draw_rectangle(arma_soprador->x - 7.0f, arma_soprador->y - 8.0f, arma_soprador->x + 5.0f, arma_soprador->y - 2.0f, al_map_rgb(40, 40, 40), 1.0f);
    }

    if (refil_agua && refil_agua->ativo && refil_agua->sala == sala_atual) {
        al_draw_filled_rectangle(refil_agua->x, refil_agua->y, refil_agua->x + refil_agua->w, refil_agua->y + refil_agua->h, al_map_rgb(60, 160, 255));
        al_draw_rectangle(refil_agua->x, refil_agua->y, refil_agua->x + refil_agua->w, refil_agua->y + refil_agua->h, al_map_rgb(255, 255, 255), 1.0f);
    }

    if (bateria && bateria->ativo && bateria->sala == sala_atual) {
        al_draw_filled_rectangle(bateria->x, bateria->y, bateria->x + bateria->w, bateria->y + bateria->h, al_map_rgb(255, 220, 50));
        al_draw_rectangle(bateria->x, bateria->y, bateria->x + bateria->w, bateria->y + bateria->h, al_map_rgb(255, 255, 255), 1.0f);
    }

    if (kit_medico && kit_medico->ativo && kit_medico->sala == sala_atual) {
        al_draw_filled_rectangle(kit_medico->x, kit_medico->y, kit_medico->x + kit_medico->w, kit_medico->y + kit_medico->h, al_map_rgb(240, 240, 240));
        al_draw_rectangle(kit_medico->x, kit_medico->y, kit_medico->x + kit_medico->w, kit_medico->y + kit_medico->h, al_map_rgb(60, 60, 60), 1.0f);
        float cx = kit_medico->x + kit_medico->w / 2.0f;
        float cy = kit_medico->y + kit_medico->h / 2.0f;
        al_draw_filled_rectangle(cx - 1.0f, cy - 4.0f, cx + 1.0f, cy + 4.0f, al_map_rgb(220, 30, 30));
        al_draw_filled_rectangle(cx - 4.0f, cy - 1.0f, cx + 4.0f, cy + 1.0f, al_map_rgb(220, 30, 30));
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

    // Arma NÃO equipada: desenhada nas costas em posição vertical (atrás do corpo)
    // Deslocada para CIMA em 50% da altura do sprite/placeholder da arma
    float costas_x = (faxineiro->direcao_atual == DIR_DIREITA) ? (faxineiro->x - 5.0f) : (faxineiro->x + 5.0f);
    float costas_y = faxineiro->y - (faxineiro->body_h / 4.0f);

    if (faxineiro->arma_equipada == ARMA_MOP && faxineiro->possui_soprador) {
        // Soprador Térmico nas costas (vertical, deslocado 50% para cima: altura 16 -> sobe 8px)
        float sop_y = costas_y - 8.0f;
        al_draw_filled_rectangle(costas_x - 3.0f, sop_y - 8.0f, costas_x + 3.0f, sop_y + 8.0f, al_map_rgb(220, 100, 20));
        al_draw_filled_rectangle(costas_x - 2.0f, sop_y - 12.0f, costas_x + 2.0f, sop_y - 8.0f, al_map_rgb(170, 170, 170));
        al_draw_rectangle(costas_x - 3.0f, sop_y - 8.0f, costas_x + 3.0f, sop_y + 8.0f, al_map_rgb(40, 40, 40), 1.0f);
    } else if (faxineiro->arma_equipada == ARMA_SOPRADOR && faxineiro->possui_mop) {
        // Mop nas costas (vertical, deslocado 50% para cima)
        if (arma_mop && arma_mop->sprite) {
            float larg_mop = al_get_bitmap_width(arma_mop->sprite);
            float alt_mop = al_get_bitmap_height(arma_mop->sprite);
            float mop_y = costas_y - (alt_mop * 0.5f);
            al_draw_bitmap(arma_mop->sprite, costas_x - (larg_mop / 2.0f), mop_y - (alt_mop / 2.0f), flag_espelhamento);
        } else {
            float alt_mop = 28.0f;
            float mop_y = costas_y - (alt_mop * 0.5f);
            al_draw_filled_rectangle(costas_x - 2.0f, mop_y - 14.0f, costas_x + 2.0f, mop_y + 14.0f, al_map_rgb(180, 180, 180));
        }
    }

    bool visivel_dano = faxineiro->morreu || (faxineiro->timer_dano <= 0) || (((faxineiro->timer_dano / 5) % 2) == 0);

    if (visivel_dano) {
        if (tem_sprites) {
            ALLEGRO_BITMAP *sprite_final = faxineiro->sprites[tipo_frame_desenhar];
            if (sprite_final != NULL) {
                float larg_img = al_get_bitmap_width(sprite_final);
                float alt_img = al_get_bitmap_height(sprite_final);
                if (faxineiro->morreu) {
                    float chao_y = faxineiro->y + (faxineiro->h / 2.0f);
                    al_draw_rotated_bitmap(sprite_final, larg_img / 2.0f, alt_img / 2.0f,
                                           faxineiro->x, chao_y - (faxineiro->body_w / 2.0f),
                                           ALLEGRO_PI / 2.0f, flag_espelhamento);
                } else {
                    float draw_x = faxineiro->x - (larg_img / 2.0f);
                    float draw_y = (faxineiro->y + faxineiro->h / 2.0f) - alt_img;
                    al_draw_bitmap(sprite_final, draw_x, draw_y, flag_espelhamento);
                }
            }
        } else {
            float chao_y = faxineiro->y + (faxineiro->h / 2.0f);
            if (faxineiro->morreu) {
                al_draw_filled_rectangle(faxineiro->x - faxineiro->body_h / 2.0f, chao_y - faxineiro->body_w,
                                         faxineiro->x + faxineiro->body_h / 2.0f, chao_y, al_map_rgb(100, 150, 255));
            } else {
                al_draw_filled_rectangle(faxineiro->x - faxineiro->body_w / 2.0f, chao_y - faxineiro->body_h,
                                         faxineiro->x + faxineiro->body_w / 2.0f, chao_y, al_map_rgb(100, 150, 255));
            }
        }
    }

    // Arma EQUIPADA: desenhada na mão do personagem (independentemente de possuir o Mop ou não)
    float mao_y = faxineiro->y - (faxineiro->body_h / 2.0) + 4;
    if (tipo_frame_desenhar == FRAME_BRE) mao_y += 1; else if (tipo_frame_desenhar == FRAME_WALK1) mao_y -= 2;

    float offset_arma_x = (faxineiro->direcao_atual == DIR_DIREITA) ? 7.0f : -7.0f;

    if (faxineiro->arma_equipada == ARMA_MOP && faxineiro->possui_mop && arma_mop && arma_mop->sprite) {
        float mao_x = ((faxineiro->direcao_atual == DIR_DIREITA) ? (faxineiro->x - 6.0f) : (faxineiro->x + 6.0f)) + offset_arma_x;
        float larg_mop = al_get_bitmap_width(arma_mop->sprite); float alt_mop = al_get_bitmap_height(arma_mop->sprite);
        float angulo_rotacao = (faxineiro->direcao_atual == DIR_DIREITA) ? (-ALLEGRO_PI / 2.0) : (ALLEGRO_PI / 2.0);
        al_draw_rotated_bitmap(arma_mop->sprite, larg_mop / 2.0, alt_mop / 2.0, mao_x, mao_y, angulo_rotacao, flag_espelhamento);
    } else if (faxineiro->arma_equipada == ARMA_SOPRADOR && faxineiro->possui_soprador) {
        float hand_x = ((faxineiro->direcao_atual == DIR_DIREITA) ? (faxineiro->x + 6.0f) : (faxineiro->x - 6.0f)) + offset_arma_x;
        if (faxineiro->direcao_atual == DIR_DIREITA) {
            al_draw_filled_rectangle(hand_x - 2.0f, mao_y - 3.0f, hand_x + 10.0f, mao_y + 3.0f, al_map_rgb(220, 100, 20));
            al_draw_filled_rectangle(hand_x + 10.0f, mao_y - 2.0f, hand_x + 14.0f, mao_y + 2.0f, al_map_rgb(180, 180, 180));
            al_draw_filled_rectangle(hand_x, mao_y + 3.0f, hand_x + 3.0f, mao_y + 7.0f, al_map_rgb(50, 50, 50));
            al_draw_rectangle(hand_x - 2.0f, mao_y - 3.0f, hand_x + 10.0f, mao_y + 3.0f, al_map_rgb(40, 40, 40), 1.0f);
        } else {
            al_draw_filled_rectangle(hand_x - 10.0f, mao_y - 3.0f, hand_x + 2.0f, mao_y + 3.0f, al_map_rgb(220, 100, 20));
            al_draw_filled_rectangle(hand_x - 14.0f, mao_y - 2.0f, hand_x - 10.0f, mao_y + 2.0f, al_map_rgb(180, 180, 180));
            al_draw_filled_rectangle(hand_x - 3.0f, mao_y + 3.0f, hand_x, mao_y + 7.0f, al_map_rgb(50, 50, 50));
            al_draw_rectangle(hand_x - 10.0f, mao_y - 3.0f, hand_x + 2.0f, mao_y + 3.0f, al_map_rgb(40, 40, 40), 1.0f);
        }

        // Hitbox de chamas (placeholder) de curto alcance
        if (soprador && soprador->ativo) {
            al_draw_filled_rectangle(soprador->x, soprador->y, soprador->x + soprador->w, soprador->y + soprador->h, al_map_rgba(255, 120, 0, 160));
            al_draw_filled_rectangle(soprador->x + 2.0f, soprador->y + 2.0f, soprador->x + soprador->w - 2.0f, soprador->y + soprador->h - 2.0f, al_map_rgba(255, 230, 40, 200));
            al_draw_rectangle(soprador->x, soprador->y, soprador->x + soprador->w, soprador->y + soprador->h, al_map_rgb(255, 80, 0), 1.0f);
        }
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

    for (int t = 0; t < max_textos; t++) {
        if (!textos[t].ativo) continue;
        unsigned char opacidade = (unsigned char)textos[t].alpha;
        unsigned char cr, cg, cb;
        al_unmap_rgb(textos[t].cor, &cr, &cg, &cb);
        al_draw_text(fonte, al_map_rgba(cr, cg, cb, opacidade), textos[t].x, textos[t].y, ALLEGRO_ALIGN_CENTRE, textos[t].texto);
    }

    if (mostrar_hitbox && estado_fade == 0) {
        al_draw_rectangle(faxineiro->x - faxineiro->w / 2, faxineiro->y - faxineiro->h / 2, faxineiro->x + faxineiro->w / 2, faxineiro->y + faxineiro->h / 2, al_map_rgb(0, 255, 0), 1.0);
        float body_top = (faxineiro->y + faxineiro->h / 2) - faxineiro->body_h;
        al_draw_rectangle(faxineiro->x - faxineiro->body_w / 2, body_top, faxineiro->x + faxineiro->body_w / 2, faxineiro->y + faxineiro->h / 2, al_map_rgb(0, 255, 255), 1.0);

        if (refil_agua && refil_agua->ativo && refil_agua->sala == sala_atual) {
            al_draw_rectangle(refil_agua->x, refil_agua->y, refil_agua->x + refil_agua->w, refil_agua->y + refil_agua->h, al_map_rgb(60, 160, 255), 1.0f);
            al_draw_text(fonte, al_map_rgb(100, 200, 255), refil_agua->x + refil_agua->w / 2.0f, refil_agua->y - 10, ALLEGRO_ALIGN_CENTRE, "Refil Agua");
        }
        if (bateria && bateria->ativo && bateria->sala == sala_atual) {
            al_draw_rectangle(bateria->x, bateria->y, bateria->x + bateria->w, bateria->y + bateria->h, al_map_rgb(255, 220, 50), 1.0f);
            al_draw_text(fonte, al_map_rgb(255, 220, 50), bateria->x + bateria->w / 2.0f, bateria->y - 10, ALLEGRO_ALIGN_CENTRE, "Bateria");
        }
        if (kit_medico && kit_medico->ativo && kit_medico->sala == sala_atual) {
            al_draw_rectangle(kit_medico->x, kit_medico->y, kit_medico->x + kit_medico->w, kit_medico->y + kit_medico->h, al_map_rgb(255, 100, 100), 1.0f);
            al_draw_text(fonte, al_map_rgb(255, 100, 100), kit_medico->x + kit_medico->w / 2.0f, kit_medico->y - 10, ALLEGRO_ALIGN_CENTRE, "Kit Medico");
        }
        if (arma_soprador && arma_soprador->no_chao && arma_soprador->sala_origem == sala_atual) {
            al_draw_rectangle(arma_soprador->x - 7.0f, arma_soprador->y - 8.0f, arma_soprador->x + 9.0f, arma_soprador->y, al_map_rgb(255, 150, 50), 1.0f);
            al_draw_text(fonte, al_map_rgb(255, 150, 50), arma_soprador->x + 1.0f, arma_soprador->y - 18, ALLEGRO_ALIGN_CENTRE, "Soprador");
        }

        for (int e = 0; e < total_escadas; e++) {
            if (escadas[e].sala != sala_atual) continue;
            Escada esc = escadas[e];
            al_draw_rectangle(esc.x, esc.y, esc.x + esc.w, esc.y + esc.h, al_map_rgb(100, 255, 180), 1.0);
        }

        for (int p = 0; p < total_portas; p++) {
            if (portas[p].sala_origem != sala_atual) continue;
            Porta porta = portas[p];
            ALLEGRO_COLOR cor_hit_porta = (porta.tipo == 2) ? al_map_rgb(255, 100, 255) : al_map_rgb(255, 200, 0);
            al_draw_rectangle(porta.x, porta.y, porta.x + porta.w, porta.y + porta.h, cor_hit_porta, 1.0);
            float tx = porta.x + porta.w / 2.0;
            int align = ALLEGRO_ALIGN_CENTRE;
            if (porta.x < 60) { tx = porta.x + 2; align = ALLEGRO_ALIGN_LEFT; }
            else if (porta.x + porta.w > largura_camera - 60) { tx = porta.x + porta.w - 2; align = ALLEGRO_ALIGN_RIGHT; }
            float ty = (porta.y > 14) ? (porta.y - 12) : (porta.y + porta.h + 2);
            if (porta.tipo == 2) {
                const char *acao = (porta.y < ALTURA_MAPA / 2.0) ? "Subir" : "Descer";
                al_draw_textf(fonte, al_map_rgb(255, 150, 255), porta.x + porta.w + 4, (porta.y < ALTURA_MAPA / 2.0) ? (porta.y + 4) : (porta.y - 10), ALLEGRO_ALIGN_LEFT, "%s [%d]", acao, porta.destino_sala);
            } else if (sala_atual >= 26 && sala_atual <= 31) {
                al_draw_textf(fonte, al_map_rgb(255, 220, 100), tx, ty, align, "Voltar ao Hall [%d]", porta.destino_sala);
            } else {
                al_draw_textf(fonte, al_map_rgb(255, 220, 100), tx, ty, align, "[%d]", porta.destino_sala);
            }
            if (porta.jogador_perto) {
                // Deslocado para Y=84 para não sobrepor o HUD
                al_draw_textf(fonte, al_map_rgb(255, 220, 100), 10, 84, ALLEGRO_ALIGN_LEFT, "Porta -> Sala %d: %s", porta.destino_sala, obter_nome_sala(porta.destino_sala));
            }
        }

        al_draw_text(fonte, al_map_rgb(255, 255, 0), largura_camera / 2.0, 10, ALLEGRO_ALIGN_CENTRE, "DEBUG MODE");
        // Deslocado para Y=72 para não sobrepor o HUD de status
        al_draw_textf(fonte, al_map_rgb(0, 255, 100), 10, 72, ALLEGRO_ALIGN_LEFT, "Sala %d: %s", sala_atual, obter_nome_sala(sala_atual));
        al_draw_textf(fonte, al_map_rgb(0, 255, 255), largura_camera - 10, 10, ALLEGRO_ALIGN_RIGHT, "Ambiente: %.1f C", TEMP_ESTACAO);
        al_draw_textf(fonte, al_map_rgb(255, 150, 150), faxineiro->x, faxineiro->y - faxineiro->h / 2 - 15, ALLEGRO_ALIGN_CENTRE, "%.1f C", TEMP_JOGADOR);

        if (sala_atual == anomalia->sala_origem) {
            al_draw_textf(fonte, al_map_rgb(255, 150, 150), anomalia->x, anomalia->y - anomalia->h / 2 - 15, ALLEGRO_ALIGN_CENTRE, "%.1f C", anomalia->temperatura);
        }
    }

    // Transformação para UI em coordenadas de tela fixas (480x270)
    ALLEGRO_TRANSFORM camera_ui;
    al_identity_transform(&camera_ui);
    al_scale_transform(&camera_ui, (float)LARGURA_JANELA / LARGURA_MAPA, (float)ALTURA_JANELA / ALTURA_MAPA);
    al_use_transform(&camera_ui);

    // --- HUD Superior Esquerdo: Status do Jogador ---
    // Barra de Vida (começa cheia, esvazia ao tomar dano)
    al_draw_text(fonte, al_map_rgb(200, 255, 200), 10, 10, ALLEGRO_ALIGN_LEFT, "VIDA");
    al_draw_filled_rectangle(45, 10, 45 + 70, 10 + 7, al_map_rgb(35, 35, 35));
    float pct_vida = faxineiro->barra_vida / 100.0f;
    if (pct_vida < 0.0f) pct_vida = 0.0f;
    if (pct_vida > 1.0f) pct_vida = 1.0f;
    al_draw_filled_rectangle(45, 10, 45 + (70.0f * pct_vida), 10 + 7, al_map_rgb(50, 220, 50));
    al_draw_rectangle(45, 10, 45 + 70, 10 + 7, al_map_rgb(220, 220, 220), 1.0f);

    // Barra de Radiação: SÓ desenhada se o valor for maior que 0
    if (faxineiro->barra_radio > 0.0f) {
        al_draw_text(fonte, al_map_rgb(220, 255, 100), 10, 21, ALLEGRO_ALIGN_LEFT, "RAD");
        al_draw_filled_rectangle(45, 21, 45 + 70, 21 + 7, al_map_rgb(35, 35, 35));
        float pct_radio = faxineiro->barra_radio / 100.0f;
        if (pct_radio < 0.0f) pct_radio = 0.0f;
        if (pct_radio > 1.0f) pct_radio = 1.0f;
        al_draw_filled_rectangle(45, 21, 45 + (70.0f * pct_radio), 21 + 7, al_map_rgb(200, 220, 20));
        al_draw_rectangle(45, 21, 45 + 70, 21 + 7, al_map_rgb(220, 220, 220), 1.0f);
    }

    // --- HUD Inferior Direito: Munição e Inventário da Arma Equipada ---
    // Reposicionada para baixo em 1x a própria altura (y = 252 + 7 = 259)
    if (faxineiro->arma_equipada == ARMA_MOP && faxineiro->possui_mop) {
        // Ícones de inventário: até 3 refis de água empilhados horizontalmente logo acima da barra
        for (int i = 0; i < 3; i++) {
            float slot_x = 390.0f + (i * 9.0f);
            float slot_y = 250.0f;
            al_draw_filled_rectangle(slot_x, slot_y, slot_x + 6.0f, slot_y + 6.0f, al_map_rgb(25, 25, 25));
            al_draw_rectangle(slot_x, slot_y, slot_x + 6.0f, slot_y + 6.0f, al_map_rgb(70, 70, 70), 1.0f);
            if (i < faxineiro->refil_agua_qtd) {
                al_draw_filled_rectangle(slot_x + 1.0f, slot_y + 1.0f, slot_x + 5.0f, slot_y + 5.0f, al_map_rgb(60, 160, 255));
            }
        }

        al_draw_text(fonte, al_map_rgb(120, 200, 255), 350, 259, ALLEGRO_ALIGN_LEFT, "AGUA");
        al_draw_filled_rectangle(390, 259, 390 + 70, 259 + 7, al_map_rgb(35, 35, 35));
        float pct_agua = faxineiro->barra_agua / 100.0f;
        if (pct_agua < 0.0f) pct_agua = 0.0f;
        if (pct_agua > 1.0f) pct_agua = 1.0f;
        al_draw_filled_rectangle(390, 259, 390 + (70.0f * pct_agua), 259 + 7, al_map_rgb(60, 160, 255));
        al_draw_rectangle(390, 259, 390 + 70, 259 + 7, al_map_rgb(220, 220, 220), 1.0f);
    } else if (faxineiro->arma_equipada == ARMA_SOPRADOR && faxineiro->possui_soprador) {
        // Ícones de inventário: até 3 baterias empilhadas horizontalmente logo acima da barra
        for (int i = 0; i < 3; i++) {
            float slot_x = 390.0f + (i * 9.0f);
            float slot_y = 250.0f;
            al_draw_filled_rectangle(slot_x, slot_y, slot_x + 6.0f, slot_y + 6.0f, al_map_rgb(25, 25, 25));
            al_draw_rectangle(slot_x, slot_y, slot_x + 6.0f, slot_y + 6.0f, al_map_rgb(70, 70, 70), 1.0f);
            if (i < faxineiro->bateria_qtd) {
                al_draw_filled_rectangle(slot_x + 1.0f, slot_y + 1.0f, slot_x + 5.0f, slot_y + 5.0f, al_map_rgb(255, 200, 20));
            }
        }

        al_draw_text(fonte, al_map_rgb(255, 220, 60), 330, 259, ALLEGRO_ALIGN_LEFT, "BATERIA");
        al_draw_filled_rectangle(390, 259, 390 + 70, 259 + 7, al_map_rgb(35, 35, 35));
        float pct_bat = faxineiro->barra_bateria / 100.0f;
        if (pct_bat < 0.0f) pct_bat = 0.0f;
        if (pct_bat > 1.0f) pct_bat = 1.0f;
        al_draw_filled_rectangle(390, 259, 390 + (70.0f * pct_bat), 259 + 7, al_map_rgb(255, 200, 20));
        al_draw_rectangle(390, 259, 390 + 70, 259 + 7, al_map_rgb(220, 220, 220), 1.0f);
    }

    // Fade entre salas (cobre a tela inteira em camera_ui)
    if (estado_fade > 0) al_draw_filled_rectangle(0, 0, LARGURA_MAPA, ALTURA_MAPA, al_map_rgba(0, 0, 0, alpha_fade));

    // Overlay de Game Over ("VOCÊ MORREU")
    if (faxineiro->morreu) {
        float a_clamped = alpha_game_over;
        if (a_clamped < 0.0f) a_clamped = 0.0f;
        if (a_clamped > 255.0f) a_clamped = 255.0f;
        unsigned char alpha_bg = (unsigned char)a_clamped;

        al_draw_filled_rectangle(0, 0, LARGURA_MAPA, ALTURA_MAPA, al_map_rgba(0, 0, 0, alpha_bg));

        // Texto "VOCÊ MORREU" centralizado em destaque (escala 2.0x)
        ALLEGRO_TRANSFORM trans_ant = *al_get_current_transform();
        ALLEGRO_TRANSFORM trans_titulo;
        al_build_transform(&trans_titulo, LARGURA_MAPA / 2.0f, 85.0f, 2.0f, 2.0f, 0.0f);
        al_compose_transform(&trans_titulo, &trans_ant);
        al_use_transform(&trans_titulo);
        al_draw_text(fonte, al_map_rgba(220, 40, 40, alpha_bg), 0.0f, 0.0f, ALLEGRO_ALIGN_CENTRE, "VOCÊ MORREU");
        al_use_transform(&trans_ant);

        const char *botoes_morte[2] = { "Novo Jogo", "Menu Principal" };
        for (int i = 0; i < 2; i++) {
            float btn_y1 = 140.0f + (i * 32.0f);
            float btn_y2 = btn_y1 + 22.0f;
            ALLEGRO_COLOR cor_btn = (opcao_game_over == i)
                ? al_map_rgba(100, 255, 100, alpha_bg)
                : al_map_rgba(200, 200, 200, alpha_bg);
            al_draw_rectangle(160.0f, btn_y1, 320.0f, btn_y2, cor_btn, 1.5f);
            al_draw_text(fonte, cor_btn, LARGURA_MAPA / 2.0f, btn_y1 + 7.0f, ALLEGRO_ALIGN_CENTRE, botoes_morte[i]);
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
