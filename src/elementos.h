#ifndef ELEMENTOS_H
#define ELEMENTOS_H

#include "defs.h"

void carregar_controles_padrao(Controles *c);
void salvar_jogo(char *nome, int sala, float x, float y, bool possui_arma);
bool carregar_jogo(char *nome, int *sala, float *x, float *y, bool *possui_arma);
char* proximo_campo_csv(char **cursor);
void carregar_tabela_periodica(const char *caminho_arquivo);

#endif
