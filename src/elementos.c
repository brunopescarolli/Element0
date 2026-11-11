#include "elementos.h"

DadosElemento tabela_periodica[MAX_ELEMENTOS + 1];

void carregar_controles_padrao(Controles *c) {
    c->tecla_pular = ALLEGRO_KEY_SPACE;
    c->tecla_esquerda = ALLEGRO_KEY_A;
    c->tecla_direita = ALLEGRO_KEY_D;
    c->tecla_atirar = ALLEGRO_KEY_F;
    c->tecla_pegar = ALLEGRO_KEY_E;
    c->tecla_soltar = ALLEGRO_KEY_Q;
    c->tecla_entrar = ALLEGRO_KEY_W;
}

#define SAVE_MAGIC 0x454C4D30u
#define SAVE_VERSION 3u

typedef struct {
    unsigned int magic;
    unsigned int version;
    char nome_jogador[30];
    int sala_atual;
    float jogador_x;
    float jogador_y;
    int direcao_atual;

    bool possui_arma;
    bool possui_mop;
    bool possui_soprador;
    int arma_equipada;
    int arma_slot_1;
    int arma_slot_2;

    float barra_vida;
    float barra_radio;
    float barra_agua;
    float barra_bateria;
    int refil_agua_qtd;
    int bateria_qtd;

    bool mop_no_chao;
    int mop_sala;
    float mop_x;
    float mop_y;

    bool soprador_no_chao;
    int soprador_sala;
    float soprador_x;
    float soprador_y;

    bool refil_agua_ativo;
    int refil_agua_sala;
    float refil_agua_x;
    float refil_agua_y;

    bool bateria_ativo;
    int bateria_sala;
    float bateria_x;
    float bateria_y;

    bool kit_medico_ativo;
    int kit_medico_sala;
    float kit_medico_x;
    float kit_medico_y;

    // Anomalia / Inimigo da Sala
    int anomalia_numero_atomico;
    float anomalia_hp;
    float anomalia_temperatura;
    float anomalia_x;
    float anomalia_y;
    bool anomalia_morto;
    bool anomalia_ativo;
} SaveData;

void salvar_jogo(const char *nome, int sala, const Jogador *faxineiro,
                 const Item *arma_mop, const Item *arma_soprador,
                 const Coletavel *refil_agua, const Coletavel *bateria, const Coletavel *kit_medico,
                 const Inimigo *anomalia) {
    FILE *f = fopen("save.dat", "wb");
    if (!f) return;

    SaveData dados;
    memset(&dados, 0, sizeof(SaveData));
    dados.magic = SAVE_MAGIC;
    dados.version = SAVE_VERSION;
    strncpy(dados.nome_jogador, nome ? nome : "", sizeof(dados.nome_jogador) - 1);
    dados.nome_jogador[sizeof(dados.nome_jogador) - 1] = '\0';
    dados.sala_atual = sala;
    dados.jogador_x = faxineiro->x;
    dados.jogador_y = faxineiro->y;
    dados.direcao_atual = faxineiro->direcao_atual;

    dados.possui_arma = faxineiro->possui_arma;
    dados.possui_mop = faxineiro->possui_mop;
    dados.possui_soprador = faxineiro->possui_soprador;
    dados.arma_equipada = faxineiro->arma_equipada;
    dados.arma_slot_1 = faxineiro->arma_slot_1;
    dados.arma_slot_2 = faxineiro->arma_slot_2;

    dados.barra_vida = faxineiro->barra_vida;
    dados.barra_radio = faxineiro->barra_radio;
    dados.barra_agua = faxineiro->barra_agua;
    dados.barra_bateria = faxineiro->barra_bateria;
    dados.refil_agua_qtd = faxineiro->refil_agua_qtd;
    dados.bateria_qtd = faxineiro->bateria_qtd;

    dados.mop_no_chao = arma_mop->no_chao;
    dados.mop_sala = arma_mop->sala_origem;
    dados.mop_x = arma_mop->x;
    dados.mop_y = arma_mop->y;

    dados.soprador_no_chao = arma_soprador->no_chao;
    dados.soprador_sala = arma_soprador->sala_origem;
    dados.soprador_x = arma_soprador->x;
    dados.soprador_y = arma_soprador->y;

    dados.refil_agua_ativo = refil_agua->ativo;
    dados.refil_agua_sala = refil_agua->sala;
    dados.refil_agua_x = refil_agua->x;
    dados.refil_agua_y = refil_agua->y;

    dados.bateria_ativo = bateria->ativo;
    dados.bateria_sala = bateria->sala;
    dados.bateria_x = bateria->x;
    dados.bateria_y = bateria->y;

    dados.kit_medico_ativo = kit_medico->ativo;
    dados.kit_medico_sala = kit_medico->sala;
    dados.kit_medico_x = kit_medico->x;
    dados.kit_medico_y = kit_medico->y;

    if (anomalia) {
        dados.anomalia_numero_atomico = anomalia->numero_atomico;
        dados.anomalia_hp = anomalia->hp;
        dados.anomalia_temperatura = anomalia->temperatura;
        dados.anomalia_x = anomalia->x;
        dados.anomalia_y = anomalia->y;
        dados.anomalia_morto = anomalia->morto;
        dados.anomalia_ativo = anomalia->ativo;
    }

    fwrite(&dados, sizeof(SaveData), 1, f);
    fclose(f);
}

