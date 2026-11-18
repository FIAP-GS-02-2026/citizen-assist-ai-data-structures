#include "arquivo.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define TAM_LINHA 4096
#define NUM_CAMPOS 8

/* Separa a linha em campos no lugar (troca '|' por '\0'). Retorna quantos campos achou. */
static int separar(char *linha, char *campos[], int max) {
    int n = 0;
    char *p = linha;
    campos[n++] = p;
    while (*p && n < max) {
        if (*p == '|') { *p = '\0'; campos[n++] = p + 1; }
        p++;
    }
    while (*p) { if (*p == '|') return max + 1; p++; }   /* campos a mais */
    return n;
}

static void tirar_quebra(char *s) {
    size_t n = strlen(s);
    while (n > 0 && (s[n - 1] == '\n' || s[n - 1] == '\r')) s[--n] = '\0';
}

int arquivo_carregar(const char *caminho, Lista *l, ResumoCarga *resumo) {
    char linha[TAM_LINHA];
    FILE *f = fopen(caminho, "r");
    resumo->carregados = 0;
    resumo->ignorados = 0;
    if (f == NULL) return -1;

    while (fgets(linha, sizeof linha, f) != NULL) {
        char *campos[NUM_CAMPOS];
        char *fim;
        long id;
        Orientacao o;

        if (strchr(linha, '\n') == NULL && !feof(f)) {     /* linha maior que o buffer */
            int c;
            while ((c = fgetc(f)) != '\n' && c != EOF) {}
            resumo->ignorados++;
            continue;
        }
        tirar_quebra(linha);
        if (linha[0] == '\0' || linha[0] == '#') continue;
        /* remove o BOM do UTF-8, se o arquivo foi salvo pelo Bloco de Notas */
        if ((unsigned char)linha[0] == 0xEF && (unsigned char)linha[1] == 0xBB &&
            (unsigned char)linha[2] == 0xBF) memmove(linha, linha + 3, strlen(linha + 3) + 1);

        if (separar(linha, campos, NUM_CAMPOS) != NUM_CAMPOS) { resumo->ignorados++; continue; }
        id = strtol(campos[0], &fim, 10);
        if (*campos[0] == '\0' || *fim != '\0' || id < ID_MINIMO || id > ID_MAXIMO) {
            resumo->ignorados++;
            continue;
        }
        memset(&o, 0, sizeof o);
        o.id = (int)id;
        copiar_texto(o.categoria,     sizeof o.categoria,     campos[1]);
        copiar_texto(o.titulo,        sizeof o.titulo,        campos[2]);
        copiar_texto(o.descricao,     sizeof o.descricao,     campos[3]);
        copiar_texto(o.requisitos,    sizeof o.requisitos,    campos[4]);
        copiar_texto(o.local,         sizeof o.local,         campos[5]);
        copiar_texto(o.fonte,         sizeof o.fonte,         campos[6]);
        copiar_texto(o.verificado_em, sizeof o.verificado_em, campos[7]);

        if (lista_inserir(l, &o) == LISTA_OK) resumo->carregados++;
        else resumo->ignorados++;
    }
    fclose(f);
    return 0;
}

int arquivo_salvar(const char *caminho, const Lista *l) {
    int n = 0;
    No *p;
    char temporario[512];
    FILE *f;

    /* grava num arquivo temporário e só depois troca, para não perder a base se algo falhar */
    if (snprintf(temporario, sizeof temporario, "%s.tmp", caminho) >= (int)sizeof temporario) return -1;
    f = fopen(temporario, "w");
    if (f == NULL) return -1;
    fprintf(f, "# Citizen Assist AI Mobilidade - base de orientações (UTF-8)\n");
    fprintf(f, "# id|categoria|titulo|descricao|requisitos|local|fonte|verificado_em\n");
    for (p = l->inicio; p != NULL; p = p->prox) {
        const Orientacao *o = &p->dado;
        fprintf(f, "%d|%s|%s|%s|%s|%s|%s|%s\n", o->id, o->categoria, o->titulo, o->descricao,
                o->requisitos, o->local, o->fonte, o->verificado_em);
        n++;
    }
    if (fclose(f) != 0) { remove(temporario); return -1; }
    remove(caminho);                         /* no Windows, rename falha se o destino existir */
    if (rename(temporario, caminho) != 0) return -1;
    return n;
}
