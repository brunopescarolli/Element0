#include "enemy.h"
#include "physics.h"
#include "elementos.h"

void atualizar_inimigo(Inimigo *anomalia, Jogador *faxineiro,
                        Plataforma *plataformas, int total_plat, Escada *escadas, int total_escadas,
                        int sala_atual, int estado_fade, Projetil *projeteis, int max_proj) {

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
                    projeteis[i].ativo = false;

                    if (projeteis[i].tipo == PROJETIL_AGUA) {
                        anomalia->timer_resfriamento = 300;
                    }

                    anomalia->raio_visao = 180.0 * 5.0;
                    anomalia->timer_agro = 300;
                    anomalia->alvo_x = faxineiro->x;
                }
            }
        }
    }

    // Termodinâmica e mudança de estado físico
    DadosElemento dados_anomalia = tabela_periodica[anomalia->numero_atomico];

    if (anomalia->timer_resfriamento > 0) {
        anomalia->timer_resfriamento--;
    }

    if (anomalia->recebendo_calor) {
        anomalia->temperatura += (4.0 / FPS);
    } else {
        float taxa_resfriamento = (2.0 / FPS);
        if (anomalia->timer_resfriamento > 0) {
            taxa_resfriamento *= 2.0;
        }
        anomalia->temperatura -= taxa_resfriamento;
    }

    if (anomalia->temperatura > TEMP_JOGADOR) anomalia->temperatura = TEMP_JOGADOR;
    if (anomalia->temperatura < TEMP_ESTACAO) anomalia->temperatura = TEMP_ESTACAO;

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
