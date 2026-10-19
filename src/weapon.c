#include "weapon.h"

void disparar_arma_jogador(Jogador *faxineiro, Projetil *projeteis, int max_proj) {
    if (faxineiro->cooldown == 0 && faxineiro->possui_arma) {
        for (int i = 0; i < max_proj; i++) {
            if (!projeteis[i].ativo) {
                projeteis[i].ativo = true;
                projeteis[i].tipo = PROJETIL_AGUA;

                float ponta_x = faxineiro->x;
                float ponta_y = faxineiro->y - (faxineiro->body_h / 2.0) + 4;
                if (faxineiro->direcao_atual == DIR_DIREITA) ponta_x -= 6; else ponta_x += 6;

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
                break;
            }
        }
    }
}

void atualizar_projeteis(Projetil *projeteis, int max_proj, int sala_atual) {
    float limite_mapa_x = (sala_atual == 0) ? (LARGURA_MAPA / 2.0) : LARGURA_MAPA;

    for (int i = 0; i < max_proj; i++) {
        if (projeteis[i].ativo) {
            projeteis[i].x += projeteis[i].vel_x;
            if (projeteis[i].x < 0 || projeteis[i].x > limite_mapa_x) {
                projeteis[i].ativo = false;
            }
        }
    }
}
