/*
 * lista.h - lista simplesmente encadeada de orientações.
 *
 *   inicio -> [No | prox] -> [No | prox] -> ... -> [No | NULL] <- fim
 *
 * Cada nó é alocado com malloc ao inserir e liberado com free ao remover.
 * O ponteiro "fim" permite inserir no final em O(1).
 */
#ifndef LISTA_H
#define LISTA_H

#include "orientacao.h"

typedef struct No {
    Orientacao dado;
    struct No *prox;
} No;

typedef struct {
    No *inicio;
    No *fim;
    int tamanho;
} Lista;

typedef enum {
    LISTA_OK = 0,
    LISTA_ID_DUPLICADO,
    LISTA_NAO_ENCONTRADO,
    LISTA_VAZIA,
    LISTA_SEM_MEMORIA,
    LISTA_DADO_INVALIDO
} ResultadoLista;

typedef enum {
    POR_TITULO = 1,
    POR_CATEGORIA,      /* categoria e, dentro dela, título */
    POR_ID,
    POR_VERIFICACAO     /* data de verificação mais antiga primeiro */
} CriterioOrdem;

typedef enum { BUSCA_TITULO = 1, BUSCA_CATEGORIA } CampoBusca;

void lista_iniciar(Lista *l);
void lista_liberar(Lista *l);
int  lista_vazia(const Lista *l);

/* Inserção no fim (O(n) pela checagem de ID duplicado; o encadeamento em si é O(1)). */
ResultadoLista lista_inserir(Lista *l, const Orientacao *o);

/* Busca sequencial pelo ID. Retorna o nó ou NULL. */
No *lista_buscar_id(const Lista *l, int id);

/* Busca sequencial por trecho do título ou da categoria (sem acento e sem maiúsculas).
 * Guarda até "max" ponteiros em "achados" e retorna quantos encontrou no total. */
int lista_pesquisar(const Lista *l, CampoBusca campo, const char *termo, No **achados, int max);

/* Atualiza os campos do registro de ID "id" (o ID não muda). */
ResultadoLista lista_atualizar(Lista *l, int id, const Orientacao *novo);

/* Remove o nó de ID "id", religa os vizinhos e libera a memória. */
ResultadoLista lista_remover(Lista *l, int id);

/* Merge Sort sobre a própria lista (troca os ponteiros, não copia registros).
 * Retorna o número de comparações feitas, para mostrar o custo na prática. */
long lista_ordenar_merge(Lista *l, CriterioOrdem criterio);

/* Insertion Sort sobre a própria lista, para comparar com o Merge Sort. */
long lista_ordenar_insercao(Lista *l, CriterioOrdem criterio);

/* Compara dois registros pelo critério (negativo, zero ou positivo). */
int comparar_por(const Orientacao *a, const Orientacao *b, CriterioOrdem criterio);

/* Maior ID existente + 1 (sugestão para o próximo cadastro). */
int lista_proximo_id(const Lista *l);

/* Confere a integridade: tamanho, ponteiro fim e ausência de IDs repetidos. */
int lista_integra(const Lista *l);

#endif
