#include "lista.h"

#include <stdlib.h>
#include <string.h>

void lista_iniciar(Lista *l) {
    l->inicio = NULL;
    l->fim = NULL;
    l->tamanho = 0;
}

void lista_liberar(Lista *l) {
    No *atual = l->inicio;
    while (atual != NULL) {
        No *proximo = atual->prox;
        free(atual);
        atual = proximo;
    }
    lista_iniciar(l);
}

int lista_vazia(const Lista *l) {
    return l->inicio == NULL;
}

No *lista_buscar_id(const Lista *l, int id) {
    No *atual;
    for (atual = l->inicio; atual != NULL; atual = atual->prox) {
        if (atual->dado.id == id) return atual;
    }
    return NULL;
}

ResultadoLista lista_inserir(Lista *l, const Orientacao *o) {
    No *novo;
    if (validar_orientacao(o) != NULL) return LISTA_DADO_INVALIDO;
    if (lista_buscar_id(l, o->id) != NULL) return LISTA_ID_DUPLICADO;   /* O(n) */

    novo = (No *)malloc(sizeof(No));
    if (novo == NULL) return LISTA_SEM_MEMORIA;
    novo->dado = *o;
    novo->prox = NULL;

    if (l->fim == NULL) {          /* lista vazia: o novo nó é início e fim */
        l->inicio = novo;
    } else {
        l->fim->prox = novo;       /* O(1) graças ao ponteiro fim */
    }
    l->fim = novo;
    l->tamanho++;
    return LISTA_OK;
}

int lista_pesquisar(const Lista *l, CampoBusca campo, const char *termo, No **achados, int max) {
    int total = 0;
    No *atual;
    for (atual = l->inicio; atual != NULL; atual = atual->prox) {
        const char *alvo = (campo == BUSCA_TITULO) ? atual->dado.titulo : atual->dado.categoria;
        if (contem_normalizado(alvo, termo)) {
            if (total < max) achados[total] = atual;
            total++;
        }
    }
    return total;
}

ResultadoLista lista_atualizar(Lista *l, int id, const Orientacao *novo) {
    No *no;
    Orientacao copia;
    if (lista_vazia(l)) return LISTA_VAZIA;
    no = lista_buscar_id(l, id);
    if (no == NULL) return LISTA_NAO_ENCONTRADO;
    copia = *novo;
    copia.id = id;                       /* o identificador não muda */
    if (validar_orientacao(&copia) != NULL) return LISTA_DADO_INVALIDO;
    no->dado = copia;                    /* os ponteiros ficam intactos */
    return LISTA_OK;
}

ResultadoLista lista_remover(Lista *l, int id) {
    No *anterior = NULL;
    No *atual = l->inicio;
    if (lista_vazia(l)) return LISTA_VAZIA;

    while (atual != NULL && atual->dado.id != id) {
        anterior = atual;
        atual = atual->prox;
    }
    if (atual == NULL) return LISTA_NAO_ENCONTRADO;

    if (anterior == NULL) l->inicio = atual->prox;   /* era o primeiro */
    else                  anterior->prox = atual->prox;
    if (atual == l->fim)  l->fim = anterior;          /* era o último */

    free(atual);
    l->tamanho--;
    return LISTA_OK;
}

int comparar_por(const Orientacao *a, const Orientacao *b, CriterioOrdem criterio) {
    int c;
    switch (criterio) {
        case POR_CATEGORIA:
            c = comparar_normalizado(a->categoria, b->categoria);
            if (c != 0) return c;
            return comparar_normalizado(a->titulo, b->titulo);
        case POR_ID:
            return (a->id > b->id) - (a->id < b->id);
        case POR_VERIFICACAO:
            c = strcmp(a->verificado_em, b->verificado_em);   /* AAAA-MM-DD ordena como texto */
            if (c != 0) return c;
            return (a->id > b->id) - (a->id < b->id);
        case POR_TITULO:
        default:
            return comparar_normalizado(a->titulo, b->titulo);
    }
}

/* ------------------------------------------------------------ Merge Sort */

