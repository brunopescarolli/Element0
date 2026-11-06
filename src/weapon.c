#include "weapon.h"
#include "enemy.h"

void instanciar_texto_flutuante(TextoFlutuante *textos, int max_textos, float x, float y, const char *mensagem, ALLEGRO_COLOR cor) {
    if (mensagem[0] != '-' && mensagem[0] != '+') {
        for (int t = 0; t < max_textos; t++) {
            if (textos[t].ativo && strcmp(textos[t].texto, mensagem) == 0) {
                return;
            }
        }
    }
    int slot = -1;
    for (int t = 0; t < max_textos; t++) {
        if (!textos[t].ativo) {
            slot = t;
            break;
        }
    }
    if (slot == -1) {
        slot = 0;
    }
    textos[slot].x = x;
    textos[slot].y = y;
    strncpy(textos[slot].texto, mensagem, sizeof(textos[slot].texto) - 1);
    textos[slot].texto[sizeof(textos[slot].texto) - 1] = '\0';
    textos[slot].alpha = 255.0f;
    textos[slot].timer = 60;
    textos[slot].ativo = true;
    textos[slot].cor = cor;
}

void disparar_arma_jogador(Jogador *faxineiro, Projetil *projeteis, int max_proj, TextoFlutuante *textos, int max_textos) {
    if (faxineiro->arma_equipada != ARMA_MOP || !faxineiro->possui_mop) return;

    if (faxineiro->barra_agua <= 0.0f) {
        if (faxineiro->refil_agua_qtd > 0) {
            faxineiro->refil_agua_qtd--;
            faxineiro->barra_agua = 100.0f;
        } else {
            float player_top = (faxineiro->y + faxineiro->h / 2.0f) - faxineiro->body_h;
            instanciar_texto_flutuante(textos, max_textos, faxineiro->x, player_top - 10.0f, "* sem água *", al_map_rgb(255, 255, 255));
            return;
        }
    }

    if (faxineiro->cooldown == 0) {
        for (int i = 0; i < max_proj; i++) {
            if (!projeteis[i].ativo) {
                projeteis[i].ativo = true;
                projeteis[i].tipo = PROJETIL_AGUA;

                float offset_arma_x = (faxineiro->direcao_atual == DIR_DIREITA) ? 7.0f : -7.0f;
                float ponta_x = faxineiro->x + ((faxineiro->direcao_atual == DIR_DIREITA) ? -6.0f : 6.0f) + offset_arma_x;
                float ponta_y = faxineiro->y - (faxineiro->body_h / 2.0) + 4;

                int tipo_frame_atual = faxineiro->esta_andando ?
                    (faxineiro->frame_atual == 0 || faxineiro->frame_atual == 2 ? FRAME_WALK1 : FRAME_IDLE) :
                    (faxineiro->frame_atual == 0 ? FRAME_IDLE : FRAME_BRE);
                if (tipo_frame_atual == FRAME_BRE) ponta_y += 1;
                else if (tipo_frame_atual == FRAME_WALK1) ponta_y -= 2;

                float tamanho_cabo = 16.0;
                if (faxineiro->direcao_atual == DIR_ESQUERDA) {
                    projeteis[i].vel_x = -VELOCIDADE_PROJETIL;
                    ponta_x -= tamanho_cabo;
                } else {
                    projeteis[i].vel_x = VELOCIDADE_PROJETIL;
                    ponta_x += tamanho_cabo;
                }

                projeteis[i].x = ponta_x;
                projeteis[i].y = ponta_y;
                faxineiro->cooldown = COOLDOWN_TIRO;
                // Redução de 3x no consumo (duração 3x maior)
                faxineiro->barra_agua -= (20.0f / 3.0f);
                if (faxineiro->barra_agua < 0.0f) faxineiro->barra_agua = 0.0f;
                break;
            }
        }
    }
}

