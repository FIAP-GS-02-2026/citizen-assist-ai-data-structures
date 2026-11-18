/*
 * testes.c - testes automatizados da lista encadeada, das buscas, das ordenações,
 * da validação de entrada e da persistência.
 *
 * Compilar e rodar (na raiz do projeto):
 *   gcc -std=c11 -Wall -Wextra -O2 -Isrc -o testes tests/testes.c src/lista.c src/orientacao.c src/arquivo.c src/entrada.c
 *   ./testes
 *
 * Cada linha mostra: grupo | caso | esperado | observado | OK/FALHOU.
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "arquivo.h"
#include "entrada.h"
#include "lista.h"
#include "orientacao.h"

static int total = 0, falhas = 0;

/* Imprime em coluna de largura fixa contando letras acentuadas como 1 caractere. */
static void coluna(const char *texto, int largura) {
    imprimir_coluna(texto, largura);
    printf(" | ");
}

static void verificar(const char *grupo, const char *caso, const char *esperado, const char *observado, int ok) {
    total++;
    if (!ok) falhas++;
    coluna(grupo, 9); coluna(caso, 52); coluna(esperado, 24); coluna(observado, 24);
    printf("%s\n", ok ? "OK" : "FALHOU");
}

static const char *nome_res(ResultadoLista r) {
    switch (r) {
        case LISTA_OK: return "OK";
        case LISTA_ID_DUPLICADO: return "ID_DUPLICADO";
        case LISTA_NAO_ENCONTRADO: return "NAO_ENCONTRADO";
        case LISTA_VAZIA: return "LISTA_VAZIA";
        case LISTA_SEM_MEMORIA: return "SEM_MEMORIA";
        default: return "DADO_INVALIDO";
    }
}

static void checar_res(const char *grupo, const char *caso, ResultadoLista esperado, ResultadoLista obtido) {
    verificar(grupo, caso, nome_res(esperado), nome_res(obtido), esperado == obtido);
}

static void checar_int(const char *grupo, const char *caso, long esperado, long obtido) {
    char e[32], o[32];
    snprintf(e, sizeof e, "%ld", esperado);
    snprintf(o, sizeof o, "%ld", obtido);
    verificar(grupo, caso, e, o, esperado == obtido);
}

static Orientacao nova(int id, const char *titulo, const char *categoria, const char *data) {
    Orientacao o;
    memset(&o, 0, sizeof o);
    o.id = id;
    copiar_texto(o.titulo, sizeof o.titulo, titulo);
    copiar_texto(o.categoria, sizeof o.categoria, categoria);
    copiar_texto(o.descricao, sizeof o.descricao, "Descrição de teste.");
    copiar_texto(o.local, sizeof o.local, "Posto de Atendimento");
    copiar_texto(o.fonte, sizeof o.fonte, "https://www.sptrans.com.br");
    copiar_texto(o.verificado_em, sizeof o.verificado_em, data);
    return o;
}

/* Sequência de IDs da lista, ex.: "3,1,2". */
static void ids(const Lista *l, char *buf, size_t tam) {
    No *p;
    size_t n = 0;
    buf[0] = '\0';
    for (p = l->inicio; p != NULL && n + 8 < tam; p = p->prox)
        n += (size_t)snprintf(buf + n, tam - n, n ? ",%d" : "%d", p->dado.id);
}

static void checar_ids(const char *grupo, const char *caso, const Lista *l, const char *esperado) {
    char obs[256];
    ids(l, obs, sizeof obs);
    verificar(grupo, caso, esperado, obs, strcmp(obs, esperado) == 0 && lista_integra(l));
}

/* ------------------------------------------------------------------ */

static void testes_lista_vazia(void) {
    Lista l;
    Orientacao o = nova(1, "Teste", "Tarifas", "2026-11-18");
    No *achados[4];
    lista_iniciar(&l);
    verificar("vazia", "lista nova está vazia", "vazia", lista_vazia(&l) ? "vazia" : "não vazia", lista_vazia(&l));
    verificar("vazia", "buscar ID 1 em lista vazia", "NULL", lista_buscar_id(&l, 1) ? "nó" : "NULL",
              lista_buscar_id(&l, 1) == NULL);
    checar_int("vazia", "pesquisar \"onibus\" em lista vazia", 0, lista_pesquisar(&l, BUSCA_TITULO, "onibus", achados, 4));
    checar_res("vazia", "remover ID 1 em lista vazia", LISTA_VAZIA, lista_remover(&l, 1));
    checar_res("vazia", "atualizar ID 1 em lista vazia", LISTA_VAZIA, lista_atualizar(&l, 1, &o));
    checar_int("vazia", "Merge Sort em lista vazia (comparações)", 0, lista_ordenar_merge(&l, POR_TITULO));
    checar_int("vazia", "Insertion Sort em lista vazia (comparações)", 0, lista_ordenar_insercao(&l, POR_TITULO));
    verificar("vazia", "início e fim continuam NULL", "NULL/NULL", (l.inicio || l.fim) ? "não NULL" : "NULL/NULL",
              l.inicio == NULL && l.fim == NULL);
    checar_int("vazia", "próximo ID sugerido", 1, lista_proximo_id(&l));
    lista_liberar(&l);
}

