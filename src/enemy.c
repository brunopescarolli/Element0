#include "enemy.h"
#include "physics.h"
#include "elementos.h"

void instanciar_particula(Particula *particulas, int max_particulas,
                          float x, float y, float vx, float vy,
                          float vida_maxima, int tipo, ALLEGRO_COLOR cor, float tamanho) {
    if (!particulas || max_particulas <= 0) return;

    int slot = -1;
    for (int i = 0; i < max_particulas; i++) {
        if (particulas[i].vida <= 0.0f) {
            slot = i;
            break;
        }
    }
    if (slot == -1) {
        return;
    }

    particulas[slot].x = x;
    particulas[slot].y = y;
    particulas[slot].vx = vx;
    particulas[slot].vy = vy;
    particulas[slot].vida = vida_maxima;
    particulas[slot].vida_maxima = (vida_maxima > 0.0f) ? vida_maxima : 1.0f;
    particulas[slot].tipo = tipo;
    particulas[slot].cor = cor;
    particulas[slot].tamanho = tamanho;
}

void instanciar_splash_agua(Particula *particulas, int max_particulas, float x, float y, float vel_x_tiro) {
    if (!particulas || max_particulas <= 0) return;

    int qtd_splash = 5 + (rand() % 4); // 5 a 8 partículas
    float dir_rebote = (vel_x_tiro >= 0.0f) ? -1.0f : 1.0f;

    for (int i = 0; i < qtd_splash; i++) {
        float vx = dir_rebote * (0.5f + ((float)(rand() % 100) / 100.0f) * 1.6f);
        float vy = (((float)(rand() % 200) / 100.0f) - 1.15f) * 1.3f;
        float vida = 12.0f + (float)(rand() % 10);
        float tamanho = 1.5f + ((float)(rand() % 10) / 10.0f) * 1.0f;
        ALLEGRO_COLOR cor_splash = al_map_rgb(0, 150, 255);
        instanciar_particula(particulas, max_particulas, x, y, vx, vy, vida, PARTICULA_GOTA, cor_splash, tamanho);
    }
}

void atualizar_particulas(Particula *particulas, int max_particulas) {
    if (!particulas) return;

    for (int i = 0; i < max_particulas; i++) {
        if (particulas[i].vida <= 0.0f) continue;

        particulas[i].x += particulas[i].vx;
        particulas[i].y += particulas[i].vy;

        if (particulas[i].tipo == PARTICULA_GOTA) {
            particulas[i].vy += 0.07f;
        } else if (particulas[i].tipo == PARTICULA_VAPOR) {
            particulas[i].tamanho += 0.06f;
        } else if (particulas[i].tipo == PARTICULA_CALOR) {
            float progresso = 1.0f - (particulas[i].vida / particulas[i].vida_maxima);
            if (progresso < 0.0f) progresso = 0.0f;
            if (progresso > 1.0f) progresso = 1.0f;
            particulas[i].tamanho = 4.0f + (4.0f * progresso);
        }

        particulas[i].vida -= 1.0f;
        if (particulas[i].vida < 0.0f) {
            particulas[i].vida = 0.0f;
        }
    }
}

