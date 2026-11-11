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

static float lerp_float(float a, float b, float ratio) {
    if (ratio < 0.0f) ratio = 0.0f;
    if (ratio > 1.0f) ratio = 1.0f;
    return a + (b - a) * ratio;
}

void obter_cores_temperatura(float temp, ALLEGRO_COLOR *cor_borda, ALLEGRO_COLOR *cor_centro) {
    float r_borda = 255.0f, g_borda = 255.0f, b_borda = 255.0f;
    float r_centro = 255.0f, g_centro = 255.0f, b_centro = 255.0f;

    if (temp < 400.0f) {
        // < 400 °C: Branco (255, 255, 255)
        r_borda = 255.0f; g_borda = 255.0f; b_borda = 255.0f;
        r_centro = 255.0f; g_centro = 255.0f; b_centro = 255.0f;
    } else if (temp < 700.0f) {
        // 400 °C a 700 °C: Interpola de Branco (255, 255, 255) para Vermelho Vivo (255, 20, 20)
        float ratio = (temp - 400.0f) / (700.0f - 400.0f);
        r_borda = lerp_float(255.0f, 255.0f, ratio);
        g_borda = lerp_float(255.0f, 20.0f, ratio);
        b_borda = lerp_float(255.0f, 20.0f, ratio);
        r_centro = r_borda; g_centro = g_borda; b_centro = b_borda;
    } else if (temp < 900.0f) {
        // 700 °C a 900 °C: Interpola de Vermelho Vivo (255, 20, 20) para Laranja (255, 100, 0)
        float ratio = (temp - 700.0f) / (900.0f - 700.0f);
        r_borda = lerp_float(255.0f, 255.0f, ratio);
        g_borda = lerp_float(20.0f, 100.0f, ratio);
        b_borda = lerp_float(20.0f, 0.0f, ratio);
        r_centro = r_borda; g_centro = g_borda; b_centro = b_borda;
    } else if (temp <= 1100.0f) {
        // 900 °C a 1100 °C: Interpola de Laranja (255, 100, 0) para Amarelo Ouro (255, 210, 0)
        float ratio = (temp - 900.0f) / (1100.0f - 900.0f);
        r_borda = lerp_float(255.0f, 255.0f, ratio);
        g_borda = lerp_float(100.0f, 210.0f, ratio);
        b_borda = lerp_float(0.0f, 0.0f, ratio);
        r_centro = r_borda; g_centro = g_borda; b_centro = b_borda;
    } else {
        // > 1100 °C: cor_borda = Amarelo Ouro (255, 210, 0), cor_centro = Branco (255, 255, 255)
        r_borda = 255.0f; g_borda = 210.0f; b_borda = 0.0f;
        r_centro = 255.0f; g_centro = 255.0f; b_centro = 255.0f;
    }

    if (cor_borda) {
        *cor_borda = al_map_rgb(clamp_canal_rgb(r_borda), clamp_canal_rgb(g_borda), clamp_canal_rgb(b_borda));
    }
    if (cor_centro) {
        *cor_centro = al_map_rgb(clamp_canal_rgb(r_centro), clamp_canal_rgb(g_centro), clamp_canal_rgb(b_centro));
    }
}

ALLEGRO_COLOR obter_cor_temperatura(float temp) {
    ALLEGRO_COLOR cor_borda;
    obter_cores_temperatura(temp, &cor_borda, NULL);
    return cor_borda;
}

void desenhar_sprite_quente(ALLEGRO_BITMAP *sprite, float centro_x, float centro_y, float larg_fallback, float alt_fallback, int flags, float temp, int frame_atual, ALLEGRO_COLOR cor_elemento) {
    ALLEGRO_COLOR cor_borda, cor_centro;
    obter_cores_temperatura(temp, &cor_borda, &cor_centro);

    unsigned char er, eg, eb;
    al_unmap_rgb(cor_elemento, &er, &eg, &eb);

    unsigned char br, bg, bb;
    al_unmap_rgb(cor_borda, &br, &bg, &bb);

    // Combina o tint RGB do elemento com a incandescência térmica acima de 400 °C
    ALLEGRO_COLOR tint_base;
    if (temp < 400.0f) {
        tint_base = al_map_rgb(er, eg, eb);
    } else if (temp < 700.0f) {
        float ratio_inc = (temp - 400.0f) / 300.0f;
        unsigned char r_mix = clamp_canal_rgb(lerp_float((float)er, (float)br, ratio_inc));
        unsigned char g_mix = clamp_canal_rgb(lerp_float((float)eg, (float)bg, ratio_inc));
        unsigned char b_mix = clamp_canal_rgb(lerp_float((float)eb, (float)bb, ratio_inc));
        tint_base = al_map_rgb(r_mix, g_mix, b_mix);
    } else {
        tint_base = cor_borda;
    }

    float alpha_aura = 0.0f;
    float alpha_centro = 0.0f;
    if (temp < 400.0f) {
        alpha_aura = 0.0f;
        alpha_centro = 0.0f;
    } else if (temp <= 525.0f) {
        float razao = (temp - 400.0f) / 125.0f;
        alpha_aura = lerp_float(0.0f, 0.15f, razao);
        alpha_centro = lerp_float(0.0f, 0.4f, razao);
    } else {
        alpha_aura = 0.15f;
        alpha_centro = 0.4f;
    }

    float r_borda_f = (float)br / 255.0f;
    float g_borda_f = (float)bg / 255.0f;
    float b_borda_f = (float)bb / 255.0f;

    unsigned char cr, cg, cb;
    al_unmap_rgb(cor_centro, &cr, &cg, &cb);
    float r_centro_f = (float)cr / 255.0f;
    float g_centro_f = (float)cg / 255.0f;
    float b_centro_f = (float)cb / 255.0f;

    ALLEGRO_COLOR tint_aura = al_map_rgba_f(r_borda_f, g_borda_f, b_borda_f, alpha_aura);
    ALLEGRO_COLOR tint_nucleo = al_map_rgba_f(r_centro_f, g_centro_f, b_centro_f, alpha_centro);

    const float offsets_x[4] = { -3.0f,  3.0f,  0.0f,  0.0f };
    const float offsets_y[4] = {  0.0f,  0.0f, -3.0f,  3.0f };

    if (sprite != NULL) {
        float larg_img = al_get_bitmap_width(sprite);
        float alt_img = al_get_bitmap_height(sprite);
        float base_x = centro_x - (larg_img / 2.0f);
        float base_y = centro_y - (alt_img / 2.0f);

        // Passo 1 (Base): Desenha normalmente com tint_base (Cor do Elemento + Incandescência)
        al_draw_tinted_bitmap(sprite, tint_base, base_x, base_y, flags);

        if (temp >= 400.0f) {
            int op_ant, src_ant, dst_ant;
            al_get_blender(&op_ant, &src_ant, &dst_ant);
            al_set_blender(ALLEGRO_ADD, ALLEGRO_ALPHA, ALLEGRO_ONE);

            // Passo 2 (Aura/Offsets): 4 offsets no modo ALLEGRO_ADD usando cor_borda com opacidade alpha_aura
            for (int i = 0; i < 4; i++) {
                al_draw_tinted_bitmap(sprite, tint_aura, base_x + offsets_x[i], base_y + offsets_y[i], flags);
            }

            // Passo 3 (Núcleo): Sprite centralizado 1 vez no modo ALLEGRO_ADD usando cor_centro com opacidade alpha_centro
            al_draw_tinted_bitmap(sprite, tint_nucleo, base_x, base_y, flags);

            al_set_blender(op_ant, src_ant, dst_ant);
        }
    } else {
        ALLEGRO_COLOR cor_base = tint_base;
        if (temp < 400.0f) {
            float fator_estagio = 1.0f - ((float)frame_atual * 0.08f);
            cor_base = al_map_rgb(clamp_canal_rgb((float)er * fator_estagio),
                                  clamp_canal_rgb((float)eg * fator_estagio),
                                  clamp_canal_rgb((float)eb * fator_estagio));
        }

        float x1 = centro_x - larg_fallback / 2.0f;
        float y1 = centro_y - alt_fallback / 2.0f;
        float x2 = centro_x + larg_fallback / 2.0f;
        float y2 = centro_y + alt_fallback / 2.0f;

        // Passo 1 (Base)
        al_draw_filled_rectangle(x1, y1, x2, y2, cor_base);

        if (temp >= 400.0f) {
            int op_ant, src_ant, dst_ant;
            al_get_blender(&op_ant, &src_ant, &dst_ant);
            al_set_blender(ALLEGRO_ADD, ALLEGRO_ALPHA, ALLEGRO_ONE);

            // Passo 2 (Aura/Offsets)
            for (int i = 0; i < 4; i++) {
                al_draw_filled_rectangle(x1 + offsets_x[i], y1 + offsets_y[i],
                                         x2 + offsets_x[i], y2 + offsets_y[i], tint_aura);
            }

            // Passo 3 (Núcleo)
            al_draw_filled_rectangle(x1, y1, x2, y2, tint_nucleo);

            al_set_blender(op_ant, src_ant, dst_ant);
        }
    }
}

void desenhar_olhos_inimigo(const Inimigo *anomalia, ALLEGRO_COLOR cor_elemento) {
    if (!anomalia || !anomalia->ativo) return;

    int estagio = anomalia->frame_atual;
    if (estagio < 0) estagio = 0;
    if (estagio > 4) estagio = 4;

    // Os olhos acompanham o derretimento nos 5 estágios (40x30 px) e a respiração (_bre) quando vivo
    const float offsets_olho_y[5] = { 11.0f, 13.0f, 15.0f, 18.0f, 22.0f };
    float base_x = anomalia->x - 20.0f;
    float base_y = anomalia->y - 15.0f;

    float olho_y = base_y + offsets_olho_y[estagio];
    if (!anomalia->morto && anomalia->frame_animacao == 1) {
        olho_y += 1.0f;
    }

    float olho1_x, olho2_x;
    if (anomalia->direcao_atual == DIR_ESQUERDA) {
        olho1_x = base_x + 12.0f;
        olho2_x = base_x + 20.0f;
    } else {
        olho1_x = base_x + 17.0f;
        olho2_x = base_x + 25.0f;
    }

    ALLEGRO_BITMAP *spr_olho = anomalia->morto ? anomalia->sprite_olho_m : anomalia->sprite_olho;
    if (spr_olho != NULL) {
        al_draw_tinted_bitmap(spr_olho, cor_elemento, olho1_x, olho_y, 0);
        al_draw_tinted_bitmap(spr_olho, cor_elemento, olho2_x, olho_y, 0);
    } else {
        // Fallback geométrico em pixel-art para o par de olhos (3x3 px)
        float xs[2] = { olho1_x, olho2_x };
        for (int i = 0; i < 2; i++) {
            float ox = xs[i];
            if (anomalia->morto) {
                al_draw_filled_rectangle(ox,        olho_y,        ox + 1.0f, olho_y + 1.0f, cor_elemento);
                al_draw_filled_rectangle(ox + 2.0f, olho_y,        ox + 3.0f, olho_y + 1.0f, cor_elemento);
                al_draw_filled_rectangle(ox + 1.0f, olho_y + 1.0f, ox + 2.0f, olho_y + 2.0f, cor_elemento);
                al_draw_filled_rectangle(ox,        olho_y + 2.0f, ox + 1.0f, olho_y + 3.0f, cor_elemento);
                al_draw_filled_rectangle(ox + 2.0f, olho_y + 2.0f, ox + 3.0f, olho_y + 3.0f, cor_elemento);
            } else {
                al_draw_filled_rectangle(ox, olho_y, ox + 3.0f, olho_y + 3.0f, cor_elemento);
                al_draw_filled_rectangle(ox + 1.0f, olho_y + 1.0f, ox + 2.0f, olho_y + 2.0f, al_map_rgb(20, 20, 20));
            }
        }
    }
}

