#ifndef ELEMENTOS_H
#define ELEMENTOS_H

#include "defs.h"

void carregar_controles_padrao(Controles *c);
void salvar_jogo(const char *nome, int sala, const Jogador *faxineiro,
                 const Item *arma_mop, const Item *arma_soprador,
                 const Coletavel *refil_agua, const Coletavel *bateria, const Coletavel *kit_medico,
                 const Inimigo *anomalia);
bool carregar_jogo(char *nome, int *sala, Jogador *faxineiro,
                   Item *arma_mop, Item *arma_soprador,
                   Coletavel *refil_agua, Coletavel *bateria, Coletavel *kit_medico,
                   Inimigo *anomalia);
void salvar_progresso_tabela(const bool *descoberto_global,
                             const char descobridor_global[MAX_ELEMENTOS + 1][32],
                             const bool *novo_na_tabela);
bool carregar_progresso_tabela(bool *descoberto_global,
                               char descobridor_global[MAX_ELEMENTOS + 1][32],
                               bool *novo_na_tabela);
char* proximo_campo_csv(char **cursor);
void carregar_tabela_periodica(const char *caminho_arquivo);
int sortear_elemento_por_grupo(const char *grupo_gameplay);

#endif
