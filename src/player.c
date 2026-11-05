#include "player.h"
#include "physics.h"
#include "weapon.h"
#include "elementos.h"

void atualizar_jogador(Jogador *faxineiro, Input *input, Item *arma_mop, Item *arma_soprador,
                       Soprador *soprador, Coletavel *refil_agua, Coletavel *bateria, Coletavel *kit_medico,
                       Inimigo *anomalia, Plataforma *plataformas, int total_plat, Escada *escadas, int total_escadas,
                       Porta *portas, int total_portas, Projetil *projeteis, int max_proj,
                       TextoFlutuante *textos, int max_textos,
                       int *sala_atual, int *estado_fade, int *proxima_sala, float *proximo_x, float *proximo_chao_y) {

    if (*estado_fade != 0) return;

    if (faxineiro->morreu) {
        input->eixo_x = 0.0f;
        input->pulo = false;
        input->atirando = false;
        input->acao_pegar = false;
        input->acao_soltar = false;
        input->acao_entrar = false;
        input->acao_descer = false;
        faxineiro->escalando = false;
        faxineiro->esta_andando = false;
        faxineiro->frame_atual = 0;
        soprador->ativo = false;

        faxineiro->vel_y += GRAVIDADE;
        faxineiro->y += faxineiro->vel_y;
        faxineiro->no_chao = false;
        resolve_platform_collisions(faxineiro->x, &faxineiro->y, faxineiro->w, faxineiro->h, &faxineiro->vel_y, &faxineiro->no_chao, *sala_atual, plataformas, total_plat, false);
        resolve_ladder_collisions(faxineiro->x, &faxineiro->y, faxineiro->w, faxineiro->h, &faxineiro->vel_y, &faxineiro->no_chao, 0.0f, *sala_atual, escadas, total_escadas, false);
        if (faxineiro->y + faxineiro->h / 2.0 > ALTURA_MAPA) {
            faxineiro->y = ALTURA_MAPA - (faxineiro->h / 2.0);
            faxineiro->vel_y = 0;
            faxineiro->no_chao = true;
        }
        atualizar_projeteis(projeteis, max_proj, *sala_atual);
        return;
    }

    // Verificar contato com escada vertical estilo marinheiro (direcao == 0)
    int indice_escada_vertical = -1;
    for (int e = 0; e < total_escadas; e++) {
        if (escadas[e].sala != *sala_atual || escadas[e].direcao != 0) continue;
        Escada esc = escadas[e];
        float player_left = faxineiro->x - faxineiro->body_w / 2.0;
        float player_right = faxineiro->x + faxineiro->body_w / 2.0;
        float player_top = (faxineiro->y + faxineiro->h / 2.0) - faxineiro->body_h;
        float player_bottom = faxineiro->y + faxineiro->h / 2.0;

        if (player_right > esc.x && player_left < esc.x + esc.w &&
            player_bottom > esc.y && player_top < esc.y + esc.h) {
            indice_escada_vertical = e;
            break;
        }
    }

    if (indice_escada_vertical == -1) {
        faxineiro->escalando = false;
    } else if (!faxineiro->escalando && (input->acao_entrar || input->acao_descer)) {
        faxineiro->escalando = true;
        faxineiro->vel_y = 0;
        faxineiro->no_chao = false;
    }

    // Proximidade e animação das portas
    for (int i = 0; i < total_portas; i++) {
        if (portas[i].sala_origem != *sala_atual || portas[i].tipo == 2) continue;

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

    // Interação de entrada nas portas normais (desativada durante escalada ou sobre a escada vertical)
    if (input->acao_entrar && !faxineiro->escalando && indice_escada_vertical == -1) {
        for (int i = 0; i < total_portas; i++) {
            Porta p = portas[i];
            if (p.sala_origem != *sala_atual || p.tipo == 2) continue;

            float chao_porta = p.y + p.h;
            float pe_jogador = faxineiro->y + faxineiro->h / 2.0;

            if (faxineiro->x > p.x && faxineiro->x < p.x + p.w && fabs(pe_jogador - chao_porta) <= 5.0) {
                *estado_fade = 1;
                *proxima_sala = p.destino_sala;

                for (int j = 0; j < total_portas; j++) {
                    if (portas[j].sala_origem == p.destino_sala && portas[j].destino_sala == *sala_atual) {
                        *proximo_x = portas[j].x + (portas[j].w / 2.0);
                        *proximo_chao_y = (portas[j].tipo == 2) ? 180.0 : (portas[j].y + portas[j].h);
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
    float limite_mapa_x = (*sala_atual == 0) ? (LARGURA_MAPA / 2.0) : LARGURA_MAPA;

    if (faxineiro->escalando && indice_escada_vertical != -1) {
        Escada esc = escadas[indice_escada_vertical];

        if (input->eixo_x == 0) {
            faxineiro->x = esc.x + (esc.w / 2.0);
        } else {
            faxineiro->x = futuro_x;
            if (input->eixo_x > 0) faxineiro->direcao_atual = DIR_DIREITA;
            else if (input->eixo_x < 0) faxineiro->direcao_atual = DIR_ESQUERDA;

            if (!input->acao_entrar && !input->acao_descer) {
                float pe_jogador = faxineiro->y + faxineiro->h / 2.0;
                for (int p = 0; p < total_plat; p++) {
                    if (plataformas[p].sala != *sala_atual) continue;
                    if (fabs(pe_jogador - plataformas[p].y) <= 10.0) {
                        faxineiro->y = plataformas[p].y - (faxineiro->h / 2.0);
                        break;
                    }
                }
            }

            if (faxineiro->x - faxineiro->body_w / 2.0 < 0) faxineiro->x = faxineiro->body_w / 2.0;
            if (faxineiro->x + faxineiro->body_w / 2.0 > limite_mapa_x) faxineiro->x = limite_mapa_x - faxineiro->body_w / 2.0;

            float player_left = faxineiro->x - faxineiro->body_w / 2.0;
            float player_right = faxineiro->x + faxineiro->body_w / 2.0;
            if (player_right <= esc.x || player_left >= esc.x + esc.w) {
                faxineiro->escalando = false;
            }
        }

        if (faxineiro->escalando) {
            faxineiro->vel_y = 0;
            faxineiro->no_chao = false;

            if (input->acao_entrar) {
                faxineiro->y -= velocidade_atual;
            } else if (input->acao_descer) {
                faxineiro->y += velocidade_atual;
            }

            float min_y = esc.y + faxineiro->body_h - (faxineiro->h / 2.0);
            float max_y = esc.y + esc.h - (faxineiro->h / 2.0);
            if (faxineiro->y < min_y) faxineiro->y = min_y;
            if (faxineiro->y > max_y) faxineiro->y = max_y;

            // Transição entre andares pelas caixas invisíveis (tipo == 2) no topo e base da escada
            float player_left = faxineiro->x - faxineiro->body_w / 2.0;
            float player_right = faxineiro->x + faxineiro->body_w / 2.0;
            float player_top = (faxineiro->y + faxineiro->h / 2.0) - faxineiro->body_h;
            float player_bottom = faxineiro->y + faxineiro->h / 2.0;

            for (int i = 0; i < total_portas; i++) {
                Porta p = portas[i];
                if (p.sala_origem != *sala_atual || p.tipo != 2) continue;

                bool colide_caixa = (player_right > p.x) && (player_left < p.x + p.w) &&
                                    (player_bottom > p.y) && (player_top < p.y + p.h);
                bool caixa_topo = (p.y < ALTURA_MAPA / 2.0);

                if (colide_caixa && ((caixa_topo && input->acao_entrar) || (!caixa_topo && input->acao_descer))) {
                    *estado_fade = 1;
                    *proxima_sala = p.destino_sala;
                    *proximo_x = p.x + (p.w / 2.0);
                    *proximo_chao_y = 180.0;

                    for (int j = 0; j < total_portas; j++) {
                        if (portas[j].sala_origem == p.destino_sala && portas[j].destino_sala == *sala_atual) {
                            *proximo_x = portas[j].x + (portas[j].w / 2.0);
                            *proximo_chao_y = (portas[j].tipo == 2) ? 180.0 : (portas[j].y + portas[j].h);
                            break;
                        }
                    }
                    break;
                }
            }
        }
    }

    if (!faxineiro->escalando) {
        // Colisão horizontal com escadas/rampas
        for (int e = 0; e < total_escadas; e++) {
            if (escadas[e].sala != *sala_atual || escadas[e].direcao == 0) continue;
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

        if (faxineiro->y + faxineiro->h / 2.0 > ALTURA_MAPA) {
            faxineiro->y = ALTURA_MAPA - (faxineiro->h / 2.0);
            faxineiro->vel_y = 0;
            faxineiro->no_chao = true;
        }
    }

    // Colisão com Inimigo (Anomalia de Gálio)
    if (*sala_atual == anomalia->sala_origem) {
        float p_left = faxineiro->x - faxineiro->body_w / 2.0f;
        float p_right = faxineiro->x + faxineiro->body_w / 2.0f;
        float p_top = (faxineiro->y + faxineiro->h / 2.0f) - faxineiro->body_h;
        float p_bottom = faxineiro->y + faxineiro->h / 2.0f;

        bool colide_corpo_x = (p_right > anomalia->x - anomalia->w / 2.0f) && (p_left < anomalia->x + anomalia->w / 2.0f);
        bool colide_corpo_y = (p_bottom > anomalia->y - anomalia->h / 2.0f) && (p_top < anomalia->y + anomalia->h / 2.0f);

        bool colide_inimigo_x = (faxineiro->x + faxineiro->w / 2 > anomalia->x - anomalia->w / 2) && (faxineiro->x - faxineiro->w / 2 < anomalia->x + anomalia->w / 2);
        bool colide_inimigo_y = (faxineiro->y + faxineiro->h / 2 > anomalia->y - anomalia->h / 2) && (faxineiro->y - faxineiro->h / 2 < anomalia->y + anomalia->h / 2);

        if ((colide_inimigo_x && colide_inimigo_y) || (colide_corpo_x && colide_corpo_y)) {
            anomalia->recebendo_calor = true;
            anomalia->recebendo_calor_jogador = true;

            // Dano térmico por contato no Ponto de Draper (>= 525.0 °C): 10.0 pontos por segundo
            if (anomalia->temperatura >= PONTO_DRAPER) {
                faxineiro->barra_vida -= (10.0f / (float)FPS);
                if (faxineiro->barra_vida < 0.0f) {
                    faxineiro->barra_vida = 0.0f;
                }
            }
        }

        if (colide_inimigo_x && colide_inimigo_y) {
            float pe_anterior = (faxineiro->y - faxineiro->vel_y) + faxineiro->h / 2;
            float topo_inimigo = anomalia->y - anomalia->h / 2;
            if (!faxineiro->escalando && faxineiro->vel_y >= 0 && pe_anterior <= topo_inimigo + 0.1 && anomalia->estado_atual == SOLIDO) {
                faxineiro->y = topo_inimigo - faxineiro->h / 2; faxineiro->vel_y = 0; faxineiro->no_chao = true;
            }
        }
    }

    // Pulo (desativado enquanto estiver escalando)
    if (!faxineiro->escalando && input->pulo && faxineiro->no_chao) {
        faxineiro->vel_y = preso_na_poca ? (FORCA_PULO * 0.7) : FORCA_PULO;
        faxineiro->no_chao = false;
    }

    // Animação
    bool movendo_agora = (input->eixo_x != 0) || (faxineiro->escalando && (input->acao_entrar || input->acao_descer));
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

    // Itens (Mop e Soprador) - Pegar
    if (input->acao_pegar) {
        float player_top_item = (faxineiro->y + faxineiro->h / 2.0f) - faxineiro->body_h;
        bool mop_alcance = false;
        float dist_sq_mop = 999999.0f;
        if (arma_mop->no_chao && arma_mop->sala_origem == *sala_atual && !faxineiro->possui_mop) {
            float dx_item = faxineiro->x - arma_mop->x;
            float dy_item = faxineiro->y - arma_mop->y;
            dist_sq_mop = (dx_item * dx_item) + (dy_item * dy_item);
            if (dist_sq_mop <= (arma_mop->raio_interacao * arma_mop->raio_interacao)) {
                mop_alcance = true;
            }
        }

        bool sop_alcance = false;
        float dist_sq_sop = 999999.0f;
        if (arma_soprador->no_chao && arma_soprador->sala_origem == *sala_atual && !faxineiro->possui_soprador) {
            float dx_sop = faxineiro->x - arma_soprador->x;
            float dy_sop = faxineiro->y - arma_soprador->y;
            dist_sq_sop = (dx_sop * dx_sop) + (dy_sop * dy_sop);
            if (dist_sq_sop <= (arma_soprador->raio_interacao * arma_soprador->raio_interacao)) {
                sop_alcance = true;
            }
        }

        // Pega com sucesso O PRIMEIRO item detectado na colisão (o mais próximo); para o segundo aperta E de novo
        if (mop_alcance && (!sop_alcance || dist_sq_mop <= dist_sq_sop)) {
            arma_mop->no_chao = false;
            faxineiro->possui_mop = true;
            if (faxineiro->arma_slot_1 == ARMA_NENHUMA) {
                faxineiro->arma_slot_1 = ARMA_MOP;
            } else if (faxineiro->arma_slot_2 == ARMA_NENHUMA) {
                faxineiro->arma_slot_2 = ARMA_MOP;
            }
            if (faxineiro->arma_equipada == ARMA_NENHUMA) {
                faxineiro->arma_equipada = ARMA_MOP;
            }
            instanciar_texto_flutuante(textos, max_textos, faxineiro->x, player_top_item - 10.0f, "* mop *");
            input->acao_pegar = false;
        } else if (sop_alcance) {
            arma_soprador->no_chao = false;
            faxineiro->possui_soprador = true;
            if (faxineiro->arma_slot_1 == ARMA_NENHUMA) {
                faxineiro->arma_slot_1 = ARMA_SOPRADOR;
            } else if (faxineiro->arma_slot_2 == ARMA_NENHUMA) {
                faxineiro->arma_slot_2 = ARMA_SOPRADOR;
            }
            if (faxineiro->arma_equipada == ARMA_NENHUMA) {
                faxineiro->arma_equipada = ARMA_SOPRADOR;
            }
            instanciar_texto_flutuante(textos, max_textos, faxineiro->x, player_top_item - 10.0f, "* soprador termico *");
            input->acao_pegar = false;
        }

        faxineiro->possui_arma = (faxineiro->possui_mop || faxineiro->possui_soprador);
    }

    // Itens (Mop e Soprador) - Soltar arma equipada
    if (input->acao_soltar) {
        if (faxineiro->arma_equipada == ARMA_MOP && faxineiro->possui_mop) {
            arma_mop->no_chao = true;
            arma_mop->x = faxineiro->x;
            arma_mop->y = faxineiro->y;
            arma_mop->vel_y = 0.0f;
            arma_mop->sala_origem = *sala_atual;
            faxineiro->possui_mop = false;
            if (faxineiro->arma_slot_1 == ARMA_MOP) {
                faxineiro->arma_slot_1 = faxineiro->arma_slot_2;
                faxineiro->arma_slot_2 = ARMA_NENHUMA;
            } else if (faxineiro->arma_slot_2 == ARMA_MOP) {
                faxineiro->arma_slot_2 = ARMA_NENHUMA;
            }
            if (faxineiro->possui_soprador) {
                faxineiro->arma_equipada = ARMA_SOPRADOR;
            } else {
                faxineiro->arma_equipada = ARMA_NENHUMA;
            }
            input->acao_soltar = false;
        } else if (faxineiro->arma_equipada == ARMA_SOPRADOR && faxineiro->possui_soprador) {
            arma_soprador->no_chao = true;
            arma_soprador->x = faxineiro->x;
            arma_soprador->y = faxineiro->y;
            arma_soprador->vel_y = 0.0f;
            arma_soprador->sala_origem = *sala_atual;
            faxineiro->possui_soprador = false;
            if (faxineiro->arma_slot_1 == ARMA_SOPRADOR) {
                faxineiro->arma_slot_1 = faxineiro->arma_slot_2;
                faxineiro->arma_slot_2 = ARMA_NENHUMA;
            } else if (faxineiro->arma_slot_2 == ARMA_SOPRADOR) {
                faxineiro->arma_slot_2 = ARMA_NENHUMA;
            }
            if (faxineiro->possui_mop) {
                faxineiro->arma_equipada = ARMA_MOP;
            } else {
                faxineiro->arma_equipada = ARMA_NENHUMA;
            }
            input->acao_soltar = false;
        } else {
            input->acao_soltar = false;
        }
        faxineiro->possui_arma = (faxineiro->possui_mop || faxineiro->possui_soprador);
    }

    if (faxineiro->cooldown > 0) faxineiro->cooldown--;

    // Disparo do Mop, Soprador Térmico e atualização de projéteis
    if (input->atirando) {
        disparar_arma_jogador(faxineiro, projeteis, max_proj, textos, max_textos);
    }
    atualizar_soprador(faxineiro, soprador, input->atirando, textos, max_textos);
    atualizar_projeteis(projeteis, max_proj, *sala_atual);

    // Hitbox de chamas do Soprador Térmico aquece a anomalia
    if (soprador->ativo && *sala_atual == anomalia->sala_origem) {
        bool chama_x = (soprador->x + soprador->w > anomalia->x - anomalia->w / 2.0f) &&
                       (soprador->x < anomalia->x + anomalia->w / 2.0f);
        bool chama_y = (soprador->y + soprador->h > anomalia->y - anomalia->h / 2.0f) &&
                       (soprador->y < anomalia->y + anomalia->h / 2.0f);
        if (chama_x && chama_y) {
            anomalia->recebendo_calor = true;
            anomalia->recebendo_calor_soprador = true;
        }
    }

    // Coleta de itens consumíveis (refil_agua, bateria e kit_medico)
    float player_left = faxineiro->x - faxineiro->body_w / 2.0f;
    float player_right = faxineiro->x + faxineiro->body_w / 2.0f;
    float player_top = (faxineiro->y + faxineiro->h / 2.0f) - faxineiro->body_h;
    float player_bottom = faxineiro->y + faxineiro->h / 2.0f;

    if (refil_agua->ativo && refil_agua->sala == *sala_atual) {
        bool col_agua = (player_right > refil_agua->x) && (player_left < refil_agua->x + refil_agua->w) &&
                        (player_bottom > refil_agua->y) && (player_top < refil_agua->y + refil_agua->h);
        if (col_agua && faxineiro->refil_agua_qtd < 3) {
            faxineiro->refil_agua_qtd++;
            refil_agua->ativo = false;
            instanciar_texto_flutuante(textos, max_textos, faxineiro->x, player_top - 10.0f, "* refil de agua *");
        }
    }

    if (bateria->ativo && bateria->sala == *sala_atual) {
        bool col_bat = (player_right > bateria->x) && (player_left < bateria->x + bateria->w) &&
                       (player_bottom > bateria->y) && (player_top < bateria->y + bateria->h);
        if (col_bat && faxineiro->bateria_qtd < 3) {
            faxineiro->bateria_qtd++;
            bateria->ativo = false;
            instanciar_texto_flutuante(textos, max_textos, faxineiro->x, player_top - 10.0f, "* bateria *");
        }
    }

    if (kit_medico->ativo && kit_medico->sala == *sala_atual) {
        bool col_kit = (player_right > kit_medico->x) && (player_left < kit_medico->x + kit_medico->w) &&
                       (player_bottom > kit_medico->y) && (player_top < kit_medico->y + kit_medico->h);
        if (col_kit) {
            faxineiro->barra_vida = 100.0f;
            kit_medico->ativo = false;
            instanciar_texto_flutuante(textos, max_textos, faxineiro->x, player_top - 10.0f, "* kit medico *");
        }
    }

    // Radiação se houver elemento radioativo na sala
    if (*sala_atual == anomalia->sala_origem && tabela_periodica[anomalia->numero_atomico].radioativo) {
        if (fabs(faxineiro->x - anomalia->x) <= anomalia->raio_visao) {
            faxineiro->barra_radio += (10.0f / FPS);
            if (faxineiro->barra_radio > 100.0f) faxineiro->barra_radio = 100.0f;
        }
    }

    // Verificação de morte (barra_vida <= 0% ou barra_radio >= 100%)
    if (faxineiro->barra_vida <= 0.0f || faxineiro->barra_radio >= 100.0f) {
        if (faxineiro->barra_vida < 0.0f) faxineiro->barra_vida = 0.0f;
        if (faxineiro->barra_radio > 100.0f) faxineiro->barra_radio = 100.0f;

        faxineiro->morreu = true;
        faxineiro->escalando = false;
        faxineiro->esta_andando = false;
        faxineiro->frame_atual = 0;
        soprador->ativo = false;

        input->eixo_x = 0.0f;
        input->pulo = false;
        input->atirando = false;
        input->acao_pegar = false;
        input->acao_soltar = false;
        input->acao_entrar = false;
        input->acao_descer = false;

        // Larga automaticamente no chão o item/arma que estiver segurando na mão no momento da morte
        if (faxineiro->arma_equipada == ARMA_MOP && faxineiro->possui_mop) {
            arma_mop->no_chao = true;
            arma_mop->x = faxineiro->x;
            arma_mop->y = faxineiro->y;
            arma_mop->vel_y = 0.0f;
            arma_mop->sala_origem = *sala_atual;
            faxineiro->possui_mop = false;
            if (faxineiro->arma_slot_1 == ARMA_MOP) {
                faxineiro->arma_slot_1 = faxineiro->arma_slot_2;
                faxineiro->arma_slot_2 = ARMA_NENHUMA;
            } else if (faxineiro->arma_slot_2 == ARMA_MOP) {
                faxineiro->arma_slot_2 = ARMA_NENHUMA;
            }
            faxineiro->arma_equipada = ARMA_NENHUMA;
        } else if (faxineiro->arma_equipada == ARMA_SOPRADOR && faxineiro->possui_soprador) {
            arma_soprador->no_chao = true;
            arma_soprador->x = faxineiro->x;
            arma_soprador->y = faxineiro->y;
            arma_soprador->vel_y = 0.0f;
            arma_soprador->sala_origem = *sala_atual;
            faxineiro->possui_soprador = false;
            if (faxineiro->arma_slot_1 == ARMA_SOPRADOR) {
                faxineiro->arma_slot_1 = faxineiro->arma_slot_2;
                faxineiro->arma_slot_2 = ARMA_NENHUMA;
            } else if (faxineiro->arma_slot_2 == ARMA_SOPRADOR) {
                faxineiro->arma_slot_2 = ARMA_NENHUMA;
            }
            faxineiro->arma_equipada = ARMA_NENHUMA;
        }
        faxineiro->possui_arma = (faxineiro->possui_mop || faxineiro->possui_soprador);
    }
}