void desenhar_marcas_fuligem(const MarcaFuligem *marcas, int max_marcas, int sala_atual) {
    if (!marcas) return;

    ALLEGRO_COLOR cor_fuligem_escura = al_map_rgba_f(0.04f, 0.04f, 0.05f, 0.38f);
    ALLEGRO_COLOR cor_fuligem_media  = al_map_rgba_f(0.09f, 0.09f, 0.11f, 0.35f);
    ALLEGRO_COLOR cor_fuligem_cinza  = al_map_rgba_f(0.16f, 0.16f, 0.18f, 0.28f);

    for (int i = 0; i < max_marcas; i++) {
        if (!marcas[i].ativo || marcas[i].sala != sala_atual) continue;

        float fx = marcas[i].x;
        float fy = marcas[i].y;

        // Manchas pixeladas na parede de fundo e chão ao redor da explosão
        al_draw_filled_rectangle(fx - 26.0f, fy - 18.0f, fx + 26.0f, fy + 2.0f,  cor_fuligem_cinza);
        al_draw_filled_rectangle(fx - 20.0f, fy - 24.0f, fx + 20.0f, fy - 2.0f,  cor_fuligem_media);
        al_draw_filled_rectangle(fx - 14.0f, fy - 28.0f, fx + 14.0f, fy - 6.0f,  cor_fuligem_media);
        al_draw_filled_rectangle(fx - 16.0f, fy - 14.0f, fx + 16.0f, fy + 1.0f,  cor_fuligem_escura);
        al_draw_filled_rectangle(fx - 10.0f, fy - 20.0f, fx + 10.0f, fy,         cor_fuligem_escura);
        al_draw_filled_rectangle(fx - 30.0f, fy - 4.0f,  fx + 30.0f, fy + 3.0f,  cor_fuligem_escura);
        al_draw_filled_rectangle(fx - 24.0f, fy - 10.0f, fx - 16.0f, fy - 4.0f,  cor_fuligem_cinza);
        al_draw_filled_rectangle(fx + 15.0f, fy - 12.0f, fx + 25.0f, fy - 5.0f,  cor_fuligem_cinza);
        al_draw_filled_rectangle(fx - 6.0f,  fy - 32.0f, fx + 6.0f,  fy - 24.0f, cor_fuligem_cinza);
    }
}

void desenhar_particulas(Particula *particulas, int max_particulas) {
    if (!particulas) return;

    for (int i = 0; i < max_particulas; i++) {
        if (particulas[i].vida <= 0.0f || particulas[i].vida_maxima <= 0.0f) continue;

        float ratio_vida = particulas[i].vida / particulas[i].vida_maxima;
        if (ratio_vida < 0.0f) ratio_vida = 0.0f;
        if (ratio_vida > 1.0f) ratio_vida = 1.0f;

        unsigned char r, g, b;
        al_unmap_rgb(particulas[i].cor, &r, &g, &b);

        if (particulas[i].tipo == PARTICULA_GOTA) {
            unsigned char alpha_gota = clamp_canal_rgb(ratio_vida * 230.0f);
            ALLEGRO_COLOR cor_gota = al_map_rgba(r, g, b, alpha_gota);
            float metade = particulas[i].tamanho * 0.5f;
            if (metade < 0.5f) metade = 0.5f;
            al_draw_filled_rectangle(particulas[i].x - metade, particulas[i].y - metade,
                                     particulas[i].x + metade, particulas[i].y + metade, cor_gota);
        } else if (particulas[i].tipo == PARTICULA_VAPOR) {
            unsigned char alpha_vapor = clamp_canal_rgb(ratio_vida * 150.0f);
            ALLEGRO_COLOR cor_vapor = al_map_rgba(r, g, b, alpha_vapor);
            float metade = particulas[i].tamanho * 0.5f;
            if (metade < 0.6f) metade = 0.6f;
            al_draw_filled_rectangle(particulas[i].x - metade, particulas[i].y - metade,
                                     particulas[i].x + metade, particulas[i].y + metade, cor_vapor);
        } else if (particulas[i].tipo == PARTICULA_CALOR) {
            unsigned char alpha_calor = clamp_canal_rgb(ratio_vida * 128.0f);
            ALLEGRO_COLOR cor_calor = al_map_rgba(r, g, b, alpha_calor);
            float largura = 1.0f;
            float altura = 4.0f + (4.0f * (1.0f - ratio_vida));
            float metade_h = altura * 0.5f;
            al_draw_filled_rectangle(particulas[i].x, particulas[i].y - metade_h,
                                     particulas[i].x + largura, particulas[i].y + metade_h, cor_calor);
        } else if (particulas[i].tipo == PARTICULA_EXPLOSAO) {
            unsigned char alpha_exp = clamp_canal_rgb(ratio_vida * 245.0f);
            ALLEGRO_COLOR cor_exp = al_map_rgba(r, g, b, alpha_exp);
            float pw = (particulas[i].largura > 0.0f) ? particulas[i].largura : particulas[i].tamanho;
            float ph = (particulas[i].altura > 0.0f) ? particulas[i].altura : particulas[i].tamanho;
            al_draw_filled_rectangle(particulas[i].x - pw * 0.5f, particulas[i].y - ph * 0.5f,
                                     particulas[i].x + pw * 0.5f, particulas[i].y + ph * 0.5f, cor_exp);
        }
    }
}

void desenhar_segmento_bateria(float x, float y, ALLEGRO_COLOR cor) {
    // Escadinha diagonal de 8 pixels (1 + 2 + 2 + 2 + 1 pixels) para as 7 divisões inferiores
    al_draw_filled_rectangle(x + 3.0f, y,        x + 4.0f, y + 1.0f, cor);
    al_draw_filled_rectangle(x + 2.0f, y + 1.0f, x + 4.0f, y + 2.0f, cor);
    al_draw_filled_rectangle(x + 1.0f, y + 2.0f, x + 3.0f, y + 3.0f, cor);
    al_draw_filled_rectangle(x,        y + 3.0f, x + 2.0f, y + 4.0f, cor);
    al_draw_filled_rectangle(x,        y + 4.0f, x + 1.0f, y + 5.0f, cor);
}

void desenhar_segmento_bateria_topo(float x, float y, ALLEGRO_COLOR cor) {
    // 1ª divisão (a mais do topo): 3 linhas alinhadas à esquerda (4px, 2px, 1px)
    al_draw_filled_rectangle(x, y,        x + 4.0f, y + 1.0f, cor);
    al_draw_filled_rectangle(x, y + 1.0f, x + 2.0f, y + 2.0f, cor);
    al_draw_filled_rectangle(x, y + 2.0f, x + 1.0f, y + 3.0f, cor);
}

void desenhar_icone_refil_agua(float x, float y, ALLEGRO_COLOR cor) {
    // Ícone de Refil de Água (Azul): 3 linhas de altura (1px centralizado, 3px, 3px)
    al_draw_filled_rectangle(x + 1.0f, y,        x + 2.0f, y + 1.0f, cor);
    al_draw_filled_rectangle(x,        y + 1.0f, x + 3.0f, y + 2.0f, cor);
    al_draw_filled_rectangle(x,        y + 2.0f, x + 3.0f, y + 3.0f, cor);
}

void desenhar_icone_bateria_guardada(float x, float y, ALLEGRO_COLOR cor) {
    // Ícone de Bateria Guardada (Amarelo): 1 linha com 1px de altura e 4px de largura
    al_draw_filled_rectangle(x, y, x + 4.0f, y + 1.0f, cor);
}

void desenhar_hud_jogador(Jogador *faxineiro, GameSprites *sprites) {
    // =========================================================================
    // Transformação local de Escala Global do HUD (2x / 200%)
    // =========================================================================
    ALLEGRO_TRANSFORM trans_original;
    al_copy_transform(&trans_original, al_get_current_transform());

    ALLEGRO_TRANSFORM hud_transform;
    al_identity_transform(&hud_transform);
    al_scale_transform(&hud_transform, 2.0f, 2.0f);
    al_compose_transform(&hud_transform, &trans_original);
    al_use_transform(&hud_transform);

    // =========================================================================
    // Variáveis de posição e offset do HUD (facilmente ajustáveis)
    // =========================================================================
    float base_x = 3.0f;
    float base_y = 3.0f;

    // Offsets da Barra de Vida/Radiação (barra_vida.png: 64x8) e Caveira (9x8)
    float offset_vida_x = 0.0f;
    float offset_vida_y = 0.0f;
    float offset_preenchimento_vida_x = 7.0f;
    float offset_preenchimento_vida_y = 2.0f;
    float largura_max_vida = 56.0f;
    float altura_preenchimento_vida = 4.0f;
    float offset_caveira_x = 63.0f;
    float offset_caveira_y = 0.0f;

    // Offsets da Barra Vertical de Água (barra_agua.png: 8x62) e Refis Guardados (-1 no X, -3 no Y)
    float offset_agua_x = -1.0f;
    float offset_agua_y = 7.0f;
    float offset_preenchimento_agua_x = 3.0f;
    float offset_preenchimento_agua_y = 3.0f;
    float largura_preenchimento_agua = 4.0f;
    float altura_max_agua = 58.0f;
    float offset_icone_agua_x = 1.0f;
    float offset_icone_agua_y = 1.0f;
    float espacamento_icone_agua_y = 6.0f;

    // Offsets da Barra Vertical de Bateria (barra_bateria.png: 7x59), 8 Segmentos e Baterias Guardadas (-2 no X, -5 no Y)
    float offset_bateria_x = 8.0f;
    float offset_bateria_y = 8.0f;
    float offset_segmento_bateria_x = 1.0f;
    float offset_segmento_bateria_y_base = 53.0f;
    float espacamento_segmento_bateria_y = 6.0f;
    float offset_topo_bateria_x = 1.0f;
    float offset_topo_bateria_y = 13.0f;
    float offset_icone_bateria_x = 1.0f;
    float offset_icone_bateria_y = 1.0f;
    float espacamento_icone_bateria_y = 4.0f;

    // Coordenadas base calculadas de cada componente
    float vida_x = base_x + offset_vida_x;
    float vida_y = base_y + offset_vida_y;
    float agua_x = base_x + offset_agua_x;
    float agua_y = base_y + offset_agua_y;
    float bateria_x = base_x + offset_bateria_x;
    float bateria_y = base_y + offset_bateria_y;

    // Cores dos preenchimentos
    ALLEGRO_COLOR cor_vida     = al_map_rgb(173, 40, 40);
    ALLEGRO_COLOR cor_radiacao = al_map_rgb(39, 172, 39);
    ALLEGRO_COLOR cor_agua     = al_map_rgb(60, 160, 255);
    ALLEGRO_COLOR cor_bateria  = al_map_rgb(255, 200, 20);

    // =========================================================================
    // PASSO 1 (Background): Preenchimentos Dinâmicos e Ícones por Código
    // =========================================================================

    // 1.1. Preenchimento de Vida (Vermelho: esquerda -> direita) e Radiação (Verde: direita -> esquerda)
    float fill_vida_x = vida_x + offset_preenchimento_vida_x;
    float fill_vida_y = vida_y + offset_preenchimento_vida_y;

    float pct_vida = faxineiro->barra_vida / 100.0f;
    if (pct_vida < 0.0f) pct_vida = 0.0f;
    if (pct_vida > 1.0f) pct_vida = 1.0f;
    float larg_vida = largura_max_vida * pct_vida;
    if (larg_vida > 0.0f) {
        al_draw_filled_rectangle(fill_vida_x, fill_vida_y,
                                 fill_vida_x + larg_vida, fill_vida_y + altura_preenchimento_vida, cor_vida);
    }

    if (faxineiro->barra_radio > 0.0f) {
        float pct_radio = faxineiro->barra_radio / 100.0f;
        if (pct_radio < 0.0f) pct_radio = 0.0f;
        if (pct_radio > 1.0f) pct_radio = 1.0f;
        float larg_radio = largura_max_vida * pct_radio;
        float radio_x2 = fill_vida_x + largura_max_vida;
        float radio_x1 = radio_x2 - larg_radio;
        al_draw_filled_rectangle(radio_x1, fill_vida_y,
                                 radio_x2, fill_vida_y + altura_preenchimento_vida, cor_radiacao);
    }

    // 1.2. Preenchimento da Barra de Água (apenas se possuir o Mop)
    if (faxineiro->possui_mop) {
        float pct_agua = faxineiro->barra_agua / 100.0f;
        if (pct_agua < 0.0f) pct_agua = 0.0f;
        if (pct_agua > 1.0f) pct_agua = 1.0f;

        int total_linhas_agua = (int)altura_max_agua;
        int linhas_preenchidas = (int)ceilf(altura_max_agua * pct_agua);
        int linha_inicial = (total_linhas_agua - linhas_preenchidas) + 1;
        if (linha_inicial < 1) linha_inicial = 1;

        float fill_agua_x = agua_x + offset_preenchimento_agua_x;
        float fill_agua_y = agua_y + offset_preenchimento_agua_y;

        // Linha 1 = topo absoluto do preenchimento; omite o pixel da 1ª coluna (X=0) nas linhas 1, 6, 7, 12 e 13
        for (int linha = linha_inicial; linha <= total_linhas_agua; linha++) {
            bool pular_primeira_coluna = (linha == 1 || linha == 6 || linha == 7 || linha == 12 || linha == 13);
            float x_inicio = pular_primeira_coluna ? (fill_agua_x + 1.0f) : fill_agua_x;
            float y_linha = fill_agua_y + (float)(linha - 1);
            al_draw_filled_rectangle(x_inicio, y_linha,
                                     fill_agua_x + largura_preenchimento_agua, y_linha + 1.0f, cor_agua);
        }
    }

    // 1.3. Preenchimento dos 8 Blocos de Bateria (apenas se possuir o Soprador)
    if (faxineiro->possui_soprador) {
        int segmentos_ativos = 0;
        if (faxineiro->barra_bateria > 0.0f) {
            segmentos_ativos = (int)ceilf(faxineiro->barra_bateria / 12.5f);
            if (segmentos_ativos < 1) segmentos_ativos = 1;
            if (segmentos_ativos > 8) segmentos_ativos = 8;
        }

        for (int i = 0; i < segmentos_ativos; i++) {
            if (i < 7) {
                float seg_x = bateria_x + offset_segmento_bateria_x;
                float seg_y = bateria_y + offset_segmento_bateria_y_base - ((float)i * espacamento_segmento_bateria_y);
                desenhar_segmento_bateria(seg_x, seg_y, cor_bateria);
            } else {
                float topo_x = bateria_x + offset_topo_bateria_x;
                float topo_y = bateria_y + offset_topo_bateria_y;
                desenhar_segmento_bateria_topo(topo_x, topo_y, cor_bateria);
            }
        }
    }

    // =========================================================================
    // PASSO 2 (Overlay / Por cima): Sprites de Moldura do HUD
    // =========================================================================
    if (sprites->sprite_hud_vida != NULL) {
        al_draw_bitmap(sprites->sprite_hud_vida, vida_x, vida_y, 0);
    } else {
        al_draw_rectangle(vida_x, vida_y, vida_x + 64.0f, vida_y + 8.0f, al_map_rgb(180, 180, 180), 1.0f);
    }

    if (sprites->sprite_hud_agua != NULL) {
        al_draw_bitmap(sprites->sprite_hud_agua, agua_x, agua_y, 0);
    } else {
        al_draw_rectangle(agua_x, agua_y, agua_x + 8.0f, agua_y + 62.0f, al_map_rgb(180, 180, 180), 1.0f);
    }

    if (sprites->sprite_hud_bateria != NULL) {
        al_draw_bitmap(sprites->sprite_hud_bateria, bateria_x, bateria_y, 0);
    } else {
        al_draw_rectangle(bateria_x, bateria_y, bateria_x + 7.0f, bateria_y + 59.0f, al_map_rgb(180, 180, 180), 1.0f);
    }

    if (faxineiro->barra_radio > 0.0f) {
        float caveira_x = vida_x + offset_caveira_x;
        float caveira_y = vida_y + offset_caveira_y;
        if (sprites->sprite_hud_caveira != NULL) {
            al_draw_bitmap(sprites->sprite_hud_caveira, caveira_x, caveira_y, 0);
        } else {
            al_draw_filled_rectangle(caveira_x, caveira_y, caveira_x + 9.0f, caveira_y + 8.0f, cor_radiacao);
        }
    }

    // =========================================================================
    // PASSO 3 (Foreground / Ícones de Inventário): Desenhados após as molduras
    // para nunca serem sobrepostos pela parte opaca da sprite da moldura
    // =========================================================================
    for (int i = 0; i < faxineiro->refil_agua_qtd && i < 3; i++) {
        float icone_x = agua_x + offset_icone_agua_x;
        float icone_y = agua_y + offset_icone_agua_y + ((float)i * espacamento_icone_agua_y);
        desenhar_icone_refil_agua(icone_x, icone_y, cor_agua);
    }

    for (int i = 0; i < faxineiro->bateria_qtd && i < 3; i++) {
        float icone_x = bateria_x + offset_icone_bateria_x;
        float icone_y = bateria_y + offset_icone_bateria_y + ((float)i * espacamento_icone_bateria_y);
        desenhar_icone_bateria_guardada(icone_x, icone_y, cor_bateria);
    }

    // Restaura a transformação original da câmera
    al_use_transform(&trans_original);
}

