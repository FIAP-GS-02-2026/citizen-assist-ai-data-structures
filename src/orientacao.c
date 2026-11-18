#include "orientacao.h"

#include <ctype.h>
#include <stdio.h>
#include <string.h>

void copiar_texto(char *destino, size_t tamanho, const char *origem) {
    size_t n;
    if (tamanho == 0) return;
    n = strlen(origem);
    if (n >= tamanho) {
        n = tamanho - 1;
        /* recua até o início de um caractere UTF-8 (bytes 10xxxxxx são continuação) */
        while (n > 0 && ((unsigned char)origem[n] & 0xC0) == 0x80) n--;
    }
    memcpy(destino, origem, n);
    destino[n] = '\0';
}

/* Troca as letras acentuadas mais comuns do português (2 bytes em UTF-8) pela letra sem acento. */
static char letra_sem_acento(unsigned char b1, unsigned char b2) {
    if (b1 != 0xC3) return 0;
    switch (b2) {
        case 0xA0: case 0xA1: case 0xA2: case 0xA3: case 0xA4:
        case 0x80: case 0x81: case 0x82: case 0x83: case 0x84: return 'a';
        case 0xA7: case 0x87:                                  return 'c';
        case 0xA8: case 0xA9: case 0xAA: case 0xAB:
        case 0x88: case 0x89: case 0x8A: case 0x8B:            return 'e';
        case 0xAC: case 0xAD: case 0xAE: case 0xAF:
        case 0x8C: case 0x8D: case 0x8E: case 0x8F:            return 'i';
        case 0xB2: case 0xB3: case 0xB4: case 0xB5: case 0xB6:
        case 0x92: case 0x93: case 0x94: case 0x95: case 0x96: return 'o';
        case 0xB9: case 0xBA: case 0xBB: case 0xBC:
        case 0x99: case 0x9A: case 0x9B: case 0x9C:            return 'u';
        case 0xB1: case 0x91:                                  return 'n';
        default:                                               return 0;
    }
}

void normalizar(const char *origem, char *destino, size_t tamanho) {
    size_t j = 0;
    const unsigned char *p = (const unsigned char *)origem;
    if (tamanho == 0) return;
    while (*p && isspace(*p)) p++;                    /* espaços do início */
    while (*p && j + 1 < tamanho) {
        if (*p < 0x80) {
            destino[j++] = (char)tolower(*p);
            p++;
        } else {
            char c = p[1] ? letra_sem_acento(p[0], p[1]) : 0;
            if (c) { destino[j++] = c; p += 2; }
            else   { destino[j++] = (char)*p; p++; }  /* outro byte: copia como está */
        }
    }
    while (j > 0 && isspace((unsigned char)destino[j - 1])) j--;   /* espaços do fim */
    destino[j] = '\0';
}

int contem_normalizado(const char *texto, const char *trecho) {
    char a[TAM_DESCRICAO], b[TAM_DESCRICAO];
    normalizar(texto, a, sizeof a);
    normalizar(trecho, b, sizeof b);
    if (b[0] == '\0') return 0;
    return strstr(a, b) != NULL;
}

int comparar_normalizado(const char *a, const char *b) {
    char na[TAM_DESCRICAO], nb[TAM_DESCRICAO];
    normalizar(a, na, sizeof na);
    normalizar(b, nb, sizeof nb);
    return strcmp(na, nb);
}

int data_valida(const char *data) {
    static const int dias[] = {31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31};
    int ano, mes, dia, max, i;
    if (strlen(data) != 10 || data[4] != '-' || data[7] != '-') return 0;
    for (i = 0; i < 10; i++) {
        if (i == 4 || i == 7) continue;
        if (!isdigit((unsigned char)data[i])) return 0;
    }
    if (sscanf(data, "%4d-%2d-%2d", &ano, &mes, &dia) != 3) return 0;
    if (ano < 2000 || ano > 2100 || mes < 1 || mes > 12) return 0;
    max = dias[mes - 1];
    if (mes == 2 && ((ano % 4 == 0 && ano % 100 != 0) || ano % 400 == 0)) max = 29;
    return dia >= 1 && dia <= max;
}

static int vazio(const char *s) {
    while (*s) { if (!isspace((unsigned char)*s)) return 0; s++; }
    return 1;
}

const char *validar_orientacao(const Orientacao *o) {
    if (o->id < ID_MINIMO || o->id > ID_MAXIMO) return "ID fora da faixa permitida (1 a 99999).";
    if (vazio(o->titulo))     return "O título é obrigatório.";
    if (vazio(o->categoria))  return "A categoria é obrigatória.";
    if (vazio(o->descricao))  return "A descrição é obrigatória.";
    if (vazio(o->local))      return "O local de atendimento é obrigatório.";
    if (vazio(o->fonte))      return "A fonte oficial é obrigatória.";
    if (!data_valida(o->verificado_em)) return "Data de verificação inválida (use AAAA-MM-DD).";
    return NULL;
}

int largura_utf8(const char *s) {
    int n = 0;
    for (; *s; s++) if (((unsigned char)*s & 0xC0) != 0x80) n++;   /* conta só o 1º byte de cada letra */
    return n;
}

void imprimir_coluna(const char *texto, int largura) {
    int usados = 0;
    const unsigned char *p = (const unsigned char *)texto;
    while (*p) {
        int bytes = (*p < 0x80) ? 1 : (*p >= 0xF0) ? 4 : (*p >= 0xE0) ? 3 : 2;
        if (usados == largura) break;
        fwrite(p, 1, (size_t)bytes, stdout);
        p += bytes;
        usados++;
    }
    while (usados++ < largura) putchar(' ');
}

void imprimir_resumo(const Orientacao *o) {
    printf("  [%3d] ", o->id);
    imprimir_coluna(o->categoria, 29);
    printf(" | %s\n", o->titulo);
}

void imprimir_ficha(const Orientacao *o) {
    printf("\n  ID %d · %s\n", o->id, o->categoria);
    printf("  %s\n", o->titulo);
    printf("  ------------------------------------------------------------\n");
    printf("  %s\n\n", o->descricao);
    if (o->requisitos[0]) printf("  Documentos/requisitos: %s\n", o->requisitos);
    printf("  Onde: %s\n", o->local);
    printf("  Fonte: %s (verificada em %s)\n", o->fonte, o->verificado_em);
    printf("  Aviso: este protótipo não é canal oficial; confirme na fonte.\n");
}