bool carregar_jogo(char *nome, int *sala, Jogador *faxineiro,
                   Item *arma_mop, Item *arma_soprador,
                   Coletavel *refil_agua, Coletavel *bateria, Coletavel *kit_medico,
                   Inimigo *anomalia) {
    FILE *f = fopen("save.dat", "rb");
    if (!f) return false;

    SaveData dados;
    size_t lidos = fread(&dados, sizeof(SaveData), 1, f);
    if (lidos == 1 && dados.magic == SAVE_MAGIC && (dados.version == SAVE_VERSION || dados.version == 2u)) {
        fclose(f);

        strncpy(nome, dados.nome_jogador, 29);
        nome[29] = '\0';
        *sala = (dados.sala_atual >= 0 && dados.sala_atual < MAX_SALAS) ? dados.sala_atual : 0;

        faxineiro->x = dados.jogador_x;
        faxineiro->y = dados.jogador_y;
        faxineiro->direcao_atual = (dados.direcao_atual == DIR_ESQUERDA) ? DIR_ESQUERDA : DIR_DIREITA;
        faxineiro->vel_y = 0.0f;
        faxineiro->no_chao = true;
        faxineiro->escalando = false;
        faxineiro->esta_andando = false;
        faxineiro->timer_dano = 0;
        faxineiro->morreu = false;

        faxineiro->possui_mop = dados.possui_mop;
        faxineiro->possui_soprador = dados.possui_soprador;
        faxineiro->possui_arma = (dados.possui_mop || dados.possui_soprador);
        faxineiro->arma_equipada = dados.arma_equipada;
        faxineiro->arma_slot_1 = dados.arma_slot_1;
        faxineiro->arma_slot_2 = dados.arma_slot_2;

        faxineiro->barra_vida = (dados.barra_vida > 0.0f) ? dados.barra_vida : 100.0f;
        faxineiro->barra_radio = (dados.barra_radio < 100.0f && dados.barra_radio >= 0.0f) ? dados.barra_radio : 0.0f;
        faxineiro->barra_agua = dados.barra_agua;
        faxineiro->barra_bateria = dados.barra_bateria;
        faxineiro->refil_agua_qtd = dados.refil_agua_qtd;
        faxineiro->bateria_qtd = dados.bateria_qtd;

        arma_mop->no_chao = dados.mop_no_chao;
        arma_mop->sala_origem = dados.mop_sala;
        arma_mop->x = dados.mop_x;
        arma_mop->y = dados.mop_y;
        arma_mop->vel_y = 0.0f;

        arma_soprador->no_chao = dados.soprador_no_chao;
        arma_soprador->sala_origem = dados.soprador_sala;
        arma_soprador->x = dados.soprador_x;
        arma_soprador->y = dados.soprador_y;
        arma_soprador->vel_y = 0.0f;

        refil_agua->ativo = dados.refil_agua_ativo;
        refil_agua->sala = dados.refil_agua_sala;
        refil_agua->x = dados.refil_agua_x;
        refil_agua->y = dados.refil_agua_y;

        bateria->ativo = dados.bateria_ativo;
        bateria->sala = dados.bateria_sala;
        bateria->x = dados.bateria_x;
        bateria->y = dados.bateria_y;

        kit_medico->ativo = dados.kit_medico_ativo;
        kit_medico->sala = dados.kit_medico_sala;
        kit_medico->x = dados.kit_medico_x;
        kit_medico->y = dados.kit_medico_y;

        // Restauração exata do Inimigo/Anomalia da sala
        if (anomalia && dados.version == SAVE_VERSION) {
            int z = dados.anomalia_numero_atomico;
            if (z >= 0 && z <= MAX_ELEMENTOS) {
                DadosElemento elem = tabela_periodica[z];
                anomalia->numero_atomico = z;
                anomalia->hp = dados.anomalia_hp;
                anomalia->hp_max = (elem.pontos_de_vida_dureza > 0.0f) ? elem.pontos_de_vida_dureza : 80.0f;
                anomalia->temperatura = (dados.anomalia_temperatura >= 20.0f) ? dados.anomalia_temperatura : 20.0f;
                anomalia->x = dados.anomalia_x;
                anomalia->y = dados.anomalia_y;
                anomalia->vel_y = 0.0f;
                anomalia->vel_x_atual = 0.0f;
                anomalia->no_chao = true;
                anomalia->sala_origem = 1;
                anomalia->alvo_x = 0.0f;
                anomalia->agua_atual = 0.0f;
                anomalia->recebendo_calor = false;
                anomalia->recebendo_calor_jogador = false;
                anomalia->recebendo_calor_soprador = false;
                anomalia->morto = dados.anomalia_morto;
                anomalia->ativo = dados.anomalia_ativo;
                anomalia->cor = al_map_rgb(elem.cor_r, elem.cor_g, elem.cor_b);
                anomalia->tempo_animacao = 0;
                anomalia->frame_animacao = 0;
                anomalia->direcao_atual = DIR_DIREITA;
                anomalia->timer_resfriamento = 0;
                anomalia->timer_agro = 0;
                anomalia->timer_patrulha = 0;

                // Reavaliação imediata de frame_atual e estado_atual da viscosidade baseados na temperatura restaurada
                float pf = elem.ponto_fusao;
                if (anomalia->temperatura >= pf) {
                    anomalia->estado_atual = LIQUIDO;
                    anomalia->frame_atual = 4;
                } else {
                    anomalia->estado_atual = SOLIDO;
                    float denom = pf - 20.0f;
                    if (denom <= 0.01f) denom = 0.01f;
                    float progresso = (anomalia->temperatura - 20.0f) / denom;
                    if (anomalia->temperatura <= 20.5f || progresso <= 0.0f) {
                        anomalia->frame_atual = 0;
                    } else if (progresso < 0.33f) {
                        anomalia->frame_atual = 1;
                    } else if (progresso < 0.66f) {
                        anomalia->frame_atual = 2;
                    } else {
                        anomalia->frame_atual = 3;
                    }
                }
            }
        }

        return true;
    }

    // Compatibilidade com formato antigo em texto (5 campos)
    rewind(f);
    int arma_temp = 0;
    char nome_temp[30] = "";
    int sala_temp = 0;
    float x_temp = 32.0f, y_temp = 101.0f;
    if (fscanf(f, "%29s\n%d\n%f\n%f\n%d", nome_temp, &sala_temp, &x_temp, &y_temp, &arma_temp) == 5) {
        fclose(f);
        strncpy(nome, nome_temp, 29);
        nome[29] = '\0';
        *sala = (sala_temp >= 0 && sala_temp < MAX_SALAS) ? sala_temp : 0;
        faxineiro->x = x_temp;
        faxineiro->y = y_temp;
        faxineiro->vel_y = 0.0f;
        faxineiro->no_chao = true;
        faxineiro->escalando = false;
        faxineiro->morreu = false;

        bool tinha_mop = (arma_temp == 1);
        faxineiro->possui_mop = tinha_mop;
        faxineiro->possui_soprador = false;
        faxineiro->possui_arma = tinha_mop;
        faxineiro->arma_equipada = tinha_mop ? ARMA_MOP : ARMA_NENHUMA;
        faxineiro->arma_slot_1 = tinha_mop ? ARMA_MOP : ARMA_NENHUMA;
        faxineiro->arma_slot_2 = ARMA_NENHUMA;
        faxineiro->barra_vida = 100.0f;
        faxineiro->barra_radio = 0.0f;
        faxineiro->barra_agua = 100.0f;
        faxineiro->barra_bateria = 100.0f;
        faxineiro->refil_agua_qtd = 0;
        faxineiro->bateria_qtd = 0;

        arma_mop->no_chao = !tinha_mop;
        arma_mop->sala_origem = 0;
        arma_mop->x = 62.0f;
        arma_mop->y = 127.0f;
        arma_mop->vel_y = 0.0f;

        arma_soprador->no_chao = true;
        arma_soprador->sala_origem = 0;
        arma_soprador->x = 80.0f;
        arma_soprador->y = 127.0f;
        arma_soprador->vel_y = 0.0f;

        refil_agua->ativo = true;
        refil_agua->sala = 0;
        refil_agua->x = 100.0f;
        refil_agua->y = 115.0f;

        bateria->ativo = true;
        bateria->sala = 0;
        bateria->x = 120.0f;
        bateria->y = 115.0f;

        kit_medico->ativo = true;
        kit_medico->sala = 0;
        kit_medico->x = 140.0f;
        kit_medico->y = 115.0f;

        // Atualiza imediatamente para o novo formato binário
        salvar_jogo(nome, *sala, faxineiro, arma_mop, arma_soprador, refil_agua, bateria, kit_medico, anomalia);
        return true;
    }

    fclose(f);
    remove("save.dat");
    return false;
}