void atualizar_soprador(Jogador *faxineiro, Soprador *soprador, bool atirando,
                        Particula *particulas, int max_particulas,
                        TextoFlutuante *textos, int max_textos) {
    if (faxineiro->arma_equipada == ARMA_SOPRADOR && faxineiro->possui_soprador && atirando) {
        if (faxineiro->barra_bateria <= 0.0f && faxineiro->bateria_qtd > 0) {
            faxineiro->bateria_qtd--;
            faxineiro->barra_bateria = 100.0f;
        }

        if (faxineiro->barra_bateria > 0.0f) {
            soprador->ativo = true;
            soprador->w = 34.0f;
            soprador->h = 16.0f;

            float mao_y = faxineiro->y - (faxineiro->body_h / 2.0f) + 4.0f;
            int tipo_frame_atual = faxineiro->esta_andando ?
                (faxineiro->frame_atual == 0 || faxineiro->frame_atual == 2 ? FRAME_WALK1 : FRAME_IDLE) :
                (faxineiro->frame_atual == 0 ? FRAME_IDLE : FRAME_BRE);
            if (tipo_frame_atual == FRAME_BRE) mao_y += 1.0f;
            else if (tipo_frame_atual == FRAME_WALK1) mao_y -= 2.0f;

            soprador->y = mao_y - (soprador->h / 2.0f);
            float offset_arma_x = (faxineiro->direcao_atual == DIR_DIREITA) ? 7.0f : -7.0f;
            if (faxineiro->direcao_atual == DIR_DIREITA) {
                soprador->x = faxineiro->x + 10.0f + offset_arma_x;
            } else {
                soprador->x = faxineiro->x - 10.0f - soprador->w + offset_arma_x;
            }

            // Instancia partículas de "Ondas de Calor" saindo do bocal do Soprador
            if (particulas != NULL && max_particulas > 0) {
                float dir_x = (faxineiro->direcao_atual == DIR_DIREITA) ? 1.0f : -1.0f;
                float bocal_x = (faxineiro->direcao_atual == DIR_DIREITA)
                    ? soprador->x
                    : (soprador->x + soprador->w - 1.0f);
                float jitter_x = dir_x * (((float)(rand() % 100) / 100.0f) * 1.8f);
                float offset_y = (((float)(rand() % 200) / 100.0f) - 1.0f) * 2.2f;
                float vx = dir_x * (1.35f + (((float)(rand() % 100) / 100.0f) * 0.55f));
                float vy = (((float)(rand() % 200) / 100.0f) - 1.0f) * 0.12f;
                float vida = 18.0f + (float)(rand() % 6);
                ALLEGRO_COLOR cor_onda = al_map_rgba(255, 100, 0, 128);
                instanciar_particula(particulas, max_particulas,
                                     bocal_x + jitter_x, (mao_y - 1.0f) + offset_y,
                                     vx, vy, vida, PARTICULA_CALOR, cor_onda, 4.0f);
            }

            // Redução de 3x no consumo (duração 3x maior)
            faxineiro->barra_bateria -= (0.5f / 3.0f);
            if (faxineiro->barra_bateria < 0.0f) {
                faxineiro->barra_bateria = 0.0f;
            }
        } else {
            soprador->ativo = false;
            float player_top = (faxineiro->y + faxineiro->h / 2.0f) - faxineiro->body_h;
            instanciar_texto_flutuante(textos, max_textos, faxineiro->x, player_top - 10.0f, "* sem bateria *", al_map_rgb(255, 255, 255));
        }
    } else {
        soprador->ativo = false;
    }
}

void atualizar_projeteis(Projetil *projeteis, int max_proj, int sala_atual,
                         Plataforma *plataformas, int total_plat, Escada *escadas, int total_escadas,
                         Particula *particulas, int max_particulas) {
    float limite_mapa_x = (sala_atual == 0) ? (LARGURA_MAPA / 2.0f) : LARGURA_MAPA;

    for (int i = 0; i < max_proj; i++) {
        if (!projeteis[i].ativo) continue;

        projeteis[i].x += projeteis[i].vel_x;
        bool colidiu = false;
        float impacto_x = projeteis[i].x;
        float impacto_y = projeteis[i].y;

        // Colisão com as paredes laterais da sala
        if (projeteis[i].x - 3.0f <= 0.0f) {
            impacto_x = 1.0f;
            colidiu = true;
        } else if (projeteis[i].x + 3.0f >= limite_mapa_x) {
            impacto_x = limite_mapa_x - 1.0f;
            colidiu = true;
        }

        // Colisão com plataformas / chão / obstáculos
        if (!colidiu && plataformas != NULL) {
            for (int p = 0; p < total_plat; p++) {
                if (plataformas[p].sala != sala_atual) continue;
                Plataforma plat = plataformas[p];
                if (projeteis[i].x + 3.0f > plat.x && projeteis[i].x - 3.0f < plat.x + plat.w &&
                    projeteis[i].y + 1.0f > plat.y && projeteis[i].y - 1.0f < plat.y + plat.h) {
                    impacto_x = (projeteis[i].vel_x > 0.0f) ? plat.x : (plat.x + plat.w);
                    colidiu = true;
                    break;
                }
            }
        }

        // Colisão com rampas / escadas inclinadas
        if (!colidiu && escadas != NULL) {
            for (int e = 0; e < total_escadas; e++) {
                if (escadas[e].sala != sala_atual || escadas[e].direcao == 0) continue;
                Escada esc = escadas[e];
                if (projeteis[i].x + 3.0f > esc.x && projeteis[i].x - 3.0f < esc.x + esc.w &&
                    projeteis[i].y + 1.0f > esc.y && projeteis[i].y - 1.0f < esc.y + esc.h) {
                    float progresso = (projeteis[i].x - esc.x) / esc.w;
                    if (progresso < 0.0f) progresso = 0.0f;
                    if (progresso > 1.0f) progresso = 1.0f;
                    float rampa_y = (esc.direcao == 1)
                        ? (esc.y + esc.h - (progresso * esc.h))
                        : (esc.y + (progresso * esc.h));
                    if (projeteis[i].y >= rampa_y) {
                        colidiu = true;
                        break;
                    }
                }
            }
        }

        if (colidiu) {
            if (projeteis[i].tipo == PROJETIL_AGUA) {
                instanciar_splash_agua(particulas, max_particulas, impacto_x, impacto_y, projeteis[i].vel_x);
            }
            projeteis[i].ativo = false;
        }
    }
}