bool scanner_posicao_dentro_painel(float mouse_x, float mouse_y) {
    return (mouse_x >= SCANNER_PAINEL_X && mouse_x <= (SCANNER_PAINEL_X + SCANNER_PAINEL_W) &&
            mouse_y >= SCANNER_PAINEL_Y && mouse_y <= (SCANNER_PAINEL_Y + SCANNER_PAINEL_H));
}

bool scanner_clique_botao_voltar(float mouse_x, float mouse_y) {
    return (mouse_x >= SCANNER_BTN_VOLTAR_X && mouse_x <= (SCANNER_BTN_VOLTAR_X + SCANNER_BTN_VOLTAR_W) &&
            mouse_y >= SCANNER_BTN_VOLTAR_Y && mouse_y <= (SCANNER_BTN_VOLTAR_Y + SCANNER_BTN_VOLTAR_H));
}

bool scanner_clique_botao_fechar(float mouse_x, float mouse_y) {
    return (mouse_x >= SCANNER_BTN_FECHAR_X && mouse_x <= (SCANNER_BTN_FECHAR_X + SCANNER_BTN_FECHAR_W) &&
            mouse_y >= SCANNER_BTN_FECHAR_Y && mouse_y <= (SCANNER_BTN_FECHAR_Y + SCANNER_BTN_FECHAR_H));
}

int scanner_obter_item_clicado_menu(float mouse_x, float mouse_y, const bool *elemento_descoberto) {
    if (!elemento_descoberto) {
        return 0;
    }
    if (mouse_x < SCANNER_MENU_ITEM_X || mouse_x > (SCANNER_MENU_ITEM_X + SCANNER_MENU_ITEM_W)) {
        return 0;
    }

    int indice_visivel = 0;
    for (int i = 1; i <= MAX_ELEMENTOS; i++) {
        if (elemento_descoberto[i] == true) {
            float item_y = SCANNER_MENU_LIST_Y + ((float)indice_visivel * (SCANNER_MENU_ITEM_H + SCANNER_MENU_ITEM_GAP));
            if (mouse_y >= item_y && mouse_y <= (item_y + SCANNER_MENU_ITEM_H)) {
                return i;
            }
            indice_visivel++;
        }
    }
    return 0;
}

bool scanner_processar_clique_mouse(int *modo_scanner, bool *scanner_ativo, int *elemento_foco_id,
                                    const bool *elemento_descoberto, float mouse_x, float mouse_y) {
    if (!modo_scanner || *modo_scanner == SCANNER_FECHADO) {
        return false;
    }

    // 1. Botão Fechar [X]
    if (scanner_clique_botao_fechar(mouse_x, mouse_y)) {
        *modo_scanner = SCANNER_FECHADO;
        if (scanner_ativo) *scanner_ativo = false;
        return true;
    }

    // 2. Botão Voltar [<- Lista] nos estados de detalhe ou escaneamento
    if (*modo_scanner == SCANNER_ESCANEANDO || *modo_scanner == SCANNER_DETALHES) {
        if (scanner_clique_botao_voltar(mouse_x, mouse_y)) {
            *modo_scanner = SCANNER_MENU;
            return true;
        }
    }

    // 3. Clique em item da lista no estado MENU
    if (*modo_scanner == SCANNER_MENU) {
        int id_clicado = scanner_obter_item_clicado_menu(mouse_x, mouse_y, elemento_descoberto);
        if (id_clicado > 0) {
            if (elemento_foco_id) *elemento_foco_id = id_clicado;
            *modo_scanner = SCANNER_DETALHES;
            return true;
        }
    }

    // 4. Qualquer outro clique dentro do painel consome o clique
    if (scanner_posicao_dentro_painel(mouse_x, mouse_y)) {
        return true;
    }

    return false;
}