static void testes_insercao(void) {
    Lista l;
    Orientacao a = nova(10, "Como tirar o Bilhete Único Comum", "Bilhete Único", "2026-11-16");
    Orientacao b = nova(3, "Gratuidade no ônibus para pessoa idosa", "Gratuidades", "2026-11-17");
    Orientacao c = nova(7, "Domingão Tarifa Zero", "Tarifas", "2026-11-15");
    Orientacao x;
    lista_iniciar(&l);
    checar_res("inserir", "inserir ID 10 (lista vazia)", LISTA_OK, lista_inserir(&l, &a));
    verificar("inserir", "início e fim apontam para o mesmo nó", "inicio == fim", l.inicio == l.fim ? "inicio == fim" : "diferentes",
              l.inicio == l.fim && l.inicio != NULL);
    checar_res("inserir", "inserir ID 3", LISTA_OK, lista_inserir(&l, &b));
    checar_res("inserir", "inserir ID 7", LISTA_OK, lista_inserir(&l, &c));
    checar_ids("inserir", "ordem de inserção preservada no fim", &l, "10,3,7");
    checar_res("inserir", "ID 3 duplicado é recusado", LISTA_ID_DUPLICADO, lista_inserir(&l, &b));
    checar_int("inserir", "tamanho não muda após duplicado", 3, l.tamanho);
    x = nova(0, "Sem ID válido", "Tarifas", "2026-11-18");
    checar_res("inserir", "ID 0 (fora da faixa) é recusado", LISTA_DADO_INVALIDO, lista_inserir(&l, &x));
    x = nova(100000, "ID grande demais", "Tarifas", "2026-11-18");
    checar_res("inserir", "ID 100000 (fora da faixa) é recusado", LISTA_DADO_INVALIDO, lista_inserir(&l, &x));
    x = nova(20, "   ", "Tarifas", "2026-11-18");
    checar_res("inserir", "título só com espaços é recusado", LISTA_DADO_INVALIDO, lista_inserir(&l, &x));
    x = nova(21, "Data errada", "Tarifas", "2026-02-30");
    checar_res("inserir", "data 2026-02-30 é recusada", LISTA_DADO_INVALIDO, lista_inserir(&l, &x));
    x = nova(22, "Bissexto", "Tarifas", "2028-02-29");
    checar_res("inserir", "data 2028-02-29 (bissexto) é aceita", LISTA_OK, lista_inserir(&l, &x));
    checar_int("inserir", "próximo ID sugerido = maior + 1", 23, lista_proximo_id(&l));
    lista_liberar(&l);
}

static void montar(Lista *l) {
    Orientacao v[] = {
        nova(1, "Gratuidade no ônibus para pessoa idosa com 65 anos ou mais", "Gratuidades", "2026-11-16"),
        nova(2, "Domingão Tarifa Zero: ônibus de graça aos domingos", "Tarifas", "2026-11-16"),
        nova(3, "Como tirar o Bilhete Único Comum", "Bilhete Único", "2026-11-12"),
        nova(4, "Atende+: transporte porta a porta", "Acessibilidade", "2026-11-16"),
        nova(5, "Bilhete Único Mensal e de 24 horas", "Tarifas", "2026-11-10"),
    };
    size_t i;
    lista_iniciar(l);
    for (i = 0; i < sizeof v / sizeof v[0]; i++) lista_inserir(l, &v[i]);
}