void atualizar_inimigo(Inimigo *anomalia, Jogador *faxineiro,
                        Plataforma *plataformas, int total_plat, Escada *escadas, int total_escadas,
                        int sala_atual, int estado_fade, Projetil *projeteis, int max_proj,
                        Particula *particulas, int max_particulas) {

    float limite_anomalia = (anomalia->sala_origem == 0) ? (LARGURA_MAPA / 2.0) : LARGURA_MAPA;

    bool anomalia_quer_descer = false;
    if (anomalia->timer_agro > 0 && faxineiro->y > anomalia->y + 15.0) {
        anomalia_quer_descer = true;
    }

    // Visão e Agro
    if (sala_atual == anomalia->sala_origem && estado_fade == 0) {
        float abs_dist_x = fabs(faxineiro->x - anomalia->x);
        float player_top = (faxineiro->y + faxineiro->h / 2.0) - faxineiro->body_h;
        float player_bottom = faxineiro->y + faxineiro->h / 2.0;
        float anomalia_top = anomalia->y - anomalia->h / 2.0;
        float anomalia_bottom = anomalia->y + anomalia->h / 2.0;

        bool toque_vertical = (player_bottom >= anomalia_top) && (player_top <= anomalia_bottom);

        if (abs_dist_x <= anomalia->raio_visao && toque_vertical) {
            anomalia->timer_agro = 300;
            anomalia->alvo_x = faxineiro->x;
        }
    }

    // Movimentação por IA
    if (anomalia->timer_agro > 0) {
        anomalia->timer_agro--;
        if (anomalia->x < anomalia->alvo_x - anomalia->velocidade) {
            anomalia->vel_x_atual = anomalia->velocidade;
            anomalia->direcao_atual = DIR_DIREITA;
        } else if (anomalia->x > anomalia->alvo_x + anomalia->velocidade) {
            anomalia->vel_x_atual = -anomalia->velocidade;
            anomalia->direcao_atual = DIR_ESQUERDA;
        } else {
            anomalia->vel_x_atual = 0;
        }

        if (anomalia->timer_agro <= 0) {
            anomalia->raio_visao = 180.0;
        }
    } else {
        if (anomalia->timer_patrulha <= 0) {
            anomalia->timer_patrulha = 60 + (rand() % 120);
            int acao = rand() % 3;
            if (acao == 0) {
                anomalia->vel_x_atual = 0;
            } else if (acao == 1) {
                anomalia->vel_x_atual = -(anomalia->velocidade * 0.5);
                anomalia->direcao_atual = DIR_ESQUERDA;
            } else {
                anomalia->vel_x_atual = (anomalia->velocidade * 0.5);
                anomalia->direcao_atual = DIR_DIREITA;
            }
        } else {
            anomalia->timer_patrulha--;
        }
    }

    anomalia->vel_y += GRAVIDADE;
    anomalia->y += anomalia->vel_y;
    anomalia->no_chao = false;

    // Colisões verticais com plataformas e escadas
    resolve_platform_collisions(anomalia->x, &anomalia->y, anomalia->w, anomalia->h, &anomalia->vel_y, &anomalia->no_chao, anomalia->sala_origem, plataformas, total_plat, anomalia_quer_descer);
    resolve_ladder_collisions(anomalia->x, &anomalia->y, anomalia->w, anomalia->h, &anomalia->vel_y, &anomalia->no_chao, anomalia->vel_x_atual, anomalia->sala_origem, escadas, total_escadas, anomalia_quer_descer);

    // Detecção de paredes e bordas
    if (anomalia->vel_x_atual != 0) {
        float futuro_x = anomalia->x + anomalia->vel_x_atual;
        bool bateu_parede = (futuro_x - anomalia->w / 2 < 0 || futuro_x + anomalia->w / 2 > limite_anomalia);

        for (int e = 0; e < total_escadas; e++) {
            if (escadas[e].sala != anomalia->sala_origem) continue;
            Escada esc = escadas[e];
            float pe_anomalia = anomalia->y + anomalia->h / 2.0;

            if (pe_anomalia > esc.y + 4.0 && pe_anomalia - anomalia->h < esc.y + esc.h) {
                if (esc.direcao == 1 && anomalia->vel_x_atual < 0 && anomalia->x - anomalia->w / 2.0 >= esc.x + esc.w - 5.0 && futuro_x - anomalia->w / 2.0 < esc.x + esc.w) {
                    bateu_parede = true;
                } else if (esc.direcao == -1 && anomalia->vel_x_atual > 0 && anomalia->x + anomalia->w / 2.0 <= esc.x + 5.0 && futuro_x + anomalia->w / 2.0 > esc.x) {
                    bateu_parede = true;
                }
            }
        }

        bool chao_frente = false;
        if (anomalia->no_chao) {
            float borda_x = anomalia->x + (anomalia->vel_x_atual > 0 ? anomalia->w / 2 : -anomalia->w / 2) + anomalia->vel_x_atual;
            float base_y = anomalia->y + anomalia->h / 2.0;

            for (int p = 0; p < total_plat; p++) {
                if (plataformas[p].sala != anomalia->sala_origem) continue;
                Plataforma plat = plataformas[p];
                if (borda_x >= plat.x && borda_x <= plat.x + plat.w) {
                    if (fabs(base_y - plat.y) <= 45.0) {
                        chao_frente = true; break;
                    }
                }
            }

            if (!chao_frente) {
                for (int e = 0; e < total_escadas; e++) {
                    if (escadas[e].sala != anomalia->sala_origem) continue;
                    Escada esc = escadas[e];
                    if (esc.direcao == 0) continue;
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

        if (bateu_parede || (anomalia->no_chao && !chao_frente && !anomalia_quer_descer)) {
            if (anomalia->timer_agro <= 0) {
                anomalia->vel_x_atual = -anomalia->vel_x_atual;
                anomalia->direcao_atual = (anomalia->vel_x_atual > 0) ? DIR_DIREITA : DIR_ESQUERDA;
                anomalia->timer_patrulha = 60;
            } else {
                anomalia->vel_x_atual = 0;
            }
        } else {
            anomalia->x = futuro_x;
        }
    }

    // Colisão com Projéteis
    for (int i = 0; i < max_proj; i++) {
        if (projeteis[i].ativo) {
            if (sala_atual == anomalia->sala_origem) {
                bool acertou_monstro_x = (projeteis[i].x > anomalia->x - anomalia->w / 2) && (projeteis[i].x < anomalia->x + anomalia->w / 2);
                bool acertou_monstro_y = (projeteis[i].y > anomalia->y - anomalia->h / 2) && (projeteis[i].y < anomalia->y + anomalia->h / 2);

                if (acertou_monstro_x && acertou_monstro_y) {
                    if (projeteis[i].tipo == PROJETIL_AGUA) {
                        instanciar_splash_agua(particulas, max_particulas, projeteis[i].x, projeteis[i].y, projeteis[i].vel_x);
                        anomalia->timer_resfriamento = 300;
                        anomalia->agua_atual += 25.0f;
                        if (anomalia->agua_atual > 100.0f) {
                            anomalia->agua_atual = 100.0f;
                        }
                    }
                    projeteis[i].ativo = false;

                    anomalia->raio_visao = 180.0 * 5.0;
                    anomalia->timer_agro = 300;
                    anomalia->alvo_x = faxineiro->x;
                }
            }
        }
    }

    // Termodinâmica, choque térmico e evaporação
    DadosElemento dados_anomalia = tabela_periodica[anomalia->numero_atomico];

    if (anomalia->timer_resfriamento > 0) {
        anomalia->timer_resfriamento--;
    }

    if (anomalia->recebendo_calor_soprador) {
        anomalia->temperatura += (100.0f / (float)FPS);
        if (anomalia->temperatura > 1500.0f) {
            anomalia->temperatura = 1500.0f;
        }
    } else if (anomalia->recebendo_calor_jogador && anomalia->temperatura < TEMP_JOGADOR) {
        anomalia->temperatura += (4.0f / (float)FPS);
        if (anomalia->temperatura > TEMP_JOGADOR) {
            anomalia->temperatura = TEMP_JOGADOR;
        }
    } else {
        float multiplicador_agua = (anomalia->agua_atual > 0.0f) ? 4.0f : 1.0f;
        float reducao_temperatura = multiplicador_agua * (2.0f * anomalia->temperatura / 100.0f);
        anomalia->temperatura -= (reducao_temperatura / 60.0f);

        if (anomalia->recebendo_calor_jogador && anomalia->temperatura < TEMP_JOGADOR) {
            anomalia->temperatura = TEMP_JOGADOR;
        }
    }

    if (anomalia->agua_atual > 0.0f) {
        float evaporacao_por_segundo = (anomalia->temperatura / 10.0f);

        if (sala_atual == anomalia->sala_origem && particulas != NULL) {
            // 3. Feedback Visual do Estado "Molhado" (Gotas Azuis: 1 ou 2 por frame)
            int qtd_gotas = 1 + (rand() % 2);
            for (int g = 0; g < qtd_gotas; g++) {
                float rx = ((float)(rand() % 1000) / 1000.0f) - 0.5f;
                float ry = ((float)(rand() % 1000) / 1000.0f) - 0.5f;
                float px = anomalia->x + rx * (anomalia->w * 0.85f);
                float py = anomalia->y + ry * (anomalia->h * 0.85f);
                float vx = (((float)(rand() % 100) / 100.0f) - 0.5f) * 0.12f;
                float vy = 0.45f + ((float)(rand() % 100) / 100.0f) * 0.45f;
                float vida_gota = 14.0f + (float)(rand() % 10);
                float tam_gota = 1.2f + ((float)(rand() % 10) / 10.0f) * 0.8f;
                ALLEGRO_COLOR cor_gota = al_map_rgb(60, 175, 255);
                instanciar_particula(particulas, max_particulas, px, py, vx, vy, vida_gota, PARTICULA_GOTA, cor_gota, tam_gota);
            }

            // 4. Feedback Visual de Evaporação (Vapor Branco/Cinza apenas quando temperatura > 100.0 °C)
            if (anomalia->temperatura > 100.0f) {
                float taxa_vapor = evaporacao_por_segundo * 0.06f;
                int qtd_vapor = (int)taxa_vapor;
                float chance_extra = taxa_vapor - (float)qtd_vapor;
                if (((float)(rand() % 1000) / 1000.0f) < chance_extra) {
                    qtd_vapor++;
                }

                for (int v = 0; v < qtd_vapor; v++) {
                    float rx = ((float)(rand() % 1000) / 1000.0f) - 0.5f;
                    float ry = ((float)(rand() % 1000) / 1000.0f) - 0.5f;
                    float px = anomalia->x + rx * (anomalia->w * 0.90f);
                    float py = anomalia->y + ry * (anomalia->h * 0.70f);
                    float vx = (((float)(rand() % 100) / 100.0f) - 0.5f) * 0.55f;
                    float vy = -(0.50f + ((float)(rand() % 100) / 100.0f) * 0.85f);
                    float vida_vapor = 28.0f + (float)(rand() % 24);
                    float tam_vapor = 2.0f + ((float)(rand() % 15) / 10.0f);
                    unsigned char tom = (unsigned char)(210 + (rand() % 46));
                    ALLEGRO_COLOR cor_vapor = al_map_rgb(tom, tom, tom);
                    instanciar_particula(particulas, max_particulas, px, py, vx, vy, vida_vapor, PARTICULA_VAPOR, cor_vapor, tam_vapor);
                }
            }
        }

        anomalia->agua_atual -= (evaporacao_por_segundo / 60.0f);
        if (anomalia->agua_atual <= 0.0f) {
            anomalia->agua_atual = 0.0f;
        }
    }

    if (anomalia->temperatura < 20.0f) anomalia->temperatura = 20.0f;
    if (anomalia->temperatura > 1500.0f) anomalia->temperatura = 1500.0f;

    if (anomalia->temperatura >= dados_anomalia.ponto_fusao && anomalia->estado_atual == SOLIDO) {
        anomalia->estado_atual = LIQUIDO;
    } else if (anomalia->temperatura < dados_anomalia.ponto_fusao && anomalia->estado_atual == LIQUIDO) {
        anomalia->estado_atual = SOLIDO;
    }

    if (anomalia->estado_atual == LIQUIDO) {
        anomalia->frame_atual = 4;
    } else {
        float range = dados_anomalia.ponto_fusao - TEMP_ESTACAO;
        if (range <= 0) range = 1.0;
        anomalia->frame_atual = (int)(((anomalia->temperatura - TEMP_ESTACAO) / range) * 4.0);

        if (anomalia->frame_atual > 3) anomalia->frame_atual = 3;
        if (anomalia->frame_atual < 0) anomalia->frame_atual = 0;
    }

    anomalia->tempo_animacao++;
    if (anomalia->tempo_animacao >= 30) {
        anomalia->tempo_animacao = 0;
        anomalia->frame_animacao = 1 - anomalia->frame_animacao;
    }
}
