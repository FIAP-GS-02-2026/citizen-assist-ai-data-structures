/*
 * entrada.h - leitura segura do teclado (nada de scanf solto).
 *
 * Todas as funções retornam:
 *    1  leitura válida
 *    0  entrada inválida (a mensagem já foi exibida)
 *   -1  fim da entrada (Ctrl+D / Ctrl+Z): o programa encerra com segurança
 */
#ifndef ENTRADA_H
#define ENTRADA_H

#include <stddef.h>

int ler_linha(const char *rotulo, char *destino, size_t tamanho);
int ler_inteiro(const char *rotulo, int minimo, int maximo, int *valor);
int ler_sim_nao(const char *rotulo, int *sim);

/* Converte texto em inteiro aceitando apenas dígitos (com espaços nas pontas). */
int texto_para_inteiro(const char *texto, int minimo, int maximo, int *valor);

#endif