static void testes_busca(void) {
    Lista l;
    No *achados[8];
    No *n;
    int k;
    montar(&l);
    n = lista_buscar_id(&l, 3);
    verificar("busca", "buscar ID 3 (existe)", "Como tirar o Bilhete...", n ? "encontrado" : "NULL",
              n != NULL && strcmp(n->dado.titulo, "Como tirar o Bilhete Único Comum") == 0);
    verificar("busca", "buscar ID 999 (não existe)", "NULL", lista_buscar_id(&l, 999) ? "nó" : "NULL",
              lista_buscar_id(&l, 999) == NULL);
    k = lista_pesquisar(&l, BUSCA_TITULO, "onibus", achados, 8);
    checar_int("busca", "título \"onibus\" sem acento acha \"ônibus\"", 2, k);
    k = lista_pesquisar(&l, BUSCA_TITULO, "BILHETE UNICO", achados, 8);
    checar_int("busca", "título \"BILHETE UNICO\" em maiúsculas", 2, k);
    k = lista_pesquisar(&l, BUSCA_CATEGORIA, "tarifa", achados, 8);
    checar_int("busca", "categoria \"tarifa\"", 2, k);
    verificar("busca", "resultados vêm na ordem da lista", "IDs 2 e 5",
              (k == 2 && achados[0]->dado.id == 2 && achados[1]->dado.id == 5) ? "IDs 2 e 5" : "outra ordem",
              k == 2 && achados[0]->dado.id == 2 && achados[1]->dado.id == 5);
    checar_int("busca", "termo inexistente \"metrô noturno\"", 0, lista_pesquisar(&l, BUSCA_TITULO, "metrô noturno", achados, 8));
    checar_int("busca", "termo vazio não devolve tudo", 0, lista_pesquisar(&l, BUSCA_TITULO, "", achados, 8));
    k = lista_pesquisar(&l, BUSCA_CATEGORIA, "a", achados, 2);
    checar_int("busca", "categorias com \"a\": conta 4 mesmo guardando só 2", 4, k);
    lista_liberar(&l);
}

static void testes_atualizar(void) {
    Lista l;
    Orientacao o;
    montar(&l);
    o = lista_buscar_id(&l, 2)->dado;
    copiar_texto(o.titulo, sizeof o.titulo, "Tarifa Zero aos domingos (atualizado)");
    o.id = 77;                                       /* tentativa de trocar o ID deve ser ignorada */
    checar_res("atualizar", "atualizar título do ID 2", LISTA_OK, lista_atualizar(&l, 2, &o));
    verificar("atualizar", "novo título gravado e ID mantido", "ID 2 atualizado",
              lista_buscar_id(&l, 2) && strstr(lista_buscar_id(&l, 2)->dado.titulo, "atualizado") ? "ID 2 atualizado" : "não",
              lista_buscar_id(&l, 2) != NULL && lista_buscar_id(&l, 77) == NULL);
    checar_res("atualizar", "atualizar ID 999 (não existe)", LISTA_NAO_ENCONTRADO, lista_atualizar(&l, 999, &o));
    copiar_texto(o.verificado_em, sizeof o.verificado_em, "18/11/2026");
    checar_res("atualizar", "data em formato errado é recusada", LISTA_DADO_INVALIDO, lista_atualizar(&l, 2, &o));
    verificar("atualizar", "registro original preservado após recusa", "2026-11-16",
              lista_buscar_id(&l, 2)->dado.verificado_em, strcmp(lista_buscar_id(&l, 2)->dado.verificado_em, "2026-11-16") == 0);
    checar_ids("atualizar", "posição dos nós não muda", &l, "1,2,3,4,5");
    lista_liberar(&l);
}

static void testes_remover(void) {
    Lista l;
    montar(&l);
    checar_res("remover", "remover ID 3 (meio)", LISTA_OK, lista_remover(&l, 3));
    checar_ids("remover", "vizinhos religados", &l, "1,2,4,5");
    checar_res("remover", "remover ID 1 (início)", LISTA_OK, lista_remover(&l, 1));
    checar_ids("remover", "início avança", &l, "2,4,5");
    checar_res("remover", "remover ID 5 (fim)", LISTA_OK, lista_remover(&l, 5));
    checar_ids("remover", "fim volta para o anterior", &l, "2,4");
    checar_res("remover", "remover ID 3 de novo (não existe)", LISTA_NAO_ENCONTRADO, lista_remover(&l, 3));
    checar_int("remover", "tamanho após 3 remoções", 2, l.tamanho);
    lista_remover(&l, 2);
    lista_remover(&l, 4);
    verificar("remover", "remover todos deixa início/fim NULL", "NULL/NULL", (l.inicio || l.fim) ? "não NULL" : "NULL/NULL",
              l.inicio == NULL && l.fim == NULL && l.tamanho == 0);
    {
        Orientacao o = nova(9, "Reinserir após esvaziar", "Tarifas", "2026-11-18");
        checar_res("remover", "inserir de novo após esvaziar", LISTA_OK, lista_inserir(&l, &o));
        checar_ids("remover", "lista com 1 nó íntegra", &l, "9");
    }
    lista_liberar(&l);
}

