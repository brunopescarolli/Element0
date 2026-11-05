#ifndef ELEMENTOS_H
#define ELEMENTOS_H

#include "defs.h"

void carregar_controles_padrao(Controles *c);
void salvar_jogo(const char *nome, int sala, const Jogador *faxineiro,
                 const Item *arma_mop, const Item *arma_soprador,
                 const Coletavel *refil_agua, const Coletavel *bateria, const Coletavel *kit_medico);
bool carregar_jogo(char *nome, int *sala, Jogador *faxineiro,
                   Item *arma_mop, Item *arma_soprador,
                   Coletavel *refil_agua, Coletavel *bateria, Coletavel *kit_medico);
char* proximo_campo_csv(char **cursor);
void carregar_tabela_periodica(const char *caminho_arquivo);

#endif
