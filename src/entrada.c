#include "entrada.h"

#include <ctype.h>
#include <errno.h>
#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* Com a variável de ambiente CITIZEN_ECO definida, o texto digitado é repetido na tela.
 * Serve para gerar evidências de teste quando a entrada vem de um arquivo (./citizen_assist < roteiro.txt). */
static int eco_ativo(void) {
    static int eco = -1;
    if (eco < 0) eco = getenv("CITIZEN_ECO") != NULL;
    return eco;
}

int ler_linha(const char *rotulo, char *destino, size_t tamanho) {
    size_t n;
    char *inicio;
    if (rotulo) { printf("%s", rotulo); fflush(stdout); }
    if (fgets(destino, (int)tamanho, stdin) == NULL) {
        destino[0] = '\0';
        printf(eco_ativo() ? "<fim da entrada>\n" : "\n");
        return -1;
    }
    n = strlen(destino);
    if (eco_ativo()) {
        printf("%s", destino);
        if (n == 0 || destino[n - 1] != '\n') printf("\n");
    }
    if (n > 0 && destino[n - 1] != '\n' && !feof(stdin)) {
        int c;                                    /* texto maior que o campo: descarta o resto */
        while ((c = getchar()) != '\n' && c != EOF) {}
        destino[0] = '\0';
        printf("  ! Texto muito longo (máximo de %d caracteres).\n", (int)tamanho - 2);
        return 0;
    }
    while (n > 0 && (destino[n - 1] == '\n' || destino[n - 1] == '\r' ||
                     isspace((unsigned char)destino[n - 1]))) destino[--n] = '\0';
    inicio = destino;
    while (*inicio && isspace((unsigned char)*inicio)) inicio++;
    if (inicio != destino) memmove(destino, inicio, strlen(inicio) + 1);
    if (strchr(destino, '|') != NULL) {
        printf("  ! O caractere '|' não é permitido (é o separador do arquivo).\n");
        destino[0] = '\0';
        return 0;
    }
    return 1;
}

int texto_para_inteiro(const char *texto, int minimo, int maximo, int *valor) {
    const char *p = texto, *q;
    long v;
    while (isspace((unsigned char)*p)) p++;
    q = p;
    if (*q == '+' || *q == '-') q++;
    if (!isdigit((unsigned char)*q)) return 0;          /* vazio, letras, "2a", "1.5"... */
    while (isdigit((unsigned char)*q)) q++;
    while (isspace((unsigned char)*q)) q++;
    if (*q != '\0') return 0;
    errno = 0;
    v = strtol(p, NULL, 10);
    if (errno == ERANGE || v < minimo || v > maximo) return 0;
    *valor = (int)v;
    return 1;
}

int ler_inteiro(const char *rotulo, int minimo, int maximo, int *valor) {
    char buf[64];
    int r = ler_linha(rotulo, buf, sizeof buf);
    if (r <= 0) return r;
    if (buf[0] == '\0') { printf("  ! Nenhum valor digitado.\n"); return 0; }
    if (!texto_para_inteiro(buf, minimo, maximo, valor)) {
        printf("  ! Valor inválido: digite um número inteiro de %d a %d.\n", minimo, maximo);
        return 0;
    }
    return 1;
}

int ler_sim_nao(const char *rotulo, int *sim) {
    char buf[16];
    int r = ler_linha(rotulo, buf, sizeof buf);
    if (r <= 0) return r;
    if (strcmp(buf, "s") == 0 || strcmp(buf, "S") == 0) { *sim = 1; return 1; }
    if (strcmp(buf, "n") == 0 || strcmp(buf, "N") == 0) { *sim = 0; return 1; }
    printf("  ! Responda com s ou n.\n");
    return 0;
}