static void testes_ordenacao(void) {
    Lista l;
    long c;
    montar(&l);
    lista_ordenar_merge(&l, POR_TITULO);
    checar_ids("ordenar", "Merge Sort por título (A-Z, sem acento)", &l, "4,5,3,2,1");
    lista_ordenar_merge(&l, POR_ID);
    checar_ids("ordenar", "Merge Sort por ID", &l, "1,2,3,4,5");
    lista_ordenar_merge(&l, POR_CATEGORIA);
    checar_ids("ordenar", "Merge Sort por categoria e título", &l, "4,3,1,5,2");
    lista_ordenar_merge(&l, POR_VERIFICACAO);
    checar_ids("ordenar", "Merge Sort por verificação (empate → ID)", &l, "5,3,1,2,4");
    lista_ordenar_insercao(&l, POR_TITULO);
    checar_ids("ordenar", "Insertion Sort por título", &l, "4,5,3,2,1");
    lista_ordenar_insercao(&l, POR_ID);
    checar_ids("ordenar", "Insertion Sort por ID", &l, "1,2,3,4,5");
    c = lista_ordenar_insercao(&l, POR_ID);
    checar_int("ordenar", "Insertion já ordenada: n-1 comparações", 4, c);
    lista_ordenar_merge(&l, POR_ID);
    {
        /* inverte por ID decrescente usando inserção no início: monta lista 5..1 */
        Lista inv;
        No *p;
        lista_iniciar(&inv);
        for (p = l.inicio; p; p = p->prox) {
            No *novo = (No *)malloc(sizeof(No));
            novo->dado = p->dado;
            novo->prox = inv.inicio;
            inv.inicio = novo;
            if (!inv.fim) inv.fim = novo;
            inv.tamanho++;
        }
        c = lista_ordenar_insercao(&inv, POR_ID);
        checar_int("ordenar", "Insertion em ordem inversa: 2(n-1) comparações", 8, c);
        checar_ids("ordenar", "resultado da ordem inversa", &inv, "1,2,3,4,5");
        lista_liberar(&inv);
    }
    lista_liberar(&l);
    {
        /* estabilidade: mesma categoria mantém a ordem de chegada quando o título empata */
        Lista e;
        Orientacao a = nova(8, "Mesmo título", "Tarifas", "2026-11-18");
        Orientacao b = nova(6, "Mesmo título", "Tarifas", "2026-11-18");
        lista_iniciar(&e);
        lista_inserir(&e, &a);
        lista_inserir(&e, &b);
        lista_ordenar_merge(&e, POR_TITULO);
        checar_ids("ordenar", "Merge Sort é estável (empate mantém 8,6)", &e, "8,6");
        lista_ordenar_insercao(&e, POR_TITULO);
        checar_ids("ordenar", "Insertion Sort é estável (empate mantém 8,6)", &e, "8,6");
        lista_liberar(&e);
    }
}

static void testes_entrada(void) {
    int v = -1;
    checar_int("entrada", "\"12\" vira 12", 1, texto_para_inteiro("12", 0, 99, &v) && v == 12);
    checar_int("entrada", "\"  7 \" com espaços vira 7", 1, texto_para_inteiro("  7 ", 0, 99, &v) && v == 7);
    checar_int("entrada", "\"abc\" é recusado", 0, texto_para_inteiro("abc", 0, 99, &v));
    checar_int("entrada", "\"12a\" é recusado", 0, texto_para_inteiro("12a", 0, 99, &v));
    checar_int("entrada", "\"1.5\" é recusado", 0, texto_para_inteiro("1.5", 0, 99, &v));
    checar_int("entrada", "texto vazio é recusado", 0, texto_para_inteiro("", 0, 99, &v));
    checar_int("entrada", "\"10\" fora da faixa 0-9 é recusado", 0, texto_para_inteiro("10", 0, 9, &v));
    checar_int("entrada", "\"-1\" fora da faixa é recusado", 0, texto_para_inteiro("-1", 0, 9, &v));
    checar_int("entrada", "\"99999999999\" (estouro) é recusado", 0, texto_para_inteiro("99999999999", 0, 99999, &v));
    {
        char s[64];
        normalizar("  Ônibus ÚNICO à Noite  ", s, sizeof s);
        verificar("entrada", "normalizar acentos e maiúsculas", "onibus unico a noite", s, strcmp(s, "onibus unico a noite") == 0);
        copiar_texto(s, 6, "Ônibus");             /* 'Ô' tem 2 bytes: não pode cortar no meio */
        verificar("entrada", "copiar_texto não corta letra UTF-8", "Ônib", s, strcmp(s, "Ônib") == 0);
    }
}