#define TABELA_MAGIC 0x54414230u
#define TABELA_VERSION 1u

typedef struct {
    unsigned int magic;
    unsigned int version;
    bool descoberto_global[MAX_ELEMENTOS + 1];
    char descobridor_global[MAX_ELEMENTOS + 1][32];
    bool novo_na_tabela[MAX_ELEMENTOS + 1];
} ProgressoTabelaData;

void salvar_progresso_tabela(const bool *descoberto_global,
                             const char descobridor_global[MAX_ELEMENTOS + 1][32],
                             const bool *novo_na_tabela) {
    FILE *f = fopen("progresso_tabela.dat", "wb");
    if (!f) return;

    ProgressoTabelaData dados;
    memset(&dados, 0, sizeof(ProgressoTabelaData));
    dados.magic = TABELA_MAGIC;
    dados.version = TABELA_VERSION;
    if (descoberto_global) {
        memcpy(dados.descoberto_global, descoberto_global, sizeof(dados.descoberto_global));
    }
    if (descobridor_global) {
        memcpy(dados.descobridor_global, descobridor_global, sizeof(dados.descobridor_global));
    }
    if (novo_na_tabela) {
        memcpy(dados.novo_na_tabela, novo_na_tabela, sizeof(dados.novo_na_tabela));
    }

    fwrite(&dados, sizeof(ProgressoTabelaData), 1, f);
    fclose(f);
}

