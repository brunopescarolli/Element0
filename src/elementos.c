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

void salvar_jogo(char *nome, int sala, float x, float y, bool possui_arma) {
    FILE *f = fopen("save.dat", "w");
    if (f) {
        fprintf(f, "%s\n%d\n%f\n%f\n%d\n", nome, sala, x, y, possui_arma ? 1 : 0);
        fclose(f);
    }
}

bool carregar_jogo(char *nome, int *sala, float *x, float *y, bool *possui_arma) {
    FILE *f = fopen("save.dat", "r");
    if (f) {
        int arma_temp = 0;
        if (fscanf(f, "%29s\n%d\n%f\n%f\n%d", nome, sala, x, y, &arma_temp) == 5) {
            *possui_arma = (arma_temp == 1);
            fclose(f);
            return true;
        }
        fclose(f);
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
        char *newline = strpbrk(inicio, "\r\n");
        if (newline) *newline = '\0';
        *cursor = NULL;
    }
    return inicio;
}

void carregar_tabela_periodica(const char *caminho_arquivo) {
    FILE *file = fopen(caminho_arquivo, "r");
    if (!file) {
        printf("Aviso: '%s' nao encontrado. Usando fallbacks.\n", caminho_arquivo);
        tabela_periodica[31].numero_atomico = 31;
        tabela_periodica[31].ponto_fusao = 29.8;
        return;
    }

    char linha[1024];
    fgets(linha, sizeof(linha), file);

    while (fgets(linha, sizeof(linha), file)) {
        char *cursor = linha;
        DadosElemento e;

        e.numero_atomico = atoi(proximo_campo_csv(&cursor));
        strcpy(e.simbolo, proximo_campo_csv(&cursor) ? "" : "");
        strcpy(e.nome, proximo_campo_csv(&cursor) ? "" : "");
        e.periodo = atoi(proximo_campo_csv(&cursor));
        strcpy(e.grupo_familia, proximo_campo_csv(&cursor) ? "" : "");
        strcpy(e.categoria, proximo_campo_csv(&cursor) ? "" : "");

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