static void testes_arquivo(void) {
    Lista l, r;
    ResumoCarga carga;
    FILE *f;
    montar(&l);
    checar_int("arquivo", "salvar 5 registros", 5, arquivo_salvar("teste_base.txt", &l));
    lista_iniciar(&r);
    arquivo_carregar("teste_base.txt", &r, &carga);
    checar_int("arquivo", "recarregar os 5 registros", 5, carga.carregados);
    checar_ids("arquivo", "mesma ordem depois de recarregar", &r, "1,2,3,4,5");
    lista_liberar(&r);

    f = fopen("teste_ruim.txt", "w");
    fprintf(f, "# comentário\n\n");
    fprintf(f, "1|Tarifas|Válido|Desc|Req|Local|https://x|2026-11-18\n");
    fprintf(f, "1|Tarifas|ID repetido|Desc|Req|Local|https://x|2026-11-18\n");
    fprintf(f, "abc|Tarifas|ID com letras|Desc|Req|Local|https://x|2026-11-18\n");
    fprintf(f, "2|Tarifas|Campos a menos|Desc\n");
    fprintf(f, "3|Tarifas|Campos a mais|Desc|Req|Local|https://x|2026-11-18|extra\n");
    fprintf(f, "4|Tarifas|Data inválida|Desc|Req|Local|https://x|2026-13-01\n");
    fclose(f);
    lista_iniciar(&r);
    arquivo_carregar("teste_ruim.txt", &r, &carga);
    checar_int("arquivo", "arquivo com erros: carrega só a linha válida", 1, carga.carregados);
    checar_int("arquivo", "arquivo com erros: 5 linhas ignoradas", 5, carga.ignorados);
    lista_liberar(&r);
    lista_iniciar(&r);
    checar_int("arquivo", "arquivo inexistente retorna -1", -1, arquivo_carregar("nao_existe.txt", &r, &carga));
    lista_liberar(&r);
    lista_liberar(&l);
    remove("teste_base.txt");
    remove("teste_ruim.txt");
}

/* Mede comparações de cada algoritmo para mostrar o crescimento previsto pelo Big-O. */
static void medir_crescimento(void) {
    int tamanhos[] = {250, 500, 1000, 2000, 4000};
    size_t t;
    unsigned semente = 12345;
    printf("\nCrescimento medido (registros em ordem aleatória, critério: ID)\n");
    printf("%8s | %14s | %16s | %14s | %18s\n", "n", "Merge (comp.)", "n*teto(log2 n)", "Insertion", "n^2/4 (caso medio)");
    for (t = 0; t < sizeof tamanhos / sizeof tamanhos[0]; t++) {
        int n = tamanhos[t], i, k = 0;
        Lista a, b;
        long cm, ci;
        int *perm = (int *)malloc(sizeof(int) * (size_t)n);
        for (i = 0; i < n; i++) perm[i] = i + 1;
        for (i = n - 1; i > 0; i--) {                       /* embaralha (Fisher-Yates) */
            int j, tmp;
            semente = semente * 1103515245u + 12345u;
            j = (int)((semente >> 8) % (unsigned)(i + 1));
            tmp = perm[i]; perm[i] = perm[j]; perm[j] = tmp;
        }
        lista_iniciar(&a);
        lista_iniciar(&b);
        for (i = 0; i < n; i++) {
            Orientacao o = nova(perm[i], "Registro", "Tarifas", "2026-11-18");
            lista_inserir(&a, &o);
            lista_inserir(&b, &o);
        }
        cm = lista_ordenar_merge(&a, POR_ID);
        ci = lista_ordenar_insercao(&b, POR_ID);
        while ((1 << k) < n) k++;                            /* k = teto(log2 n) */
        printf("%8d | %14ld | %16.0f | %14ld | %18.0f\n", n, cm, (double)n * k, ci, (double)n * n / 4.0);
        lista_liberar(&a);
        lista_liberar(&b);
        free(perm);
    }
}

int main(void) {
    printf("Testes automatizados · Citizen Assist AI Mobilidade (DSA)\n");
    coluna("grupo", 9); coluna("caso", 52); coluna("esperado", 24); coluna("observado", 24);
    printf("resultado\n");
    testes_lista_vazia();
    testes_insercao();
    testes_busca();
    testes_atualizar();
    testes_remover();
    testes_ordenacao();
    testes_entrada();
    testes_arquivo();
    printf("\n%d testes, %d falha(s).\n", total, falhas);
    medir_crescimento();
    return falhas ? 1 : 0;
}
