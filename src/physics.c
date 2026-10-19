#include "physics.h"

bool check_aabb_center(float cx1, float cy1, float w1, float h1, float cx2, float cy2, float w2, float h2) {
    bool col_x = (cx1 + w1 / 2.0 > cx2 - w2 / 2.0) && (cx1 - w1 / 2.0 < cx2 + w2 / 2.0);
    bool col_y = (cy1 + h1 / 2.0 > cy2 - h2 / 2.0) && (cy1 - h1 / 2.0 < cy2 + h2 / 2.0);
    return col_x && col_y;
}

void resolve_platform_collisions(float x, float *y, float w, float h, float *vel_y, bool *no_chao,
                                 int sala, Plataforma *plataformas, int total_plat, bool descer_atravessavel) {
    for (int p = 0; p < total_plat; p++) {
        if (plataformas[p].sala != sala) continue;
        Plataforma plat = plataformas[p];

        if (descer_atravessavel && plat.atravessavel) continue;

        bool colide_x = (x + w / 2.0 > plat.x) && (x - w / 2.0 < plat.x + plat.w);
        bool colide_y = (*y + h / 2.0 > plat.y) && (*y - h / 2.0 < plat.y + plat.h);

        if (colide_x && colide_y) {
            float pe_anterior = (*y - *vel_y) + h / 2.0;
            if (*vel_y >= 0 && pe_anterior <= plat.y + 0.1) {
                *y = plat.y - h / 2.0;
                *vel_y = 0;
                *no_chao = true;
            }
        }
    }
}

void resolve_ladder_collisions(float x, float *y, float w, float h, float *vel_y, bool *no_chao,
                               float mov_x, int sala, Escada *escadas, int total_escadas, bool quer_descer) {
    for (int e = 0; e < total_escadas; e++) {
        if (escadas[e].sala != sala) continue;
        Escada esc = escadas[e];

        bool colide_x = (x + w / 2.0 >= esc.x) && (x - w / 2.0 <= esc.x + esc.w);
        if (colide_x) {
            float calc_x = x;
            if (calc_x < esc.x) calc_x = esc.x;
            if (calc_x > esc.x + esc.w) calc_x = esc.x + esc.w;

            float progresso = (calc_x - esc.x) / esc.w;
            float chao_rampa_y = (esc.direcao == 1) ? (esc.y + esc.h - (progresso * esc.h)) : (esc.y + (progresso * esc.h));

            float pe_entidade = *y + h / 2.0;
            float pe_anterior = (*y - *vel_y) + h / 2.0;

            bool descendo_rampa = (esc.direcao == 1 && mov_x < 0) || (esc.direcao == -1 && mov_x > 0);
            bool no_topo = (pe_anterior <= esc.y + 8.0);

            if (no_topo && descendo_rampa && !quer_descer) {
                continue;
            }

            if (*vel_y >= 0 && pe_entidade >= chao_rampa_y - 15.0 && pe_anterior <= chao_rampa_y + 15.0) {
                *y = chao_rampa_y - h / 2.0;
                *vel_y = 0;
                *no_chao = true;
            }
        }
    }
}
