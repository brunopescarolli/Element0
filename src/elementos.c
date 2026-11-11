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
#define SAVE_VERSION 2u

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
} SaveData;

void salvar_jogo(const char *nome, int sala, const Jogador *faxineiro,
                 const Item *arma_mop, const Item *arma_soprador,
                 const Coletavel *refil_agua, const Coletavel *bateria, const Coletavel *kit_medico) {
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

    fwrite(&dados, sizeof(SaveData), 1, f);
    fclose(f);
}

bool carregar_jogo(char *nome, int *sala, Jogador *faxineiro,
                   Item *arma_mop, Item *arma_soprador,
                   Coletavel *refil_agua, Coletavel *bateria, Coletavel *kit_medico) {
    FILE *f = fopen("save.dat", "rb");
    if (!f) return false;

    SaveData dados;
    size_t lidos = fread(&dados, sizeof(SaveData), 1, f);
    if (lidos == 1 && dados.magic == SAVE_MAGIC && dados.version == SAVE_VERSION) {
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
        salvar_jogo(nome, *sala, faxineiro, arma_mop, arma_soprador, refil_agua, bateria, kit_medico);
        return true;
    }

    fclose(f);
    remove("save.dat");
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
        char *newline = strpbrk(inicio, "\r\n");
        if (newline) *newline = '\0';
        *cursor = NULL;
    }
    return inicio;
}

void carregar_tabela_periodica(const char *caminho_arquivo) {
    FILE *file = fopen(caminho_arquivo, "r");
    if (!file) {
        char caminho_alt[256];
        snprintf(caminho_alt, sizeof(caminho_alt), "../%s", caminho_arquivo);
        file = fopen(caminho_alt, "r");
    }
    if (!file) {
        printf("Aviso: '%s' nao encontrado. Usando fallbacks.\n", caminho_arquivo);
        tabela_periodica[31].numero_atomico = 31;
        strncpy(tabela_periodica[31].simbolo, "Ga", sizeof(tabela_periodica[31].simbolo) - 1);
        tabela_periodica[31].simbolo[sizeof(tabela_periodica[31].simbolo) - 1] = '\0';
        strncpy(tabela_periodica[31].nome, "Gálio", sizeof(tabela_periodica[31].nome) - 1);
        tabela_periodica[31].nome[sizeof(tabela_periodica[31].nome) - 1] = '\0';
        strncpy(tabela_periodica[31].categoria, "Metal", sizeof(tabela_periodica[31].categoria) - 1);
        tabela_periodica[31].categoria[sizeof(tabela_periodica[31].categoria) - 1] = '\0';
        tabela_periodica[31].ponto_fusao = 29.8f;
        tabela_periodica[31].ponto_ebulicao = 2400.0f;
        return;
    }

    char linha[1024];
    fgets(linha, sizeof(linha), file);

    while (fgets(linha, sizeof(linha), file)) {
        char *cursor = linha;
        DadosElemento e;
        memset(&e, 0, sizeof(DadosElemento));

        char *str_num = proximo_campo_csv(&cursor);
        e.numero_atomico = str_num ? atoi(str_num) : 0;

        char *str_simbolo = proximo_campo_csv(&cursor);
        strncpy(e.simbolo, str_simbolo ? str_simbolo : "", sizeof(e.simbolo) - 1);
        e.simbolo[sizeof(e.simbolo) - 1] = '\0';

        char *str_nome = proximo_campo_csv(&cursor);
        strncpy(e.nome, str_nome ? str_nome : "", sizeof(e.nome) - 1);
        e.nome[sizeof(e.nome) - 1] = '\0';

        char *str_periodo = proximo_campo_csv(&cursor);
        e.periodo = str_periodo ? atoi(str_periodo) : 0;

        char *str_familia = proximo_campo_csv(&cursor);
        strncpy(e.grupo_familia, str_familia ? str_familia : "", sizeof(e.grupo_familia) - 1);
        e.grupo_familia[sizeof(e.grupo_familia) - 1] = '\0';

        char *str_categoria = proximo_campo_csv(&cursor);
        strncpy(e.categoria, str_categoria ? str_categoria : "", sizeof(e.categoria) - 1);
        e.categoria[sizeof(e.categoria) - 1] = '\0';

        char *str_estado = proximo_campo_csv(&cursor);
        if (str_estado && (strcmp(str_estado, "Sólido") == 0 || strcmp(str_estado, "Solido") == 0)) e.estado_fisico_20C = SOLIDO;
        else if (str_estado && (strcmp(str_estado, "Líquido") == 0 || strcmp(str_estado, "Liquido") == 0)) e.estado_fisico_20C = LIQUIDO;
        else e.estado_fisico_20C = GASOSO;

        char *str_mag = proximo_campo_csv(&cursor);
        e.magnetico = (str_mag && strcmp(str_mag, "Sim") == 0);

        char *str_rad = proximo_campo_csv(&cursor);
        e.radioativo = (str_rad && strcmp(str_rad, "Sim") == 0);

        char *str_fusao = proximo_campo_csv(&cursor);
        e.ponto_fusao = (str_fusao && strlen(str_fusao) > 0) ? atof(str_fusao) : TEMP_INDETERMINADA;

        char *str_ebulicao = proximo_campo_csv(&cursor);
        e.ponto_ebulicao = (str_ebulicao && strlen(str_ebulicao) > 0) ? atof(str_ebulicao) : TEMP_INDETERMINADA;

        char *str_hp = proximo_campo_csv(&cursor);
        e.pontos_de_vida_dureza = (str_hp && strlen(str_hp) > 0) ? atof(str_hp) : 100.0;

        if (e.numero_atomico > 0 && e.numero_atomico <= MAX_ELEMENTOS) {
            tabela_periodica[e.numero_atomico] = e;
        }
    }
    fclose(file);
}
