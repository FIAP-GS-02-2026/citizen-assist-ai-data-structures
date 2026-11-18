/*
 * orientacao.h - modelo do registro do Citizen Assist AI Mobilidade.
 *
 * Cada registro é uma orientação ao cidadão sobre ônibus municipal, Bilhete Único,
 * gratuidades, acessibilidade, canais de atendimento ou mobilidade elétrica.
 */
#ifndef ORIENTACAO_H
#define ORIENTACAO_H

#include <stddef.h>

#define TAM_TITULO      120
#define TAM_CATEGORIA    48
#define TAM_DESCRICAO  1024
#define TAM_REQUISITOS  512
#define TAM_LOCAL       200
#define TAM_FONTE       200
#define TAM_DATA         11   /* AAAA-MM-DD + '\0' */

#define ID_MINIMO 1
#define ID_MAXIMO 99999

typedef struct {
    int  id;                              /* identificador único */
    char titulo[TAM_TITULO];
    char categoria[TAM_CATEGORIA];
    char descricao[TAM_DESCRICAO];        /* a orientação em si */
    char requisitos[TAM_REQUISITOS];      /* documentos ou condições, separados por "; " */
    char local[TAM_LOCAL];                /* onde o cidadão resolve */
    char fonte[TAM_FONTE];                /* URL oficial consultada */
    char verificado_em[TAM_DATA];         /* data em que a fonte foi conferida */
} Orientacao;

/* Copia texto com limite de tamanho, sem cortar um caractere UTF-8 ao meio. */
void copiar_texto(char *destino, size_t tamanho, const char *origem);

/* Normaliza para comparação: minúsculas, sem acentos (UTF-8 → ASCII) e sem espaços nas pontas. */
void normalizar(const char *origem, char *destino, size_t tamanho);

/* Retorna 1 se "trecho" aparece em "texto", ignorando maiúsculas e acentos. */
int contem_normalizado(const char *texto, const char *trecho);

/* Compara dois textos ignorando maiúsculas e acentos (como strcmp). */
int comparar_normalizado(const char *a, const char *b);

/* Valida uma data no formato AAAA-MM-DD (inclui ano bissexto). */
int data_valida(const char *data);

/* Valida os campos obrigatórios. Retorna NULL se estiver tudo certo ou a mensagem do erro. */
const char *validar_orientacao(const Orientacao *o);

/* Largura visível de um texto UTF-8 e impressão em coluna de largura fixa. */
int  largura_utf8(const char *s);
void imprimir_coluna(const char *texto, int largura);

/* Exibição: resumo em uma linha e ficha completa. */
void imprimir_resumo(const Orientacao *o);
void imprimir_ficha(const Orientacao *o);

#endif