void desenhar_visor_scanner(int modo_scanner, float tempo_scan, int elemento_foco_id,
                            const bool *elemento_descoberto, const Inimigo *anomalia,
                            ALLEGRO_FONT *fonte) {
    if (modo_scanner == SCANNER_FECHADO) return;

    ALLEGRO_TRANSFORM trans_ui = *al_get_current_transform();
    ALLEGRO_TRANSFORM trans_texto;
    al_identity_transform(&trans_texto);
    al_scale_transform(&trans_texto, 0.5f, 0.5f);
    al_compose_transform(&trans_texto, &trans_ui);

    // Fundo do painel: retângulo escuro arredondado com 50% de opacidade (reduzido em 50%)
    al_draw_filled_rounded_rectangle(
        SCANNER_PAINEL_X, SCANNER_PAINEL_Y,
        SCANNER_PAINEL_X + SCANNER_PAINEL_W, SCANNER_PAINEL_Y + SCANNER_PAINEL_H,
        3.0f, 3.0f, al_map_rgba_f(0.0f, 0.0f, 0.0f, 0.5f)
    );

    // Borda tecnológica verde com brilho sutil
    al_draw_rounded_rectangle(
        SCANNER_PAINEL_X, SCANNER_PAINEL_Y,
        SCANNER_PAINEL_X + SCANNER_PAINEL_W, SCANNER_PAINEL_Y + SCANNER_PAINEL_H,
        3.0f, 3.0f, al_map_rgba_f(0.2f, 0.7f, 0.4f, 0.7f), 1.0f
    );

    ALLEGRO_COLOR cor_texto = al_map_rgba_f(1.0f, 1.0f, 1.0f, 0.75f);
    ALLEGRO_COLOR cor_destaque = al_map_rgba_f(0.4f, 1.0f, 0.6f, 0.9f);
    ALLEGRO_COLOR cor_btn_bg = al_map_rgba_f(0.12f, 0.2f, 0.15f, 0.8f);
    ALLEGRO_COLOR cor_btn_borda = al_map_rgba_f(0.3f, 0.8f, 0.5f, 0.85f);

    // Botão Fechar [X] no topo direito
    al_draw_filled_rounded_rectangle(
        SCANNER_BTN_FECHAR_X, SCANNER_BTN_FECHAR_Y,
        SCANNER_BTN_FECHAR_X + SCANNER_BTN_FECHAR_W, SCANNER_BTN_FECHAR_Y + SCANNER_BTN_FECHAR_H,
        1.5f, 1.5f, cor_btn_bg
    );
    al_draw_rounded_rectangle(
        SCANNER_BTN_FECHAR_X, SCANNER_BTN_FECHAR_Y,
        SCANNER_BTN_FECHAR_X + SCANNER_BTN_FECHAR_W, SCANNER_BTN_FECHAR_Y + SCANNER_BTN_FECHAR_H,
        1.5f, 1.5f, cor_btn_borda, 1.0f
    );
    al_use_transform(&trans_texto);
    al_draw_text(fonte, cor_texto, (SCANNER_BTN_FECHAR_X + (SCANNER_BTN_FECHAR_W / 2.0f)) * 2.0f, (SCANNER_BTN_FECHAR_Y + 2.0f) * 2.0f, ALLEGRO_ALIGN_CENTRE, "X");
    al_use_transform(&trans_ui);

    if (modo_scanner == SCANNER_ESCANEANDO || modo_scanner == SCANNER_DETALHES) {
        // Botão [<- Lista] no topo esquerdo
        al_draw_filled_rounded_rectangle(
            SCANNER_BTN_VOLTAR_X, SCANNER_BTN_VOLTAR_Y,
            SCANNER_BTN_VOLTAR_X + SCANNER_BTN_VOLTAR_W, SCANNER_BTN_VOLTAR_Y + SCANNER_BTN_VOLTAR_H,
            1.5f, 1.5f, cor_btn_bg
        );
        al_draw_rounded_rectangle(
            SCANNER_BTN_VOLTAR_X, SCANNER_BTN_VOLTAR_Y,
            SCANNER_BTN_VOLTAR_X + SCANNER_BTN_VOLTAR_W, SCANNER_BTN_VOLTAR_Y + SCANNER_BTN_VOLTAR_H,
            1.5f, 1.5f, cor_btn_borda, 1.0f
        );

        const char *titulo = (modo_scanner == SCANNER_ESCANEANDO) ? "ESCANEANDO..." : "DETALHES";
        al_use_transform(&trans_texto);
        al_draw_text(fonte, cor_texto, (SCANNER_BTN_VOLTAR_X + (SCANNER_BTN_VOLTAR_W / 2.0f)) * 2.0f, (SCANNER_BTN_VOLTAR_Y + 2.0f) * 2.0f, ALLEGRO_ALIGN_CENTRE, "<- Lista");
        al_draw_text(fonte, cor_destaque, (SCANNER_PAINEL_X + 77.5f) * 2.0f, (SCANNER_PAINEL_Y + 5.0f) * 2.0f, ALLEGRO_ALIGN_CENTRE, titulo);
        al_use_transform(&trans_ui);

        // Linha divisória do cabeçalho
        al_draw_line(
            SCANNER_PAINEL_X + 4.0f, SCANNER_PAINEL_Y + 13.0f,
            SCANNER_PAINEL_X + SCANNER_PAINEL_W - 4.0f, SCANNER_PAINEL_Y + 13.0f,
            al_map_rgba_f(0.3f, 0.8f, 0.5f, 0.35f), 1.0f
        );

        // Caixa de exibição do sprite do inimigo (50% menor)
        float box_x = SCANNER_PAINEL_X + 5.0f;
        float box_y = SCANNER_PAINEL_Y + 15.0f;
        float box_w = 27.0f;
        float box_h = 21.0f;

        al_draw_filled_rectangle(box_x, box_y, box_x + box_w, box_y + box_h, al_map_rgba_f(0.05f, 0.1f, 0.08f, 0.7f));
        al_draw_rectangle(box_x, box_y, box_x + box_w, box_y + box_h, al_map_rgba_f(0.3f, 0.7f, 0.5f, 0.6f), 1.0f);

        if (anomalia && elemento_foco_id > 0 && elemento_foco_id <= MAX_ELEMENTOS && anomalia->sprites_normal[0] != NULL) {
            DadosElemento elem_prev = tabela_periodica[elemento_foco_id];
            int estagio_prev = (elem_prev.ponto_fusao <= 20.0f) ? 4 : 0;
            ALLEGRO_BITMAP *spr = anomalia->sprites_normal[estagio_prev] ? anomalia->sprites_normal[estagio_prev] : anomalia->sprites_normal[0];
            const float offsets_olho_y[5] = { 11.0f, 13.0f, 15.0f, 18.0f, 22.0f };
            float sw = (float)al_get_bitmap_width(spr);
            float sh = (float)al_get_bitmap_height(spr);
            float dw = sw * 0.5f;
            float dh = sh * 0.5f;
            float spr_x = box_x + (box_w - dw) / 2.0f;
            float spr_y = box_y + (box_h - dh) / 2.0f;

            if (modo_scanner == SCANNER_ESCANEANDO) {
                // Silhueta totalmente preta durante o escaneamento
                al_draw_tinted_scaled_bitmap(spr, al_map_rgba_f(0.0f, 0.0f, 0.0f, 1.0f), 0.0f, 0.0f, sw, sh, spr_x, spr_y, dw, dh, 0);
            } else {
                // Sprite com o tint RGB real do elemento após o escaneamento
                ALLEGRO_COLOR cor_prev = al_map_rgb(elem_prev.cor_r, elem_prev.cor_g, elem_prev.cor_b);
                al_draw_tinted_scaled_bitmap(spr, cor_prev, 0.0f, 0.0f, sw, sh, spr_x, spr_y, dw, dh, 0);
                if (anomalia->sprite_olho != NULL) {
                    float ow = (float)al_get_bitmap_width(anomalia->sprite_olho);
                    float oh = (float)al_get_bitmap_height(anomalia->sprite_olho);
                    float oy = spr_y + (offsets_olho_y[estagio_prev] * 0.5f);
                    al_draw_tinted_scaled_bitmap(anomalia->sprite_olho, cor_prev, 0.0f, 0.0f, ow, oh, spr_x + (17.0f * 0.5f), oy, ow * 0.5f, oh * 0.5f, 0);
                    al_draw_tinted_scaled_bitmap(anomalia->sprite_olho, cor_prev, 0.0f, 0.0f, ow, oh, spr_x + (25.0f * 0.5f), oy, ow * 0.5f, oh * 0.5f, 0);
                }
            }
        } else {
            if (modo_scanner == SCANNER_ESCANEANDO) {
                al_draw_filled_rectangle(box_x + 4.0f, box_y + 3.0f, box_x + box_w - 4.0f, box_y + box_h - 3.0f, al_map_rgba_f(0.0f, 0.0f, 0.0f, 1.0f));
            } else {
                al_draw_filled_rectangle(box_x + 4.0f, box_y + 3.0f, box_x + box_w - 4.0f, box_y + box_h - 3.0f, al_map_rgba_f(0.2f, 0.5f, 0.35f, 0.8f));
                if (elemento_foco_id > 0 && elemento_foco_id <= MAX_ELEMENTOS) {
                    al_use_transform(&trans_texto);
                    al_draw_text(fonte, al_map_rgba_f(1.0f, 1.0f, 1.0f, 0.9f), (box_x + (box_w / 2.0f)) * 2.0f, (box_y + 8.5f) * 2.0f, ALLEGRO_ALIGN_CENTRE, tabela_periodica[elemento_foco_id].simbolo);
                    al_use_transform(&trans_ui);
                }
            }
        }

        // Informações no topo ao lado do preview
        float info_x = box_x + box_w + 5.0f;
        float info_y = box_y + 2.0f;

        if (modo_scanner == SCANNER_ESCANEANDO) {
            al_use_transform(&trans_texto);
            al_draw_text(fonte, cor_destaque, info_x * 2.0f, info_y * 2.0f, 0, "Alvo: Anomalia");
            al_draw_textf(fonte, cor_texto, info_x * 2.0f, (info_y + 6.0f) * 2.0f, 0, "Timer: %.1fs", tempo_scan);
            al_use_transform(&trans_ui);

            float prog = (1.0f - (tempo_scan / 3.0f));
            if (prog < 0.0f) prog = 0.0f;
            if (prog > 1.0f) prog = 1.0f;
            al_draw_rectangle(info_x, info_y + 12.5f, info_x + 50.0f, info_y + 16.5f, al_map_rgba_f(0.3f, 0.8f, 0.5f, 0.6f), 1.0f);
            al_draw_filled_rectangle(info_x + 0.5f, info_y + 13.0f, info_x + 0.5f + (prog * 49.0f), info_y + 16.0f, cor_destaque);
        } else {
            if (elemento_foco_id > 0 && elemento_foco_id <= MAX_ELEMENTOS) {
                al_use_transform(&trans_texto);
                al_draw_textf(fonte, cor_destaque, info_x * 2.0f, (box_y + 8.5f) * 2.0f, 0, "%s (%s)", tabela_periodica[elemento_foco_id].nome, tabela_periodica[elemento_foco_id].simbolo);
                al_use_transform(&trans_ui);
            }
        }

        // Linha divisória da ficha técnica
        al_draw_line(
            SCANNER_PAINEL_X + 4.0f, SCANNER_PAINEL_Y + 38.0f,
            SCANNER_PAINEL_X + SCANNER_PAINEL_W - 4.0f, SCANNER_PAINEL_Y + 38.0f,
            al_map_rgba_f(0.3f, 0.8f, 0.5f, 0.35f), 1.0f
        );

        // Ficha técnica (fonte reduzida em 50% e espaçamento vertical ajustado)
        float tx = (SCANNER_PAINEL_X + 6.0f) * 2.0f;
        float ty = (SCANNER_PAINEL_Y + 40.5f) * 2.0f;
        float lh = 17.0f; // 8.5f de espaçamento vertical no espaço do painel

        al_use_transform(&trans_texto);
        if (modo_scanner == SCANNER_ESCANEANDO) {
            al_draw_text(fonte, cor_texto, tx, ty + (0.0f * lh), 0, "No Atomico:   ???");
            al_draw_text(fonte, cor_texto, tx, ty + (1.0f * lh), 0, "Simbolo:      ???");
            al_draw_text(fonte, cor_texto, tx, ty + (2.0f * lh), 0, "Nome:         ???");
            al_draw_text(fonte, cor_texto, tx, ty + (3.0f * lh), 0, "Categoria:    ???");
            al_draw_text(fonte, cor_texto, tx, ty + (4.0f * lh), 0, "Magnetico:    ???");
            al_draw_text(fonte, cor_texto, tx, ty + (5.0f * lh), 0, "Radioativo:   ???");
            al_draw_text(fonte, cor_texto, tx, ty + (6.0f * lh), 0, "P. Fusao:     ???");
            al_draw_text(fonte, cor_texto, tx, ty + (7.0f * lh), 0, "P. Ebulicao:  ???");
            al_draw_text(fonte, cor_texto, tx, ty + (8.0f * lh), 0, "Reacao Agua:  ???");
        } else {
            if (elemento_foco_id > 0 && elemento_foco_id <= MAX_ELEMENTOS) {
                DadosElemento elem = tabela_periodica[elemento_foco_id];
                al_draw_textf(fonte, cor_texto, tx, ty + (0.0f * lh), 0, "No Atomico:   %d", elem.numero_atomico);
                al_draw_textf(fonte, cor_texto, tx, ty + (1.0f * lh), 0, "Simbolo:      %s", elem.simbolo);
                al_draw_textf(fonte, cor_texto, tx, ty + (2.0f * lh), 0, "Nome:         %s", elem.nome);
                al_draw_textf(fonte, cor_texto, tx, ty + (3.0f * lh), 0, "Categoria:    %s", elem.categoria);
                al_draw_textf(fonte, cor_texto, tx, ty + (4.0f * lh), 0, "Magnetico:    %s", elem.magnetico ? "Sim" : "Nao");
                al_draw_textf(fonte, cor_texto, tx, ty + (5.0f * lh), 0, "Radioativo:   %s", elem.radioativo ? "Sim" : "Nao");
                al_draw_textf(fonte, cor_texto, tx, ty + (6.0f * lh), 0, "P. Fusao:     %.1f C", elem.ponto_fusao);
                al_draw_textf(fonte, cor_texto, tx, ty + (7.0f * lh), 0, "P. Ebulicao:  %.1f C", elem.ponto_ebulicao);
                al_draw_textf(fonte, cor_texto, tx, ty + (8.0f * lh), 0, "Reacao Agua:  %s", elem.reacao_agua);
            }
        }
        al_use_transform(&trans_ui);
    } else if (modo_scanner == SCANNER_MENU) {
        // Cabeçalho do Menu de Elementos Descobertos
        al_use_transform(&trans_texto);
        al_draw_text(fonte, cor_destaque, (SCANNER_PAINEL_X + 58.0f) * 2.0f, (SCANNER_PAINEL_Y + 5.0f) * 2.0f, ALLEGRO_ALIGN_CENTRE, "Anomalias escaneadas");
        al_draw_text(fonte, cor_texto, (SCANNER_PAINEL_X + 5.0f) * 2.0f, (SCANNER_PAINEL_Y + 14.0f) * 2.0f, 0, "Inimigos Catalogados:");
        al_use_transform(&trans_ui);

        // Linha divisória
        al_draw_line(
            SCANNER_PAINEL_X + 4.0f, SCANNER_PAINEL_Y + 20.0f,
            SCANNER_PAINEL_X + SCANNER_PAINEL_W - 4.0f, SCANNER_PAINEL_Y + 20.0f,
            al_map_rgba_f(0.3f, 0.8f, 0.5f, 0.35f), 1.0f
        );

        // Lista vertical de elementos catalogados (apenas descobertos, empilhados sequencialmente)
        int indice_visivel = 0;
        for (int i = 1; i <= MAX_ELEMENTOS; i++) {
            if (!elemento_descoberto || elemento_descoberto[i] != true) {
                continue;
            }

            float item_x = SCANNER_MENU_ITEM_X;
            float item_y = SCANNER_MENU_LIST_Y + ((float)indice_visivel * (SCANNER_MENU_ITEM_H + SCANNER_MENU_ITEM_GAP));
            float item_w = SCANNER_MENU_ITEM_W;
            float item_h = SCANNER_MENU_ITEM_H;
            indice_visivel++;

            if ((item_y + item_h) > (SCANNER_PAINEL_Y + SCANNER_PAINEL_H - 2.0f)) {
                break;
            }

            // Item descoberto
            al_draw_filled_rounded_rectangle(item_x, item_y, item_x + item_w, item_y + item_h, 2.0f, 2.0f, al_map_rgba_f(0.08f, 0.22f, 0.14f, 0.7f));
            al_draw_rounded_rectangle(item_x, item_y, item_x + item_w, item_y + item_h, 2.0f, 2.0f, al_map_rgba_f(0.3f, 0.8f, 0.5f, 0.75f), 1.0f);

            // Miniatura
            if (anomalia && anomalia->sprites_normal[0] != NULL) {
                int estagio_thumb = (tabela_periodica[i].ponto_fusao <= 20.0f) ? 4 : 0;
                ALLEGRO_BITMAP *spr_thumb = anomalia->sprites_normal[estagio_thumb] ? anomalia->sprites_normal[estagio_thumb] : anomalia->sprites_normal[0];
                const float offsets_olho_y[5] = { 11.0f, 13.0f, 15.0f, 18.0f, 22.0f };
                ALLEGRO_COLOR cor_thumb = al_map_rgb(tabela_periodica[i].cor_r, tabela_periodica[i].cor_g, tabela_periodica[i].cor_b);
                float tx0 = item_x + 2.0f;
                float ty0 = item_y + 2.5f;
                float sx = 13.0f / 40.0f;
                float sy = 10.0f / 30.0f;
                al_draw_tinted_scaled_bitmap(spr_thumb, cor_thumb, 0, 0, 40, 30, tx0, ty0, 13.0f, 10.0f, 0);
                if (anomalia->sprite_olho != NULL) {
                    float ow = (float)al_get_bitmap_width(anomalia->sprite_olho);
                    float oh = (float)al_get_bitmap_height(anomalia->sprite_olho);
                    float oy = ty0 + (offsets_olho_y[estagio_thumb] * sy);
                    al_draw_tinted_scaled_bitmap(anomalia->sprite_olho, cor_thumb, 0.0f, 0.0f, ow, oh, tx0 + (17.0f * sx), oy, ow * sx, oh * sy, 0);
                    al_draw_tinted_scaled_bitmap(anomalia->sprite_olho, cor_thumb, 0.0f, 0.0f, ow, oh, tx0 + (25.0f * sx), oy, ow * sx, oh * sy, 0);
                }
            } else {
                al_use_transform(&trans_texto);
                al_draw_text(fonte, cor_destaque, (item_x + 9.0f) * 2.0f, (item_y + 5.5f) * 2.0f, ALLEGRO_ALIGN_CENTRE, tabela_periodica[i].simbolo);
                al_use_transform(&trans_ui);
            }

            al_use_transform(&trans_texto);
            al_draw_textf(fonte, cor_texto, (item_x + 18.0f) * 2.0f, (item_y + 2.5f) * 2.0f, 0, "%s (%s)", tabela_periodica[i].nome, tabela_periodica[i].simbolo);
            al_draw_textf(fonte, al_map_rgba_f(0.6f, 0.9f, 0.7f, 0.65f), (item_x + 18.0f) * 2.0f, (item_y + 8.5f) * 2.0f, 0, "Z=%d | %s", i, tabela_periodica[i].categoria);
            al_draw_text(fonte, cor_destaque, (item_x + item_w - 3.0f) * 2.0f, (item_y + 5.5f) * 2.0f, ALLEGRO_ALIGN_RIGHT, "[VER]");
            al_use_transform(&trans_ui);
        }
    }
}

