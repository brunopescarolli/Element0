#include "player.h"
#include "physics.h"
#include "weapon.h"
#include "elementos.h"

void atualizar_jogador(Jogador *faxineiro, Input *input, Item *arma_mop, Inimigo *anomalia,
                       Plataforma *plataformas, int total_plat, Escada *escadas, int total_escadas,
                       Porta *portas, int total_portas, Projetil *projeteis, int max_proj,
                       int *sala_atual, int *estado_fade, int *proxima_sala, float *proximo_x, float *proximo_chao_y) {

    if (*estado_fade != 0) return;

    // Proximidade e animação das portas
    for (int i = 0; i < total_portas; i++) {
        if (portas[i].sala_origem != *sala_atual) continue;

        bool prox_x = (faxineiro->x + faxineiro->w / 2 > portas[i].x - 15) &&
                      (faxineiro->x - faxineiro->w / 2 < portas[i].x + portas[i].w + 15);

        float chao_porta = portas[i].y + portas[i].h;
        float pe_jogador = faxineiro->y + faxineiro->h / 2.0;
        bool mesmo_nivel = (fabs(pe_jogador - chao_porta) <= 5.0);

        if (prox_x && mesmo_nivel) {
            portas[i].jogador_perto = true;
            if (portas[i].timer_animacao < 30) portas[i].timer_animacao++;
        } else {
            portas[i].jogador_perto = false;
            if (portas[i].timer_animacao > 0) portas[i].timer_animacao--;
        }
    }

    // Interação de entrada nas portas
    if (input->acao_entrar) {
        for (int i = 0; i < total_portas; i++) {
            Porta p = portas[i];
            if (p.sala_origem != *sala_atual) continue;

            float chao_porta = p.y + p.h;
            float pe_jogador = faxineiro->y + faxineiro->h / 2.0;

            if (faxineiro->x > p.x && faxineiro->x < p.x + p.w && fabs(pe_jogador - chao_porta) <= 5.0) {
                *estado_fade = 1;
                *proxima_sala = p.destino_sala;

                for (int j = 0; j < total_portas; j++) {
                    if (portas[j].sala_origem == p.destino_sala && portas[j].destino_sala == *sala_atual) {
                        *proximo_x = portas[j].x + (portas[j].w / 2.0);
                        *proximo_chao_y = portas[j].y + portas[j].h;
                        break;
                    }
                }

                input->acao_entrar = false;
                if (*sala_atual == anomalia->sala_origem) {
                    float abs_dist_x = fabs(faxineiro->x - anomalia->x);
                    float player_top = (faxineiro->y + faxineiro->h / 2.0) - faxineiro->body_h;
                    float player_bottom = faxineiro->y + faxineiro->h / 2.0;
                    float anomalia_top = anomalia->y - anomalia->h / 2.0;
                    float anomalia_bottom = anomalia->y + anomalia->h / 2.0;

                    bool toque_vertical = (player_bottom >= anomalia_top) && (player_top <= anomalia_bottom);

                    if (abs_dist_x <= anomalia->raio_visao && toque_vertical) {
                        anomalia->timer_agro = 300;
                        anomalia->alvo_x = p.x + p.w / 2;
                    }
                }
                break;
            }
        }
    }

    bool preso_na_poca = false;
    if (*sala_atual == anomalia->sala_origem && anomalia->estado_atual == LIQUIDO) {
        bool col_poca_x = (faxineiro->x + faxineiro->w / 2 > anomalia->x - anomalia->w / 2) && (faxineiro->x - faxineiro->w / 2 < anomalia->x + anomalia->w / 2);
        bool col_poca_y = (faxineiro->y + faxineiro->h / 2 > anomalia->y - anomalia->h / 2) && (faxineiro->y - faxineiro->h / 2 < anomalia->y + anomalia->h / 2);
        if (col_poca_x && col_poca_y) preso_na_poca = true;
    }

    float velocidade_atual = preso_na_poca ? (faxineiro->velocidade * 0.4) : faxineiro->velocidade;
    float vel_x_aplicada = input->eixo_x * velocidade_atual;
    float futuro_x = faxineiro->x + vel_x_aplicada;

    // Colisão horizontal com escadas/rampas
    for (int e = 0; e < total_escadas; e++) {
        if (escadas[e].sala != *sala_atual) continue;
        Escada esc = escadas[e];
        float pe_jogador = faxineiro->y + faxineiro->h / 2.0;

        if (pe_jogador > esc.y + 4.0 && pe_jogador - faxineiro->h < esc.y + esc.h) {
            if (esc.direcao == 1) {
                if (vel_x_aplicada < 0 && faxineiro->x - faxineiro->w / 2.0 >= esc.x + esc.w - 5.0 && futuro_x - faxineiro->w / 2.0 < esc.x + esc.w) {
                    futuro_x = esc.x + esc.w + faxineiro->w / 2.0;
                }
            } else if (esc.direcao == -1) {
                if (vel_x_aplicada > 0 && faxineiro->x + faxineiro->w / 2.0 <= esc.x + 5.0 && futuro_x + faxineiro->w / 2.0 > esc.x) {
                    futuro_x = esc.x - faxineiro->w / 2.0;
                }
            }
        }
    }

    float limite_mapa_x = (*sala_atual == 0) ? (LARGURA_MAPA / 2.0) : LARGURA_MAPA;
    faxineiro->x = futuro_x;
    if (input->eixo_x > 0) faxineiro->direcao_atual = DIR_DIREITA; else if (input->eixo_x < 0) faxineiro->direcao_atual = DIR_ESQUERDA;

    if (faxineiro->x - faxineiro->body_w / 2 < 0) faxineiro->x = faxineiro->body_w / 2;
    if (faxineiro->x + faxineiro->body_w / 2 > limite_mapa_x) faxineiro->x = limite_mapa_x - faxineiro->body_w / 2;

    // Gravidade e movimento vertical
    faxineiro->vel_y += GRAVIDADE;
    faxineiro->y += faxineiro->vel_y;
    faxineiro->no_chao = false;

    // Resolução de colisões com plataformas e escadas
    resolve_platform_collisions(faxineiro->x, &faxineiro->y, faxineiro->w, faxineiro->h, &faxineiro->vel_y, &faxineiro->no_chao, *sala_atual, plataformas, total_plat, input->acao_descer);
    resolve_ladder_collisions(faxineiro->x, &faxineiro->y, faxineiro->w, faxineiro->h, &faxineiro->vel_y, &faxineiro->no_chao, input->eixo_x, *sala_atual, escadas, total_escadas, input->acao_descer);

    // Colisão com Inimigo (Anomalia de Gálio)
    if (*sala_atual == anomalia->sala_origem) {
        bool colide_inimigo_x = (faxineiro->x + faxineiro->w / 2 > anomalia->x - anomalia->w / 2) && (faxineiro->x - faxineiro->w / 2 < anomalia->x + anomalia->w / 2);
        bool colide_inimigo_y = (faxineiro->y + faxineiro->h / 2 > anomalia->y - anomalia->h / 2) && (faxineiro->y - faxineiro->h / 2 < anomalia->y + anomalia->h / 2);

        if (colide_inimigo_x && colide_inimigo_y) {
            anomalia->recebendo_calor = true;
            float pe_anterior = (faxineiro->y - faxineiro->vel_y) + faxineiro->h / 2;
            float topo_inimigo = anomalia->y - anomalia->h / 2;
            if (faxineiro->vel_y >= 0 && pe_anterior <= topo_inimigo + 0.1 && anomalia->estado_atual == SOLIDO) {
                faxineiro->y = topo_inimigo - faxineiro->h / 2; faxineiro->vel_y = 0; faxineiro->no_chao = true;
            }
        }
    }

    // Pulo
    if (input->pulo && faxineiro->no_chao) {
        faxineiro->vel_y = preso_na_poca ? (FORCA_PULO * 0.7) : FORCA_PULO;
        faxineiro->no_chao = false;
    }

    // Animação
    bool movendo_agora = (input->eixo_x != 0);
    if (movendo_agora != faxineiro->esta_andando) {
        faxineiro->esta_andando = movendo_agora;
        faxineiro->frame_atual = 0;
        faxineiro->tempo_animacao = 0;
    }
    faxineiro->tempo_animacao++;
    int limite_tempo = faxineiro->esta_andando ? 10 : 30;
    if (faxineiro->tempo_animacao >= limite_tempo) {
        faxineiro->tempo_animacao = 0; faxineiro->frame_atual++;
        if (faxineiro->esta_andando) { if (faxineiro->frame_atual > 3) faxineiro->frame_atual = 0; }
        else { if (faxineiro->frame_atual > 1) faxineiro->frame_atual = 0; }
    }

    // Item (Mop)
    if (arma_mop->no_chao && arma_mop->sala_origem == *sala_atual) {
        float dx_item = faxineiro->x - arma_mop->x; float dy_item = faxineiro->y - arma_mop->y;
        if (input->acao_pegar && !faxineiro->possui_arma) {
            if ((dx_item * dx_item) + (dy_item * dy_item) <= (arma_mop->raio_interacao * arma_mop->raio_interacao)) {
                arma_mop->no_chao = false; faxineiro->possui_arma = true;
            }
        }
    }
    if (input->acao_soltar && faxineiro->possui_arma && !arma_mop->no_chao) {
        arma_mop->no_chao = true; arma_mop->x = faxineiro->x; arma_mop->y = faxineiro->y; arma_mop->vel_y = 0.0; arma_mop->sala_origem = *sala_atual; faxineiro->possui_arma = false;
    }

    if (faxineiro->cooldown > 0) faxineiro->cooldown--;

    // Disparo e atualização de projéteis
    if (input->atirando) {
        disparar_arma_jogador(faxineiro, projeteis, max_proj);
    }
    atualizar_projeteis(projeteis, max_proj, *sala_atual);
}