bool carregar_progresso_tabela(bool *descoberto_global,
                               char descobridor_global[MAX_ELEMENTOS + 1][32],
                               bool *novo_na_tabela) {
    if (descoberto_global) {
        memset(descoberto_global, 0, sizeof(bool) * (MAX_ELEMENTOS + 1));
    }
    if (descobridor_global) {
        memset(descobridor_global, 0, sizeof(char) * (MAX_ELEMENTOS + 1) * 32);
    }
    if (novo_na_tabela) {
        memset(novo_na_tabela, 0, sizeof(bool) * (MAX_ELEMENTOS + 1));
    }

    FILE *f = fopen("progresso_tabela.dat", "rb");
    if (!f) return false;

    ProgressoTabelaData dados;
    size_t lidos = fread(&dados, sizeof(ProgressoTabelaData), 1, f);
    fclose(f);

    if (lidos == 1 && dados.magic == TABELA_MAGIC && dados.version == TABELA_VERSION) {
        if (descoberto_global) {
            memcpy(descoberto_global, dados.descoberto_global, sizeof(dados.descoberto_global));
        }
        if (descobridor_global) {
            memcpy(descobridor_global, dados.descobridor_global, sizeof(dados.descobridor_global));
        }
        if (novo_na_tabela) {
            memcpy(novo_na_tabela, dados.novo_na_tabela, sizeof(dados.novo_na_tabela));
        }
        return true;
    }

    return false;
}

