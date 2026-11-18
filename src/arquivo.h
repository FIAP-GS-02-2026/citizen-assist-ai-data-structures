/*
 * arquivo.h - persistência da lista em arquivo texto (extensão opcional do enunciado).
 *
 * Formato: uma orientação por linha, campos separados por '|', em UTF-8:
 *   id|categoria|titulo|descricao|requisitos|local|fonte|verificado_em
 * Linhas vazias e linhas que começam com '#' são ignoradas.
 */
#ifndef ARQUIVO_H
#define ARQUIVO_H

#include "lista.h"

typedef struct {
    int carregados;
    int ignorados;      /* linhas com erro de formato, dado inválido ou ID repetido */
} ResumoCarga;

/* Carrega o arquivo para o fim da lista. Retorna 0 se abriu o arquivo, -1 se não existe. */
int arquivo_carregar(const char *caminho, Lista *l, ResumoCarga *resumo);

/* Grava a lista inteira (na ordem atual). Retorna o número de registros ou -1 em caso de erro. */
int arquivo_salvar(const char *caminho, const Lista *l);

#endif