void desenhar_mundo(int sala_atual, Jogador *faxineiro, Inimigo *anomalia, Item *arma_mop, Item *arma_soprador,
                    Soprador *soprador, Coletavel *refil_agua, Coletavel *bateria, Coletavel *kit_medico,
                    Coletavel *item_scanner, Coletavel *pilula_antirad,
                    Projetil *projeteis, int max_proj, Particula *particulas, int max_particulas,
                    MarcaFuligem *marcas_fuligem, int max_marcas,
                    Plataforma *plataformas, int total_plat,
                    Escada *escadas, int total_escadas, Porta *portas, int total_portas,
                    Elevador *elevadores, int total_elevadores, TextoFlutuante *textos, int max_textos,
                    GameSprites *sprites, bool prop_na_frente, int estado_fade, int alpha_fade,
                    float alpha_game_over, int opcao_game_over,
                    int modo_scanner, float tempo_scan, int elemento_foco_id, const bool *elemento_descoberto,
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

    // Marcas permanentes de fuligem no background (atrás de inimigos, itens e personagens)
    desenhar_marcas_fuligem(marcas_fuligem, max_marcas, sala_atual);

    if (sala_atual == anomalia->sala_origem && anomalia->ativo) {
        DadosElemento elem_anom = tabela_periodica[anomalia->numero_atomico];
        ALLEGRO_COLOR cor_elem = al_map_rgb(elem_anom.cor_r, elem_anom.cor_g, elem_anom.cor_b);

        ALLEGRO_BITMAP *sprite_galio = (!anomalia->morto && anomalia->frame_animacao == 1)
            ? anomalia->sprites_bre[anomalia->frame_atual]
            : anomalia->sprites_normal[anomalia->frame_atual];
        int flag_galio_flip = (anomalia->direcao_atual == DIR_ESQUERDA) ? ALLEGRO_FLIP_HORIZONTAL : 0;

        // Aura de Radiação (Sprite ampliada desenhada por trás do corpo com blending aditivo)
        if (elem_anom.radioativo) {
            int op_ant, src_ant, dst_ant;
            al_get_blender(&op_ant, &src_ant, &dst_ant);
            al_set_blender(ALLEGRO_ADD, ALLEGRO_ALPHA, ALLEGRO_ONE);

            ALLEGRO_COLOR cor_aura_rad = al_map_rgba_f(0.1f, 1.0f, 0.2f, 0.18f);
            float escala_rad = 1.18f;

            if (sprite_galio != NULL) {
                float sw = (float)al_get_bitmap_width(sprite_galio);
                float sh = (float)al_get_bitmap_height(sprite_galio);
                float dw = sw * escala_rad;
                float dh = sh * escala_rad;
                float dx = anomalia->x - (dw / 2.0f);
                float dy = anomalia->y - (dh / 2.0f);
                al_draw_tinted_scaled_bitmap(sprite_galio, cor_aura_rad,
                                            0.0f, 0.0f, sw, sh,
                                            dx, dy, dw, dh, flag_galio_flip);
            } else {
                float dw = anomalia->w * escala_rad;
                float dh = anomalia->h * escala_rad;
                al_draw_filled_rectangle(anomalia->x - (dw / 2.0f), anomalia->y - (dh / 2.0f),
                                         anomalia->x + (dw / 2.0f), anomalia->y + (dh / 2.0f),
                                         cor_aura_rad);
            }

            al_set_blender(op_ant, src_ant, dst_ant);
        }

        desenhar_sprite_quente(sprite_galio, anomalia->x, anomalia->y, anomalia->w, anomalia->h,
                               flag_galio_flip, anomalia->temperatura, anomalia->frame_atual, cor_elem);
        desenhar_olhos_inimigo(anomalia, cor_elem);
    }

    if (arma_mop && arma_mop->no_chao && arma_mop->sala_origem == sala_atual) {
        if (arma_mop->sprite != NULL) {
            float larg_mop = al_get_bitmap_width(arma_mop->sprite);
            float alt_mop = al_get_bitmap_height(arma_mop->sprite);
            al_draw_bitmap(arma_mop->sprite, arma_mop->x - (larg_mop / 2.0f), arma_mop->y - alt_mop, 0);
        } else {
            al_draw_filled_rectangle(arma_mop->x - 2.0f, arma_mop->y - 28.0f, arma_mop->x + 2.0f, arma_mop->y, al_map_rgb(180, 180, 180));
        }
    }

    if (arma_soprador && arma_soprador->no_chao && arma_soprador->sala_origem == sala_atual) {
        ALLEGRO_BITMAP *spr_sop = arma_soprador->sprite ? arma_soprador->sprite : sprites->sprite_soprador;
        if (spr_sop != NULL) {
            float x_desenho = arma_soprador->x - (al_get_bitmap_width(spr_sop) / 2.0f);
            float y_desenho = arma_soprador->y - al_get_bitmap_height(spr_sop) + 2.0f;
            al_draw_bitmap(spr_sop, x_desenho, y_desenho, 0);
        } else {
            al_draw_filled_rectangle(arma_soprador->x - 7.0f, arma_soprador->y - 8.0f, arma_soprador->x + 5.0f, arma_soprador->y - 2.0f, al_map_rgb(220, 100, 20));
            al_draw_filled_rectangle(arma_soprador->x + 5.0f, arma_soprador->y - 7.0f, arma_soprador->x + 9.0f, arma_soprador->y - 3.0f, al_map_rgb(180, 180, 180));
            al_draw_filled_rectangle(arma_soprador->x - 5.0f, arma_soprador->y - 2.0f, arma_soprador->x - 2.0f, arma_soprador->y, al_map_rgb(50, 50, 50));
            al_draw_rectangle(arma_soprador->x - 7.0f, arma_soprador->y - 8.0f, arma_soprador->x + 5.0f, arma_soprador->y - 2.0f, al_map_rgb(40, 40, 40), 1.0f);
        }
    }

    if (refil_agua && refil_agua->ativo && refil_agua->sala == sala_atual) {
        if (sprites->sprite_refil != NULL) {
            float x_desenho = refil_agua->x + (refil_agua->w / 2.0f) - (al_get_bitmap_width(sprites->sprite_refil) / 2.0f);
            float y_desenho = refil_agua->y + refil_agua->h - al_get_bitmap_height(sprites->sprite_refil);
            al_draw_bitmap(sprites->sprite_refil, x_desenho, y_desenho, 0);
        } else {
            al_draw_filled_rectangle(refil_agua->x, refil_agua->y, refil_agua->x + refil_agua->w, refil_agua->y + refil_agua->h, al_map_rgb(60, 160, 255));
            al_draw_rectangle(refil_agua->x, refil_agua->y, refil_agua->x + refil_agua->w, refil_agua->y + refil_agua->h, al_map_rgb(255, 255, 255), 1.0f);
        }
    }

    if (bateria && bateria->ativo && bateria->sala == sala_atual) {
        if (sprites->sprite_bateria != NULL) {
            float x_desenho = bateria->x + (bateria->w / 2.0f) - (al_get_bitmap_width(sprites->sprite_bateria) / 2.0f);
            float y_desenho = bateria->y + bateria->h - al_get_bitmap_height(sprites->sprite_bateria);
            al_draw_bitmap(sprites->sprite_bateria, x_desenho, y_desenho, 0);
        } else {
            al_draw_filled_rectangle(bateria->x, bateria->y, bateria->x + bateria->w, bateria->y + bateria->h, al_map_rgb(255, 220, 50));
            al_draw_rectangle(bateria->x, bateria->y, bateria->x + bateria->w, bateria->y + bateria->h, al_map_rgb(255, 255, 255), 1.0f);
        }
    }

    if (kit_medico && kit_medico->ativo && kit_medico->sala == sala_atual) {
        if (sprites->sprite_medkit != NULL) {
            float x_desenho = kit_medico->x + (kit_medico->w / 2.0f) - (al_get_bitmap_width(sprites->sprite_medkit) / 2.0f);
            float y_desenho = kit_medico->y + kit_medico->h - al_get_bitmap_height(sprites->sprite_medkit);
            al_draw_bitmap(sprites->sprite_medkit, x_desenho, y_desenho, 0);
        } else {
            al_draw_filled_rectangle(kit_medico->x, kit_medico->y, kit_medico->x + kit_medico->w, kit_medico->y + kit_medico->h, al_map_rgb(240, 240, 240));
            al_draw_rectangle(kit_medico->x, kit_medico->y, kit_medico->x + kit_medico->w, kit_medico->y + kit_medico->h, al_map_rgb(60, 60, 60), 1.0f);
            float cx = kit_medico->x + kit_medico->w / 2.0f;
            float cy = kit_medico->y + kit_medico->h / 2.0f;
            al_draw_filled_rectangle(cx - 1.0f, cy - 4.0f, cx + 1.0f, cy + 4.0f, al_map_rgb(220, 30, 30));
            al_draw_filled_rectangle(cx - 4.0f, cy - 1.0f, cx + 4.0f, cy + 1.0f, al_map_rgb(220, 30, 30));
        }
    }

    if (item_scanner && item_scanner->ativo && item_scanner->sala == sala_atual) {
        if (sprites->sprite_scanner != NULL) {
            float x_desenho = item_scanner->x + (item_scanner->w / 2.0f) - (al_get_bitmap_width(sprites->sprite_scanner) / 2.0f);
            float y_desenho = item_scanner->y + item_scanner->h - al_get_bitmap_height(sprites->sprite_scanner);
            al_draw_bitmap(sprites->sprite_scanner, x_desenho, y_desenho, 0);
        } else {
            al_draw_filled_rectangle(item_scanner->x, item_scanner->y, item_scanner->x + item_scanner->w, item_scanner->y + item_scanner->h, al_map_rgb(80, 220, 140));
            al_draw_rectangle(item_scanner->x, item_scanner->y, item_scanner->x + item_scanner->w, item_scanner->y + item_scanner->h, al_map_rgb(20, 80, 50), 1.0f);
            al_draw_filled_rectangle(item_scanner->x + 2.0f, item_scanner->y + 2.0f, item_scanner->x + item_scanner->w - 2.0f, item_scanner->y + 5.0f, al_map_rgb(180, 255, 220));
        }
    }

    if (pilula_antirad && pilula_antirad->ativo && pilula_antirad->sala == sala_atual) {
        if (sprites->sprite_anti_radiacao != NULL) {
            float x_desenho = pilula_antirad->x + (pilula_antirad->w / 2.0f) - (al_get_bitmap_width(sprites->sprite_anti_radiacao) / 2.0f);
            float y_desenho = pilula_antirad->y + pilula_antirad->h - al_get_bitmap_height(sprites->sprite_anti_radiacao);
            al_draw_bitmap(sprites->sprite_anti_radiacao, x_desenho, y_desenho, 0);
        } else {
            // Fallback em formato de pílula / frasco de anti-radiação
            float px = pilula_antirad->x;
            float py = pilula_antirad->y;
            float pw = pilula_antirad->w;
            float ph = pilula_antirad->h;
            al_draw_filled_rectangle(px, py, px + pw * 0.5f, py + ph, al_map_rgb(40, 210, 70));
            al_draw_filled_rectangle(px + pw * 0.5f, py, px + pw, py + ph, al_map_rgb(240, 240, 240));
            al_draw_rectangle(px, py, px + pw, py + ph, al_map_rgb(20, 80, 30), 1.0f);
        }
    }

    for (int i = 0; i < max_proj; i++) {
        if (projeteis[i].ativo) {
            if (projeteis[i].tipo == PROJETIL_AGUA) {
                // Jato de água procedural: retângulo de 6x2 pixels azul claro translúcido
                al_draw_filled_rectangle(
                    projeteis[i].x - 3.0f, projeteis[i].y - 1.0f,
                    projeteis[i].x + 3.0f, projeteis[i].y + 1.0f,
                    al_map_rgba(0, 150, 255, 200)
                );
            } else {
                al_draw_filled_circle(projeteis[i].x, projeteis[i].y, 2, al_map_rgb(100, 200, 255));
            }
        }
    }

    int tipo_frame_desenhar = FRAME_IDLE;
    bool tem_sprites = (faxineiro->sprites[FRAME_IDLE] != NULL);
    if (faxineiro->esta_andando) {
        int sequencia_andar[] = {FRAME_WALK1, FRAME_IDLE, FRAME_WALK2, FRAME_IDLE};
        tipo_frame_desenhar = sequencia_andar[faxineiro->frame_atual];
    } else {
        int sequencia_parado[] = {FRAME_IDLE, FRAME_BRE};
        tipo_frame_desenhar = sequencia_parado[faxineiro->frame_atual];
    }

    int flag_espelhamento = (faxineiro->direcao_atual == DIR_ESQUERDA) ? ALLEGRO_FLIP_HORIZONTAL : 0;

    // Arma NÃO equipada: desenhada nas costas em posição vertical (atrás do corpo)
    // Deslocada para CIMA em 50% da altura do sprite/placeholder da arma
    float costas_x = (faxineiro->direcao_atual == DIR_DIREITA) ? (faxineiro->x - 5.0f) : (faxineiro->x + 5.0f);
    float costas_y = faxineiro->y - (faxineiro->body_h / 4.0f);
    if (tipo_frame_desenhar == FRAME_BRE) {
        costas_y += 1.0f;
    }

    if (faxineiro->arma_equipada == ARMA_MOP && faxineiro->possui_soprador) {
        // Soprador Térmico nas costas (isolado: bocal sempre apontando para cima)
        ALLEGRO_BITMAP *spr_sop = (arma_soprador && arma_soprador->sprite) ? arma_soprador->sprite : sprites->sprite_soprador;
        if (spr_sop != NULL) {
            float larg_sop = al_get_bitmap_width(spr_sop);
            float alt_sop = al_get_bitmap_height(spr_sop);
            float sop_y = costas_y - (larg_sop * 0.5f);
            float angulo_sop_costas = -ALLEGRO_PI / 2.0f;
            int flag_sop_costas = (faxineiro->direcao_atual == DIR_ESQUERDA) ? ALLEGRO_FLIP_VERTICAL : 0;
            al_draw_rotated_bitmap(spr_sop, larg_sop / 2.0f, alt_sop / 2.0f, costas_x, sop_y, angulo_sop_costas, flag_sop_costas);
        } else {
            float sop_y = costas_y - 8.0f;
            al_draw_filled_rectangle(costas_x - 3.0f, sop_y - 8.0f, costas_x + 3.0f, sop_y + 8.0f, al_map_rgb(220, 100, 20));
            al_draw_filled_rectangle(costas_x - 2.0f, sop_y - 12.0f, costas_x + 2.0f, sop_y - 8.0f, al_map_rgb(170, 170, 170));
            al_draw_rectangle(costas_x - 3.0f, sop_y - 8.0f, costas_x + 3.0f, sop_y + 8.0f, al_map_rgb(40, 40, 40), 1.0f);
        }
    } else if (faxineiro->arma_equipada == ARMA_SOPRADOR && faxineiro->possui_mop) {
        // Mop nas costas (vertical, deslocado 50% para cima)
        if (arma_mop && arma_mop->sprite != NULL) {
            float larg_mop = al_get_bitmap_width(arma_mop->sprite);
            float alt_mop = al_get_bitmap_height(arma_mop->sprite);
            float mop_y = costas_y - (alt_mop * 0.5f);
            int flag_mop_costas = (faxineiro->direcao_atual == DIR_ESQUERDA) ? ALLEGRO_FLIP_HORIZONTAL : 0;
            al_draw_bitmap(arma_mop->sprite, costas_x - (larg_mop / 2.0f), mop_y - (alt_mop / 2.0f), flag_mop_costas);
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
                    float angulo_morte = (faxineiro->direcao_atual == DIR_DIREITA) ? (-ALLEGRO_PI / 2.0f) : (ALLEGRO_PI / 2.0f);
                    al_draw_rotated_bitmap(sprite_final, larg_img / 2.0f, alt_img / 2.0f,
                                           faxineiro->x, chao_y - (faxineiro->body_w / 2.0f),
                                           angulo_morte, flag_espelhamento);
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

        if (!faxineiro->morreu && faxineiro->scanner_ativo) {
            float body_top = (faxineiro->y + (faxineiro->h / 2.0f)) - faxineiro->body_h;
            float olho_y = body_top + 7.0f;
            if (tipo_frame_desenhar == FRAME_BRE) olho_y += 1.0f;

            if (sprites->sprite_scanner != NULL) {
                float sw = (float)al_get_bitmap_width(sprites->sprite_scanner);
                float sh = (float)al_get_bitmap_height(sprites->sprite_scanner);
                float offset_x = (faxineiro->direcao_atual == DIR_DIREITA) ? 2.0f : -2.0f;
                al_draw_bitmap(sprites->sprite_scanner, faxineiro->x + offset_x - (sw / 2.0f), olho_y - (sh / 2.0f), flag_espelhamento);
            } else {
                float olho_w = 6.0f;
                float olho_h = 3.0f;
                float offset_x = (faxineiro->direcao_atual == DIR_DIREITA) ? 1.0f : -7.0f;
                al_draw_filled_rectangle(faxineiro->x + offset_x, olho_y - 1.0f,
                                         faxineiro->x + offset_x + olho_w, olho_y - 1.0f + olho_h,
                                         al_map_rgba(80, 255, 120, 180));
                al_draw_rectangle(faxineiro->x + offset_x, olho_y - 1.0f,
                                  faxineiro->x + offset_x + olho_w, olho_y - 1.0f + olho_h,
                                  al_map_rgba(150, 255, 180, 220), 1.0f);
            }
        }
    }

    // Arma EQUIPADA: desenhada na mão do personagem (independentemente de possuir o Mop ou não)
    float mao_y = faxineiro->y - (faxineiro->body_h / 2.0) + 4;
    if (tipo_frame_desenhar == FRAME_BRE) mao_y += 1; else if (tipo_frame_desenhar == FRAME_WALK1) mao_y -= 2;

    float offset_arma_x = (faxineiro->direcao_atual == DIR_DIREITA) ? 7.0f : -7.0f;

    if (faxineiro->arma_equipada == ARMA_MOP && faxineiro->possui_mop) {
        float mao_x = ((faxineiro->direcao_atual == DIR_DIREITA) ? (faxineiro->x - 6.0f) : (faxineiro->x + 6.0f)) + offset_arma_x;
        if (arma_mop && arma_mop->sprite != NULL) {
            float larg_mop = al_get_bitmap_width(arma_mop->sprite);
            float alt_mop = al_get_bitmap_height(arma_mop->sprite);
            float angulo_rotacao = (faxineiro->direcao_atual == DIR_DIREITA) ? (-ALLEGRO_PI / 2.0) : (ALLEGRO_PI / 2.0);
            al_draw_rotated_bitmap(arma_mop->sprite, larg_mop / 2.0, alt_mop / 2.0, mao_x, mao_y, angulo_rotacao, flag_espelhamento);
        } else {
            al_draw_filled_rectangle(mao_x - 14.0f, mao_y - 2.0f, mao_x + 14.0f, mao_y + 2.0f, al_map_rgb(180, 180, 180));
        }
    } else if (faxineiro->arma_equipada == ARMA_SOPRADOR && faxineiro->possui_soprador) {
        ALLEGRO_BITMAP *spr_sop = (arma_soprador && arma_soprador->sprite) ? arma_soprador->sprite : sprites->sprite_soprador;
        if (spr_sop != NULL) {
            float mao_x = ((faxineiro->direcao_atual == DIR_DIREITA) ? (faxineiro->x - 6.0f) : (faxineiro->x + 6.0f)) + offset_arma_x;
            float larg_sop = al_get_bitmap_width(spr_sop);
            float alt_sop = al_get_bitmap_height(spr_sop);
            int flag_sop = (faxineiro->direcao_atual == DIR_ESQUERDA) ? ALLEGRO_FLIP_HORIZONTAL : 0;
            al_draw_bitmap(spr_sop, mao_x - (larg_sop / 2.0f), mao_y - (alt_sop / 2.0f), flag_sop);
        } else {
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
        }
        (void)soprador;
    }

    desenhar_particulas(particulas, max_particulas);

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
        if (item_scanner && item_scanner->ativo && item_scanner->sala == sala_atual) {
            al_draw_rectangle(item_scanner->x, item_scanner->y, item_scanner->x + item_scanner->w, item_scanner->y + item_scanner->h, al_map_rgb(80, 255, 120), 1.0f);
            al_draw_text(fonte, al_map_rgb(80, 255, 120), item_scanner->x + item_scanner->w / 2.0f, item_scanner->y - 10, ALLEGRO_ALIGN_CENTRE, "Scanner [T]");
        }
        if (pilula_antirad && pilula_antirad->ativo && pilula_antirad->sala == sala_atual) {
            al_draw_rectangle(pilula_antirad->x, pilula_antirad->y, pilula_antirad->x + pilula_antirad->w, pilula_antirad->y + pilula_antirad->h, al_map_rgb(50, 230, 80), 1.0f);
            al_draw_text(fonte, al_map_rgb(50, 230, 80), pilula_antirad->x + pilula_antirad->w / 2.0f, pilula_antirad->y - 10, ALLEGRO_ALIGN_CENTRE, "Anti-Rad");
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

        if (sala_atual == anomalia->sala_origem && anomalia->ativo) {
            al_draw_textf(fonte, al_map_rgb(255, 150, 150), anomalia->x, anomalia->y - anomalia->h / 2 - 15, ALLEGRO_ALIGN_CENTRE, "%.1f C (HP: %.0f)", anomalia->temperatura, anomalia->hp);
        }
    }

    // Transformação para UI em coordenadas de tela fixas (480x270)
    ALLEGRO_TRANSFORM camera_ui;
    al_identity_transform(&camera_ui);
    al_scale_transform(&camera_ui, (float)LARGURA_JANELA / LARGURA_MAPA, (float)ALTURA_JANELA / ALTURA_MAPA);
    al_use_transform(&camera_ui);

    // --- HUD Superior Esquerdo (Pixel Art + Preenchimentos Dinâmicos por Código) ---
    desenhar_hud_jogador(faxineiro, sprites);

    // --- Visor / Scanner Químico (Canto Superior Direito) ---
    desenhar_visor_scanner(modo_scanner, tempo_scan, elemento_foco_id, elemento_descoberto, anomalia, fonte);

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
        const char *itens[] = {"Novo Jogo", "Continuar", "Tabela Periódica", "Recorde", "Configuracao"};
        for (int i = 0; i < 5; i++) {
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

void obter_coordenadas_tabela(int z, int *gx, int *gy) {
    if (!gx || !gy) return;
    if (z == 0) {
        *gx = 8; *gy = 0; // Neutrônio (Elemento 0) no topo centro fora do bloco principal
    } else if (z == 1) {
        *gx = 0; *gy = 0; // H
    } else if (z == 2) {
        *gx = 17; *gy = 0; // He
    } else if (z >= 3 && z <= 4) {
        *gx = z - 3; *gy = 1; // Li, Be
    } else if (z >= 5 && z <= 10) {
        *gx = z + 7; *gy = 1; // B .. Ne
    } else if (z >= 11 && z <= 12) {
        *gx = z - 11; *gy = 2; // Na, Mg
    } else if (z >= 13 && z <= 18) {
        *gx = z - 1; *gy = 2; // Al .. Ar
    } else if (z >= 19 && z <= 36) {
        *gx = z - 19; *gy = 3; // K .. Kr
    } else if (z >= 37 && z <= 54) {
        *gx = z - 37; *gy = 4; // Rb .. Xe
    } else if (z >= 55 && z <= 56) {
        *gx = z - 55; *gy = 5; // Cs, Ba
    } else if (z >= 57 && z <= 71) {
        *gx = z - 55; *gy = 7; // Lantanídeos (La .. Lu) na linha 7 separados
    } else if (z >= 72 && z <= 86) {
        *gx = z - 69; *gy = 5; // Hf .. Rn
    } else if (z >= 87 && z <= 88) {
        *gx = z - 87; *gy = 6; // Fr, Ra
    } else if (z >= 89 && z <= 103) {
        *gx = z - 87; *gy = 8; // Actinídeos (Ac .. Lr) na linha 8 separados
    } else if (z >= 104 && z <= 118) {
        *gx = z - 101; *gy = 6; // Rf .. Og
    } else {
        *gx = -1; *gy = -1;
    }
}

static void obter_retangulo_celula_tabela(int z, float *rx, float *ry, float *rw, float *rh) {
    int gx = -1, gy = -1;
    obter_coordenadas_tabela(z, &gx, &gy);
    if (gx < 0 || gy < 0) {
        *rx = -100.0f; *ry = -100.0f; *rw = 0.0f; *rh = 0.0f;
        return;
    }
    const float grid_x0 = 8.0f;
    const float grid_y0 = 36.0f;
    const float cell_w = 15.0f;
    const float cell_h = 16.0f;
    const float step_x = 16.0f;
    const float step_y = 18.0f;

    *rx = grid_x0 + ((float)gx * step_x);
    *ry = grid_y0 + ((float)gy * step_y) + ((gy >= 7) ? 6.0f : 0.0f);
    if (z == 0) {
        *ry -= 3.0f;
    }
    *rw = cell_w;
    *rh = cell_h;
}

int tabela_periodica_primeiro_descoberto(const bool *elemento_descoberto) {
    if (elemento_descoberto) {
        for (int z = 0; z <= MAX_ELEMENTOS; z++) {
            if (elemento_descoberto[z]) return z;
        }
    }
    return 1; // Padrão: Hidrogênio (Z=1) caso nenhum tenha sido descoberto ainda
}

int tabela_periodica_obter_elemento_clicado(float mouse_x, float mouse_y) {
    for (int z = 0; z <= MAX_ELEMENTOS; z++) {
        float rx, ry, rw, rh;
        obter_retangulo_celula_tabela(z, &rx, &ry, &rw, &rh);
        if (mouse_x >= rx && mouse_x <= rx + rw && mouse_y >= ry && mouse_y <= ry + rh) {
            return z;
        }
    }
    return -1;
}

bool tabela_periodica_clique_voltar(float mouse_x, float mouse_y) {
    return (mouse_x >= 8.0f && mouse_x <= 112.0f && mouse_y >= 240.0f && mouse_y <= 260.0f);
}

bool tabela_periodica_clique_reset(float mouse_x, float mouse_y, bool mostrar_botao_reset) {
    if (!mostrar_botao_reset) return false;
    return (mouse_x >= 8.0f && mouse_x <= 148.0f && mouse_y >= 214.0f && mouse_y <= 234.0f);
}

int tabela_periodica_navegar(int z_atual, int dx, int dy) {
    if (z_atual < 0 || z_atual > MAX_ELEMENTOS) z_atual = 1;
    int grid[9][18];
    for (int r = 0; r < 9; r++) {
        for (int c = 0; c < 18; c++) {
            grid[r][c] = -1;
        }
    }
    for (int z = 0; z <= MAX_ELEMENTOS; z++) {
        int gx = -1, gy = -1;
        obter_coordenadas_tabela(z, &gx, &gy);
        if (gx >= 0 && gx < 18 && gy >= 0 && gy < 9) {
            grid[gy][gx] = z;
        }
    }

    int cur_gx = 0, cur_gy = 0;
    obter_coordenadas_tabela(z_atual, &cur_gx, &cur_gy);

    if (dx != 0) {
        for (int step = 1; step <= 18; step++) {
            int nx = (cur_gx + dx * step + 18) % 18;
            if (grid[cur_gy][nx] != -1) {
                return grid[cur_gy][nx];
            }
        }
    }

    if (dy != 0) {
        // Primeiro tenta encontrar um elemento na mesma coluna (pulando espaços vazios verticais)
        for (int step = 1; step <= 9; step++) {
            int ny = (cur_gy + dy * step + 9) % 9;
            if (grid[ny][cur_gx] != -1) {
                return grid[ny][cur_gx];
            }
        }
        // Caso a coluna não possua outros elementos, escolhe o mais próximo na próxima linha válida
        for (int step = 1; step < 9; step++) {
            int ny = (cur_gy + dy * step + 9) % 9;
            int melhor_z = -1;
            int menor_dist = 999;
            for (int c = 0; c < 18; c++) {
                if (grid[ny][c] != -1) {
                    int dist = abs(c - cur_gx);
                    if (dist < menor_dist) {
                        menor_dist = dist;
                        melhor_z = grid[ny][c];
                    }
                }
            }
            if (melhor_z != -1) {
                return melhor_z;
            }
        }
    }

    return z_atual;
}

static int calcular_estagio_viscosidade_20c(float temp, float ponto_fusao) {
    if (ponto_fusao <= 20.0f) return 4;
    if (temp >= ponto_fusao) return 4;
    if (temp <= 20.0f) return 0;
    float progresso = (temp - 20.0f) / (ponto_fusao - 20.0f);
    if (progresso < 0.33f) return 1;
    if (progresso < 0.66f) return 2;
    return 3;
}

void desenhar_menu_tabela_periodica(int elemento_selecionado, int frame_anim,
                                    const bool *descoberto_global,
                                    const char descobridor_global[MAX_ELEMENTOS + 1][32],
                                    const bool *novo_na_tabela,
                                    bool mostrar_botao_reset,
                                    const Inimigo *anomalia,
                                    ALLEGRO_FONT *fonte) {
    if (!fonte) return;
    if (elemento_selecionado < 0 || elemento_selecionado > MAX_ELEMENTOS) {
        elemento_selecionado = 1;
    }

    ALLEGRO_TRANSFORM trans_ui = *al_get_current_transform();
    ALLEGRO_TRANSFORM trans_texto;
    al_identity_transform(&trans_texto);
    al_scale_transform(&trans_texto, 0.5f, 0.5f);
    al_compose_transform(&trans_texto, &trans_ui);

    // Fundo da tela da Tabela Periódica
    al_draw_filled_rectangle(0.0f, 0.0f, LARGURA_MAPA, ALTURA_MAPA, al_map_rgb(16, 20, 24));

    // Cabeçalho superior
    al_draw_filled_rectangle(0.0f, 0.0f, LARGURA_MAPA, 24.0f, al_map_rgb(24, 34, 42));
    al_draw_line(0.0f, 24.0f, LARGURA_MAPA, 24.0f, al_map_rgb(70, 180, 120), 1.0f);
    al_draw_text(fonte, al_map_rgb(110, 255, 160), 10.0f, 8.0f, ALLEGRO_ALIGN_LEFT, "TABELA PERIODICA - BANCO QUIMICO ACHERON-9");

    int total_desc = 0;
    if (descoberto_global) {
        for (int z = 0; z <= MAX_ELEMENTOS; z++) {
            if (descoberto_global[z]) total_desc++;
        }
    }
    al_use_transform(&trans_texto);
    al_draw_textf(fonte, al_map_rgb(180, 230, 200), (LARGURA_MAPA - 10.0f) * 2.0f, 18.0f, ALLEGRO_ALIGN_RIGHT,
                  "CATALOGADOS: %d / %d", total_desc, MAX_ELEMENTOS + 1);
    al_use_transform(&trans_ui);

    // --- 1. GRID DA TABELA PERIÓDICA (Esquerda da Tela: 18 colunas x 9 linhas) ---
    // Passo 1: Desenha todas as células do grid (fundos, bordas e símbolos)
    for (int z = 0; z <= MAX_ELEMENTOS; z++) {
        float rx, ry, rw, rh;
        obter_retangulo_celula_tabela(z, &rx, &ry, &rw, &rh);
        bool descoberto = (descoberto_global && descoberto_global[z]);
        DadosElemento el = tabela_periodica[z];

        if (descoberto) {
            ALLEGRO_COLOR cor_bloco = al_map_rgb(el.cor_r, el.cor_g, el.cor_b);
            al_draw_filled_rectangle(rx, ry, rx + rw, ry + rh, cor_bloco);
            al_draw_rectangle(rx, ry, rx + rw, ry + rh, al_map_rgb(25, 30, 35), 1.0f);

            int lum = (el.cor_r * 299 + el.cor_g * 587 + el.cor_b * 114) / 1000;
            ALLEGRO_COLOR cor_simbolo = (lum > 140) ? al_map_rgb(15, 18, 22) : al_map_rgb(245, 250, 255);

            al_use_transform(&trans_texto);
            al_draw_textf(fonte, cor_simbolo, (rx + 1.5f) * 2.0f, (ry + 1.5f) * 2.0f, ALLEGRO_ALIGN_LEFT, "%d", z);
            al_draw_text(fonte, cor_simbolo, (rx + (rw * 0.5f)) * 2.0f, (ry + 7.5f) * 2.0f, ALLEGRO_ALIGN_CENTRE, el.simbolo);
            al_use_transform(&trans_ui);
        } else {
            al_draw_filled_rectangle(rx, ry, rx + rw, ry + rh, al_map_rgb(42, 46, 52));
            al_draw_rectangle(rx, ry, rx + rw, ry + rh, al_map_rgb(78, 84, 94), 1.0f);

            al_use_transform(&trans_texto);
            al_draw_text(fonte, al_map_rgb(160, 170, 180), (rx + (rw * 0.5f)) * 2.0f, (ry + 5.5f) * 2.0f, ALLEGRO_ALIGN_CENTRE, "?");
            al_use_transform(&trans_ui);
        }
    }

    // Passo 2: Destaque amarelo da célula selecionada desenhado após todas as células (evita sobreposição)
    if (elemento_selecionado >= 0 && elemento_selecionado <= MAX_ELEMENTOS) {
        float sel_rx, sel_ry, sel_rw, sel_rh;
        obter_retangulo_celula_tabela(elemento_selecionado, &sel_rx, &sel_ry, &sel_rw, &sel_rh);
        al_draw_rectangle(sel_rx - 1.0f, sel_ry - 1.0f, sel_rx + sel_rw + 1.0f, sel_ry + sel_rh + 1.0f, al_map_rgb(255, 235, 60), 1.5f);
    }

    // Passo 3: Tag animada "NOVO" pulsando e rotacionada na diagonal superior direita
    if (novo_na_tabela) {
        float tempo = (float)al_get_time();
        float pulse = 0.35f + 0.05f * sinf(tempo * 6.0f);

        for (int z = 0; z <= MAX_ELEMENTOS; z++) {
            if (!novo_na_tabela[z]) continue;

            float rx, ry, rw, rh;
            obter_retangulo_celula_tabela(z, &rx, &ry, &rw, &rh);

            ALLEGRO_TRANSFORM t_novo;
            al_identity_transform(&t_novo);
            al_scale_transform(&t_novo, pulse, pulse);
            al_rotate_transform(&t_novo, 0.65f);
            al_translate_transform(&t_novo, rx + rw - 3.5f, ry + 3.5f);
            al_compose_transform(&t_novo, &trans_ui);
            al_use_transform(&t_novo);

            al_draw_filled_rectangle(-12.0f, -4.5f, 12.0f, 4.5f, al_map_rgba(10, 35, 20, 230));
            al_draw_rectangle(-12.0f, -4.5f, 12.0f, 4.5f, al_map_rgb(50, 220, 100), 1.0f);
            al_draw_text(fonte, al_map_rgb(50, 220, 100), 0.0f, -4.0f, ALLEGRO_ALIGN_CENTRE, "NOVO");
        }
        al_use_transform(&trans_ui);
    }

    // Botão Secreto de Reset do Banco Químico (revelado via F3 + B)
    if (mostrar_botao_reset) {
        al_draw_filled_rectangle(8.0f, 214.0f, 148.0f, 234.0f, al_map_rgb(55, 20, 20));
        al_draw_rectangle(8.0f, 214.0f, 148.0f, 234.0f, al_map_rgb(240, 70, 70), 1.0f);
        al_use_transform(&trans_texto);
        al_draw_text(fonte, al_map_rgb(255, 130, 130), 78.0f * 2.0f, 221.5f * 2.0f, ALLEGRO_ALIGN_CENTRE, "[RESETAR BANCO QUIMICO]");
        al_use_transform(&trans_ui);
    }

    // Botão Voltar e Dica de Navegação
    al_draw_filled_rectangle(8.0f, 240.0f, 112.0f, 260.0f, al_map_rgb(35, 55, 45));
    al_draw_rectangle(8.0f, 240.0f, 112.0f, 260.0f, al_map_rgb(90, 220, 140), 1.0f);
    al_use_transform(&trans_texto);
    al_draw_text(fonte, al_map_rgb(140, 255, 180), 60.0f * 2.0f, 247.5f * 2.0f, ALLEGRO_ALIGN_CENTRE, "VOLTAR AO MENU [ESC]");
    al_draw_text(fonte, al_map_rgb(150, 175, 165), 120.0f * 2.0f, 247.5f * 2.0f, ALLEGRO_ALIGN_LEFT, "WASD / SETAS / MOUSE: Selecionar");
    al_use_transform(&trans_ui);

    // --- 2. PAINEL DE DETALHES (Direita da Tela) ---
    float px = 300.0f;
    float py = 30.0f;
    float pw = 172.0f;
    float ph = 232.0f;

    al_draw_filled_rounded_rectangle(px, py, px + pw, py + ph, 4.0f, 4.0f, al_map_rgb(22, 30, 36));
    al_draw_rounded_rectangle(px, py, px + pw, py + ph, 4.0f, 4.0f, al_map_rgb(75, 190, 130), 1.2f);

    DadosElemento sel = tabela_periodica[elemento_selecionado];
    bool sel_descoberto = (descoberto_global && descoberto_global[elemento_selecionado]);
    ALLEGRO_COLOR cor_elem = al_map_rgb(sel.cor_r, sel.cor_g, sel.cor_b);

    // Crachá do elemento selecionado no topo do painel
    float badge_x = px + 7.0f;
    float badge_y = py + 7.0f;
    float badge_s = 24.0f;

    if (sel_descoberto) {
        al_draw_filled_rectangle(badge_x, badge_y, badge_x + badge_s, badge_y + badge_s, cor_elem);
        al_draw_rectangle(badge_x, badge_y, badge_x + badge_s, badge_y + badge_s, al_map_rgb(240, 240, 240), 1.0f);

        int lum_sel = (sel.cor_r * 299 + sel.cor_g * 587 + sel.cor_b * 114) / 1000;
        ALLEGRO_COLOR cor_txt_badge = (lum_sel > 140) ? al_map_rgb(15, 18, 22) : al_map_rgb(245, 250, 255);
        al_use_transform(&trans_texto);
        al_draw_textf(fonte, cor_txt_badge, (badge_x + 2.0f) * 2.0f, (badge_y + 2.0f) * 2.0f, ALLEGRO_ALIGN_LEFT, "%d", sel.numero_atomico);
        al_use_transform(&trans_ui);
        al_draw_text(fonte, cor_txt_badge, badge_x + (badge_s * 0.5f), badge_y + 10.0f, ALLEGRO_ALIGN_CENTRE, sel.simbolo);

        al_use_transform(&trans_texto);
        al_draw_textf(fonte, al_map_rgb(130, 255, 175), (badge_x + badge_s + 6.0f) * 2.0f, (badge_y + 4.0f) * 2.0f, ALLEGRO_ALIGN_LEFT,
                      "%s (%s)", sel.nome, sel.simbolo);
        al_draw_textf(fonte, al_map_rgb(190, 215, 205), (badge_x + badge_s + 6.0f) * 2.0f, (badge_y + 14.0f) * 2.0f, ALLEGRO_ALIGN_LEFT,
                      "No Atomico Z = %d | Per. %d", sel.numero_atomico, sel.periodo);
        al_use_transform(&trans_ui);
    } else {
        al_draw_filled_rectangle(badge_x, badge_y, badge_x + badge_s, badge_y + badge_s, al_map_rgb(42, 46, 52));
        al_draw_rectangle(badge_x, badge_y, badge_x + badge_s, badge_y + badge_s, al_map_rgb(78, 84, 94), 1.0f);

        ALLEGRO_COLOR cor_txt_badge = al_map_rgb(160, 170, 180);
        al_use_transform(&trans_texto);
        al_draw_textf(fonte, cor_txt_badge, (badge_x + 2.0f) * 2.0f, (badge_y + 2.0f) * 2.0f, ALLEGRO_ALIGN_LEFT, "%d", sel.numero_atomico);
        al_use_transform(&trans_ui);
        al_draw_text(fonte, cor_txt_badge, badge_x + (badge_s * 0.5f), badge_y + 10.0f, ALLEGRO_ALIGN_CENTRE, "?");

        al_use_transform(&trans_texto);
        al_draw_text(fonte, al_map_rgb(130, 255, 175), (badge_x + badge_s + 6.0f) * 2.0f, (badge_y + 4.0f) * 2.0f, ALLEGRO_ALIGN_LEFT,
                     "Desconhecido");
        al_draw_textf(fonte, al_map_rgb(190, 215, 205), (badge_x + badge_s + 6.0f) * 2.0f, (badge_y + 14.0f) * 2.0f, ALLEGRO_ALIGN_LEFT,
                      "No Atomico Z = %d", sel.numero_atomico);
        al_use_transform(&trans_ui);
    }

    // Câmara de Simulação do Golem
    float cam_x = px + 7.0f;
    float cam_y = py + 36.0f;
    float cam_w = pw - 14.0f;
    float cam_h = 62.0f;
    al_draw_filled_rectangle(cam_x, cam_y, cam_x + cam_w, cam_y + cam_h, al_map_rgb(14, 18, 22));
    al_draw_filled_rectangle(cam_x, cam_y + cam_h - 10.0f, cam_x + cam_w, cam_y + cam_h, al_map_rgb(36, 42, 48));
    al_draw_line(cam_x, cam_y + cam_h - 10.0f, cam_x + cam_w, cam_y + cam_h - 10.0f, al_map_rgb(70, 85, 95), 1.0f);
    al_draw_rectangle(cam_x, cam_y, cam_x + cam_w, cam_y + cam_h, al_map_rgb(55, 130, 95), 1.0f);

    if (sel_descoberto) {
        float temp_sim = 20.0f;
        int estagio_20c = calcular_estagio_viscosidade_20c(temp_sim, sel.ponto_fusao);

        float golem_cx = cam_x + (cam_w * 0.5f);
        float golem_chao_y = cam_y + cam_h - 10.0f;
        float golem_x = golem_cx - 20.0f;
        float golem_y = golem_chao_y - 30.0f;

        ALLEGRO_BITMAP *spr_corpo = NULL;
        if (anomalia && anomalia->sprites_normal[estagio_20c] != NULL) {
            spr_corpo = (frame_anim == 1 && anomalia->sprites_bre[estagio_20c] != NULL)
                        ? anomalia->sprites_bre[estagio_20c]
                        : anomalia->sprites_normal[estagio_20c];
        }

        // Aura de Radiação: sprite ampliada por trás do corpo com blending aditivo
        if (sel.radioativo) {
            int op_ant, src_ant, dst_ant;
            al_get_blender(&op_ant, &src_ant, &dst_ant);
            al_set_blender(ALLEGRO_ADD, ALLEGRO_ALPHA, ALLEGRO_ONE);

            ALLEGRO_COLOR cor_aura_rad = al_map_rgba_f(0.1f, 1.0f, 0.2f, 0.18f);
            float escala_rad = 1.18f;

            if (spr_corpo != NULL) {
                float sw = (float)al_get_bitmap_width(spr_corpo);
                float sh = (float)al_get_bitmap_height(spr_corpo);
                float dw = sw * escala_rad;
                float dh = sh * escala_rad;
                float dx = golem_cx - (dw * 0.5f);
                float dy = (golem_y + 15.0f) - (dh * 0.5f);
                al_draw_tinted_scaled_bitmap(spr_corpo, cor_aura_rad,
                                             0.0f, 0.0f, sw, sh,
                                             dx, dy, dw, dh, 0);
            } else {
                float dw = 28.0f * escala_rad;
                float dh = 24.0f * escala_rad;
                al_draw_filled_rectangle(golem_cx - (dw * 0.5f), (golem_y + 15.0f) - (dh * 0.5f),
                                         golem_cx + (dw * 0.5f), (golem_y + 15.0f) + (dh * 0.5f),
                                         cor_aura_rad);
            }

            al_set_blender(op_ant, src_ant, dst_ant);
        }

        if (spr_corpo != NULL) {
            al_draw_tinted_bitmap(spr_corpo, cor_elem, golem_x, golem_y, 0);

            const float offsets_olho_y[5] = { 11.0f, 13.0f, 15.0f, 18.0f, 22.0f };
            float olho_y = golem_y + offsets_olho_y[estagio_20c] + ((frame_anim == 1) ? 1.0f : 0.0f);
            float olho1_x = golem_x + 17.0f;
            float olho2_x = golem_x + 25.0f;

            if (anomalia && anomalia->sprite_olho != NULL) {
                al_draw_tinted_bitmap(anomalia->sprite_olho, cor_elem, olho1_x, olho_y, 0);
                al_draw_tinted_bitmap(anomalia->sprite_olho, cor_elem, olho2_x, olho_y, 0);
            } else {
                al_draw_filled_rectangle(olho1_x, olho_y, olho1_x + 2.0f, olho_y + 3.0f, cor_elem);
                al_draw_filled_rectangle(olho2_x, olho_y, olho2_x + 2.0f, olho_y + 3.0f, cor_elem);
            }
        } else {
            al_draw_filled_rectangle(golem_x + 6.0f, golem_y + 6.0f, golem_x + 34.0f, golem_y + 30.0f, cor_elem);
        }
    }

    // Ficha de Propriedades do CSV
    float fx = (px + 8.0f) * 2.0f;
    float fy = (cam_y + cam_h + 8.0f) * 2.0f;
    float flh = 16.0f;

    ALLEGRO_COLOR cor_lbl = al_map_rgb(205, 235, 215);
    al_use_transform(&trans_texto);

    if (sel_descoberto) {
        al_draw_textf(fonte, cor_lbl, fx, fy + (0.0f * flh), 0, "Categoria:      %s", sel.categoria);
        al_draw_textf(fonte, cor_lbl, fx, fy + (1.0f * flh), 0, "Familia:        %s", sel.grupo_familia);

        if (sel.ponto_fusao >= 99999.0f) {
            al_draw_text(fonte, cor_lbl, fx, fy + (2.0f * flh), 0, "P. Fusao:       Indeterminado");
        } else {
            al_draw_textf(fonte, cor_lbl, fx, fy + (2.0f * flh), 0, "P. Fusao:       %.1f C", sel.ponto_fusao);
        }

        if (sel.ponto_ebulicao >= 99999.0f) {
            al_draw_text(fonte, cor_lbl, fx, fy + (3.0f * flh), 0, "P. Ebulicao:    Indeterminado");
        } else {
            al_draw_textf(fonte, cor_lbl, fx, fy + (3.0f * flh), 0, "P. Ebulicao:    %.1f C", sel.ponto_ebulicao);
        }

        al_draw_textf(fonte, cor_lbl, fx, fy + (4.0f * flh), 0, "Estavel Ar:     %s", sel.estavel_argonio);
        al_draw_textf(fonte, cor_lbl, fx, fy + (5.0f * flh), 0, "Reacao Agua:    %s", sel.reacao_agua);
        al_draw_textf(fonte, cor_lbl, fx, fy + (6.0f * flh), 0, "Radioativo:     %s", sel.radioativo ? "Sim" : "Nao");
        al_draw_textf(fonte, cor_lbl, fx, fy + (7.0f * flh), 0, "Magnetico:      %s", sel.magnetico ? "Sim" : "Nao");

        const char *nome_desc = (descobridor_global && descobridor_global[elemento_selecionado][0] != '\0')
                                ? descobridor_global[elemento_selecionado]
                                : "Desconhecido";
        al_draw_textf(fonte, cor_lbl, fx, fy + (8.0f * flh), 0, "Descoberto por: %s", nome_desc);
    } else {
        al_draw_text(fonte, cor_lbl, fx, fy + (0.0f * flh), 0, "Categoria:      Desconhecido");
        al_draw_text(fonte, cor_lbl, fx, fy + (1.0f * flh), 0, "Familia:        Desconhecido");
        al_draw_text(fonte, cor_lbl, fx, fy + (2.0f * flh), 0, "P. Fusao:       Desconhecido");
        al_draw_text(fonte, cor_lbl, fx, fy + (3.0f * flh), 0, "P. Ebulicao:    Desconhecido");
        al_draw_text(fonte, cor_lbl, fx, fy + (4.0f * flh), 0, "Estavel Ar:     Desconhecido");
        al_draw_text(fonte, cor_lbl, fx, fy + (5.0f * flh), 0, "Reacao Agua:    Desconhecido");
        al_draw_text(fonte, cor_lbl, fx, fy + (6.0f * flh), 0, "Radioativo:     Desconhecido");
        al_draw_text(fonte, cor_lbl, fx, fy + (7.0f * flh), 0, "Magnetico:      Desconhecido");
        al_draw_text(fonte, cor_lbl, fx, fy + (8.0f * flh), 0, "Descoberto por: Desconhecido");
    }

    al_use_transform(&trans_ui);
}
