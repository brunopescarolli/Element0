#ifndef PHYSICS_H
#define PHYSICS_H

#include "defs.h"

bool check_aabb_center(float cx1, float cy1, float w1, float h1, float cx2, float cy2, float w2, float h2);

void resolve_platform_collisions(float x, float *y, float w, float h, float *vel_y, bool *no_chao,
                                 int sala, Plataforma *plataformas, int total_plat, bool descer_atravessavel);

void resolve_ladder_collisions(float x, float *y, float w, float h, float *vel_y, bool *no_chao,
                               float mov_x, int sala, Escada *escadas, int total_escadas, bool quer_descer);

#endif