/* Divide a lista ao meio com dois ponteiros (lento anda 1, rápido anda 2). */
static No *dividir(No *cabeca) {
    No *lento = cabeca, *rapido = cabeca->prox, *segunda;
    while (rapido != NULL && rapido->prox != NULL) {
        lento = lento->prox;
        rapido = rapido->prox->prox;
    }
    segunda = lento->prox;
    lento->prox = NULL;
    return segunda;
}

/* Intercala duas listas já ordenadas. "<=" mantém a ordem original dos empates (estável). */
static No *intercalar(No *a, No *b, CriterioOrdem criterio, long *comparacoes) {
    No sentinela;
    No *cauda = &sentinela;
    sentinela.prox = NULL;
    while (a != NULL && b != NULL) {
        (*comparacoes)++;
        if (comparar_por(&a->dado, &b->dado, criterio) <= 0) {
            cauda->prox = a; a = a->prox;
        } else {
            cauda->prox = b; b = b->prox;
        }
        cauda = cauda->prox;
    }
    cauda->prox = (a != NULL) ? a : b;
    return sentinela.prox;
}

static No *merge_sort(No *cabeca, CriterioOrdem criterio, long *comparacoes) {
    No *segunda;
    if (cabeca == NULL || cabeca->prox == NULL) return cabeca;   /* 0 ou 1 nó: já ordenado */
    segunda = dividir(cabeca);
    cabeca = merge_sort(cabeca, criterio, comparacoes);
    segunda = merge_sort(segunda, criterio, comparacoes);
    return intercalar(cabeca, segunda, criterio, comparacoes);
}

static void refazer_fim(Lista *l) {
    No *p = l->inicio;
    l->fim = NULL;
    while (p != NULL) { l->fim = p; p = p->prox; }
}

long lista_ordenar_merge(Lista *l, CriterioOrdem criterio) {
    long comparacoes = 0;
    l->inicio = merge_sort(l->inicio, criterio, &comparacoes);
    refazer_fim(l);
    return comparacoes;
}

/* ------------------------------------------------------------ Insertion Sort */

long lista_ordenar_insercao(Lista *l, CriterioOrdem criterio) {
    long comparacoes = 0;
    No *ordenada = NULL;          /* nova lista, montada já em ordem */
    No *cauda = NULL;             /* último nó da lista ordenada */
    No *atual = l->inicio;
    while (atual != NULL) {
        No *proximo = atual->prox;
        atual->prox = NULL;
        if (ordenada == NULL) {
            ordenada = cauda = atual;                     /* primeiro nó */
            atual = proximo;
            continue;
        }
        /* maior ou igual ao último: vai para o fim (entrada já ordenada custa 1 comparação) */
        comparacoes++;
        if (comparar_por(&atual->dado, &cauda->dado, criterio) >= 0) {
            cauda->prox = atual;
            cauda = atual;
            atual = proximo;
            continue;
        }
        /* menor que o primeiro: vai para o início */
        comparacoes++;
        if (comparar_por(&atual->dado, &ordenada->dado, criterio) < 0) {
            atual->prox = ordenada;
            ordenada = atual;
        } else {
            /* posição do meio: avança enquanto o próximo for <= atual (mantém a estabilidade) */
            No *p = ordenada;
            for (;;) {
                comparacoes++;
                if (comparar_por(&atual->dado, &p->prox->dado, criterio) < 0) break;
                p = p->prox;
            }
            atual->prox = p->prox;
            p->prox = atual;
        }
        atual = proximo;
    }
    l->inicio = ordenada;
    l->fim = cauda;
    return comparacoes;
}

int lista_proximo_id(const Lista *l) {
    int maior = 0;
    No *p;
    for (p = l->inicio; p != NULL; p = p->prox)
        if (p->dado.id > maior) maior = p->dado.id;
    return (maior < ID_MAXIMO) ? maior + 1 : -1;
}

int lista_integra(const Lista *l) {
    int n = 0;
    No *p, *q, *ultimo = NULL;
    for (p = l->inicio; p != NULL; p = p->prox) {
        for (q = p->prox; q != NULL; q = q->prox)
            if (q->dado.id == p->dado.id) return 0;
        ultimo = p;
        n++;
    }
    return n == l->tamanho && ultimo == l->fim;
}