char* proximo_campo_csv(char **cursor) {
    if (!*cursor) return NULL;
    char *inicio = *cursor;
    char *virgula = strchr(inicio, ',');
    if (virgula) {
        *virgula = '\0';
        *cursor = virgula + 1;
    } else {
        *cursor = NULL;
    }
    inicio[strcspn(inicio, "\r\n")] = 0;
    while (*inicio == ' ' || *inicio == '\t') {
        inicio++;
    }
    size_t len = strlen(inicio);
    while (len > 0 && (inicio[len - 1] == ' ' || inicio[len - 1] == '\t')) {
        inicio[len - 1] = '\0';
        len--;
    }
    return inicio;
}

void carregar_tabela_periodica(const char *caminho_arquivo) {
    FILE *file = NULL;
    char caminho_alt[256];
    snprintf(caminho_alt, sizeof(caminho_alt), "../%s", caminho_arquivo);

    // Se existir ../assets/elementos.csv (ex: executando de dentro de dist/ ou bin/),
    // verifica se o arquivo local possui o cabeçalho atualizado com Grupo_Gameplay.
    FILE *f_local = fopen(caminho_arquivo, "r");
    FILE *f_parent = fopen(caminho_alt, "r");
    if (f_local && f_parent) {
        char hdr_local[1024] = "";
        char hdr_parent[1024] = "";
        if (fgets(hdr_local, sizeof(hdr_local), f_local) &&
            fgets(hdr_parent, sizeof(hdr_parent), f_parent)) {
            if (!strstr(hdr_local, "Grupo_Gameplay") && strstr(hdr_parent, "Grupo_Gameplay")) {
                fclose(f_local);
                f_local = NULL;
                rewind(f_parent);
                file = f_parent;
                f_parent = NULL;
            }
        }
    }
    if (f_parent) {
        fclose(f_parent);
    }
    if (!file && f_local) {
        rewind(f_local);
        file = f_local;
    }
    if (!file) {
        file = fopen(caminho_alt, "r");
    }
    if (!file) {
        printf("ERRO CRITICO: Nao foi possivel abrir '%s'!\n", caminho_arquivo);
        return;
    }

    char linha[1024];
    if (!fgets(linha, sizeof(linha), file)) {
        fclose(file);
        return;
    }

    while (fgets(linha, sizeof(linha), file)) {
        linha[strcspn(linha, "\r\n")] = 0;
        if (linha[0] == '\0') continue;

        char *cursor = linha;
        DadosElemento e;
        memset(&e, 0, sizeof(DadosElemento));

        char *str_num = proximo_campo_csv(&cursor);
        if (str_num) str_num[strcspn(str_num, "\r\n")] = 0;
        e.numero_atomico = str_num ? atoi(str_num) : 0;

        char *str_simbolo = proximo_campo_csv(&cursor);
        if (str_simbolo) str_simbolo[strcspn(str_simbolo, "\r\n")] = 0;
        strncpy(e.simbolo, str_simbolo ? str_simbolo : "", sizeof(e.simbolo) - 1);
        e.simbolo[sizeof(e.simbolo) - 1] = '\0';

        char *str_nome = proximo_campo_csv(&cursor);
        if (str_nome) str_nome[strcspn(str_nome, "\r\n")] = 0;
        strncpy(e.nome, str_nome ? str_nome : "", sizeof(e.nome) - 1);
        e.nome[sizeof(e.nome) - 1] = '\0';

        char *str_periodo = proximo_campo_csv(&cursor);
        if (str_periodo) str_periodo[strcspn(str_periodo, "\r\n")] = 0;
        e.periodo = str_periodo ? atoi(str_periodo) : 0;

        char *str_familia = proximo_campo_csv(&cursor);
        if (str_familia) str_familia[strcspn(str_familia, "\r\n")] = 0;
        strncpy(e.grupo_familia, str_familia ? str_familia : "", sizeof(e.grupo_familia) - 1);
        e.grupo_familia[sizeof(e.grupo_familia) - 1] = '\0';

        char *str_categoria = proximo_campo_csv(&cursor);
        if (str_categoria) str_categoria[strcspn(str_categoria, "\r\n")] = 0;
        strncpy(e.categoria, str_categoria ? str_categoria : "", sizeof(e.categoria) - 1);
        e.categoria[sizeof(e.categoria) - 1] = '\0';

        char *str_estado = proximo_campo_csv(&cursor);
        if (str_estado) str_estado[strcspn(str_estado, "\r\n")] = 0;
        if (str_estado && (strcmp(str_estado, "Sólido") == 0 || strcmp(str_estado, "Solido") == 0)) e.estado_fisico_20C = SOLIDO;
        else if (str_estado && (strcmp(str_estado, "Líquido") == 0 || strcmp(str_estado, "Liquido") == 0)) e.estado_fisico_20C = LIQUIDO;
        else e.estado_fisico_20C = GASOSO;

        char *str_mag = proximo_campo_csv(&cursor);
        if (str_mag) str_mag[strcspn(str_mag, "\r\n")] = 0;
        e.magnetico = (str_mag && strcmp(str_mag, "Sim") == 0);

        char *str_rad = proximo_campo_csv(&cursor);
        if (str_rad) str_rad[strcspn(str_rad, "\r\n")] = 0;
        e.radioativo = (str_rad && strcmp(str_rad, "Sim") == 0);

        char *str_fusao = proximo_campo_csv(&cursor);
        if (str_fusao) str_fusao[strcspn(str_fusao, "\r\n")] = 0;
        e.ponto_fusao = (str_fusao && strlen(str_fusao) > 0) ? atof(str_fusao) : TEMP_INDETERMINADA;

        char *str_ebulicao = proximo_campo_csv(&cursor);
        if (str_ebulicao) str_ebulicao[strcspn(str_ebulicao, "\r\n")] = 0;
        e.ponto_ebulicao = (str_ebulicao && strlen(str_ebulicao) > 0) ? atof(str_ebulicao) : TEMP_INDETERMINADA;

        char *str_hp = proximo_campo_csv(&cursor);
        if (str_hp) str_hp[strcspn(str_hp, "\r\n")] = 0;
        e.pontos_de_vida_dureza = (str_hp && strlen(str_hp) > 0) ? atof(str_hp) : 100.0;

        char *str_estavel = proximo_campo_csv(&cursor);
        if (str_estavel) str_estavel[strcspn(str_estavel, "\r\n")] = 0;
        strncpy(e.estavel_argonio, (str_estavel && strlen(str_estavel) > 0) ? str_estavel : "Sim", sizeof(e.estavel_argonio) - 1);
        e.estavel_argonio[sizeof(e.estavel_argonio) - 1] = '\0';

        char *str_grupo_gp = proximo_campo_csv(&cursor);
        if (str_grupo_gp) str_grupo_gp[strcspn(str_grupo_gp, "\r\n")] = 0;
        strncpy(e.grupo_gameplay, (str_grupo_gp && strlen(str_grupo_gp) > 0) ? str_grupo_gp : "Nenhum", sizeof(e.grupo_gameplay) - 1);
        e.grupo_gameplay[sizeof(e.grupo_gameplay) - 1] = '\0';
        e.grupo_gameplay[strcspn(e.grupo_gameplay, "\r\n")] = 0;

        char *str_reacao_agua = proximo_campo_csv(&cursor);
        if (str_reacao_agua) str_reacao_agua[strcspn(str_reacao_agua, "\r\n")] = 0;
        strncpy(e.reacao_agua, (str_reacao_agua && strlen(str_reacao_agua) > 0) ? str_reacao_agua : "Resfriamento 4x", sizeof(e.reacao_agua) - 1);
        e.reacao_agua[sizeof(e.reacao_agua) - 1] = '\0';
        e.reacao_agua[strcspn(e.reacao_agua, "\r\n")] = 0;

        char *str_cor_r = proximo_campo_csv(&cursor);
        if (str_cor_r) str_cor_r[strcspn(str_cor_r, "\r\n")] = 0;
        e.cor_r = (str_cor_r && strlen(str_cor_r) > 0) ? atoi(str_cor_r) : 255;

        char *str_cor_g = proximo_campo_csv(&cursor);
        if (str_cor_g) str_cor_g[strcspn(str_cor_g, "\r\n")] = 0;
        e.cor_g = (str_cor_g && strlen(str_cor_g) > 0) ? atoi(str_cor_g) : 255;

        char *str_cor_b = proximo_campo_csv(&cursor);
        if (str_cor_b) str_cor_b[strcspn(str_cor_b, "\r\n")] = 0;
        e.cor_b = (str_cor_b && strlen(str_cor_b) > 0) ? atoi(str_cor_b) : 255;

        if (e.numero_atomico >= 0 && e.numero_atomico <= MAX_ELEMENTOS) {
            tabela_periodica[e.numero_atomico] = e;
        }
    }
    fclose(file);
}

int sortear_elemento_por_grupo(const char *grupo_gameplay) {
    if (!grupo_gameplay || grupo_gameplay[0] == '\0') {
        printf("ERRO [sortear_elemento_por_grupo]: Grupo invalido (NULL ou vazio)!\n");
        return -1;
    }

    int array_temporario[MAX_ELEMENTOS + 1];
    int quantidade_encontrada = 0;

    for (int i = 0; i <= MAX_ELEMENTOS; i++) {
        if (tabela_periodica[i].simbolo[0] == '\0') {
            continue;
        }
        if (strcmp(tabela_periodica[i].grupo_gameplay, grupo_gameplay) == 0) {
            array_temporario[quantidade_encontrada++] = tabela_periodica[i].numero_atomico;
        }
    }

    if (quantidade_encontrada > 0) {
        return array_temporario[rand() % quantidade_encontrada];
    }

    printf("ERRO [sortear_elemento_por_grupo]: Nenhum elemento encontrado para o grupo '%s'!\n", grupo_gameplay);
    return -1;
}
