/*
 * Citizen Assist AI Mobilidade - cadastro de orientações em lista encadeada
 * Global Solution 2026.2 · FIAP · 1CC-FEV · Data Structures and Algorithms
 *
 * Integrantes: Gabriel Fagundes (RM 569074), Glauco Kelly (RM 572840),
 *              Thiago Renatino (RM 569073)
 *
 * Compilar:  gcc -std=c11 -Wall -Wextra -O2 -o citizen_assist src/main.c src/lista.c \
 *             src/orientacao.c src/arquivo.c src/entrada.c
 * Executar:  ./citizen_assist            (usa dados/orientacoes.txt)
 *            ./citizen_assist outro.txt  (usa outro arquivo)
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#ifdef _WIN32
#include <windows.h>
#endif

#include "arquivo.h"
#include "entrada.h"
#include "lista.h"
#include "orientacao.h"

#define ARQUIVO_PADRAO "dados/orientacoes.txt"
#define MAX_CATEGORIAS 32
#define MAX_RESULTADOS 64
#define DIAS_RECONFERIR 30

typedef struct {
    Lista lista;
    const char *arquivo;
    int alterado;           /* há mudanças ainda não salvas */
} Sistema;

/* Sinaliza fim da entrada (Ctrl+D / Ctrl+Z) para encerrar o menu com segurança. */
static int fim_entrada = 0;
#define CHECA(r) do { if ((r) < 0) { fim_entrada = 1; return; } } while (0)

static void titulo_secao(const char *t) {
    printf("\n== %s ==\n", t);
}

/* ------------------------------------------------------------ categorias */

static int listar_categorias(const Lista *l, char cats[][TAM_CATEGORIA], int qtd[], int max) {
    int n = 0, i;
    No *p;
    for (p = l->inicio; p != NULL; p = p->prox) {
        for (i = 0; i < n; i++)
            if (comparar_normalizado(cats[i], p->dado.categoria) == 0) break;
        if (i == n) {
            if (n == max) continue;
            copiar_texto(cats[n], TAM_CATEGORIA, p->dado.categoria);
            qtd[n++] = 0;
        }
        qtd[i]++;
    }
    return n;
}

/* Lê a categoria: número de uma categoria existente ou o nome de uma nova. */
static int ler_categoria(const Lista *l, char *destino, const char *atual) {
    char cats[MAX_CATEGORIAS][TAM_CATEGORIA];
    int qtd[MAX_CATEGORIAS];
    int n = listar_categorias(l, cats, qtd, MAX_CATEGORIAS), i, escolha, r;
    char buf[TAM_CATEGORIA];

    printf("  Categorias existentes:%s", n == 0 ? " (nenhuma ainda)" : "");
    for (i = 0; i < n; i++) printf("%s %d) %s", i % 3 == 0 ? "\n   " : "  ", i + 1, cats[i]);
    printf("\n");
    for (;;) {
        r = ler_linha(atual ? "  Categoria (número, nome novo ou Enter para manter): "
                            : "  Categoria (número ou nome de uma nova): ", buf, sizeof buf);
        if (r < 0) return -1;
        if (r == 0) continue;
        if (buf[0] == '\0') {
            if (atual) { copiar_texto(destino, TAM_CATEGORIA, atual); return 1; }
            printf("  ! A categoria é obrigatória.\n");
            continue;
        }
        if (texto_para_inteiro(buf, 1, 1000, &escolha)) {
            if (escolha > n) { printf("  ! Não existe a categoria %d.\n", escolha); continue; }
            copiar_texto(destino, TAM_CATEGORIA, cats[escolha - 1]);
            return 1;
        }
        for (i = 0; i < n; i++)              /* nome igual a uma existente: usa a grafia já cadastrada */
            if (comparar_normalizado(cats[i], buf) == 0) { copiar_texto(destino, TAM_CATEGORIA, cats[i]); return 1; }
        copiar_texto(destino, TAM_CATEGORIA, buf);
        printf("  Nova categoria criada: %s\n", destino);
        return 1;
    }
}

/* Lê um campo de texto. Obrigatório: repete até ter valor. Com "atual", Enter mantém o valor. */
static int ler_campo(const char *nome, char *destino, size_t tamanho, int obrigatorio, const char *atual) {
    char rotulo[160];
    char *buf = (char *)malloc(tamanho);
    int r;
    if (buf == NULL) return -1;
    snprintf(rotulo, sizeof rotulo, atual ? "  %s (Enter mantém): " : "  %s: ", nome);
    for (;;) {
        r = ler_linha(rotulo, buf, tamanho);
        if (r < 0) { free(buf); return -1; }
        if (r == 0) continue;
        if (buf[0] == '\0' && atual) { copiar_texto(destino, tamanho, atual); break; }
        if (buf[0] == '\0' && obrigatorio) { printf("  ! Este campo é obrigatório.\n"); continue; }
        copiar_texto(destino, tamanho, buf);
        break;
    }
    free(buf);
    return 1;
}

static int ler_data(char *destino, const char *atual) {
    char buf[32];
    char hoje[TAM_DATA];
    time_t t = time(NULL);
    int r;
    strftime(hoje, sizeof hoje, "%Y-%m-%d", localtime(&t));
    for (;;) {
        char rotulo[96];
        snprintf(rotulo, sizeof rotulo, "  Verificado em AAAA-MM-DD (Enter = %s): ", atual ? atual : hoje);
        r = ler_linha(rotulo, buf, sizeof buf);
        if (r < 0) return -1;
        if (r == 0) continue;
        if (buf[0] == '\0') { copiar_texto(destino, TAM_DATA, atual ? atual : hoje); return 1; }
        if (data_valida(buf)) { copiar_texto(destino, TAM_DATA, buf); return 1; }
        printf("  ! Data inválida. Exemplo: 2026-11-18.\n");
    }
}

/* Preenche os campos de texto. Se "atual" não for NULL, Enter mantém cada valor. */
static int ler_campos(const Lista *l, Orientacao *o, const Orientacao *atual) {
    if (ler_campo("Título", o->titulo, sizeof o->titulo, 1, atual ? atual->titulo : NULL) < 0) return -1;
    if (ler_categoria(l, o->categoria, atual ? atual->categoria : NULL) < 0) return -1;
    if (ler_campo("Descrição (a orientação ao cidadão)", o->descricao, sizeof o->descricao, 1,
                  atual ? atual->descricao : NULL) < 0) return -1;
    if (ler_campo("Documentos/requisitos (opcional, separe com ;)", o->requisitos, sizeof o->requisitos, 0,
                  atual ? atual->requisitos : NULL) < 0) return -1;
    if (ler_campo("Onde resolver", o->local, sizeof o->local, 1, atual ? atual->local : NULL) < 0) return -1;
    if (ler_campo("Fonte oficial (URL)", o->fonte, sizeof o->fonte, 1, atual ? atual->fonte : NULL) < 0) return -1;
    if (ler_data(o->verificado_em, atual ? atual->verificado_em : NULL) < 0) return -1;
    return 1;
}

static int pedir_id(const char *rotulo, int *id) {
    int r;
    do {
        r = ler_inteiro(rotulo, ID_MINIMO, ID_MAXIMO, id);
        if (r < 0) return -1;
    } while (r == 0);
    return 1;
}

static int avisar_lista_vazia(const Sistema *s) {
    if (lista_vazia(&s->lista)) {
        printf("  A lista está vazia. Cadastre uma orientação pela opção 1.\n");
        return 1;
    }
    return 0;
}

/* ------------------------------------------------------------ opções do menu */

static void op_cadastrar(Sistema *s) {
    Orientacao o;
    char rotulo[96], buf[32];
    int sugestao = lista_proximo_id(&s->lista), r;
    ResultadoLista res;

    titulo_secao("Cadastrar orientação");
    memset(&o, 0, sizeof o);
    for (;;) {
        snprintf(rotulo, sizeof rotulo, "  ID (1 a %d, Enter = %d): ", ID_MAXIMO, sugestao);
        r = ler_linha(rotulo, buf, sizeof buf);
        CHECA(r);
        if (r == 0) continue;
        if (buf[0] == '\0' && sugestao > 0) o.id = sugestao;
        else if (!texto_para_inteiro(buf, ID_MINIMO, ID_MAXIMO, &o.id)) {
            printf("  ! ID inválido: use um número inteiro de 1 a %d.\n", ID_MAXIMO);
            continue;
        }
        if (lista_buscar_id(&s->lista, o.id) != NULL) {
            printf("  ! Já existe uma orientação com o ID %d. Escolha outro.\n", o.id);
            continue;
        }
        break;
    }
    CHECA(ler_campos(&s->lista, &o, NULL));
    res = lista_inserir(&s->lista, &o);
    if (res == LISTA_OK) {
        s->alterado = 1;
        printf("  Orientação %d cadastrada. Total: %d.\n", o.id, s->lista.tamanho);
    } else if (res == LISTA_SEM_MEMORIA) {
        printf("  ! Memória insuficiente para cadastrar.\n");
    } else {
        printf("  ! Não foi possível cadastrar: %s\n", validar_orientacao(&o) ? validar_orientacao(&o) : "ID duplicado.");
    }
}

static void op_consultar(Sistema *s) {
    int id;
    No *no;
    titulo_secao("Consultar por ID");
    if (avisar_lista_vazia(s)) return;
    CHECA(pedir_id("  ID: ", &id));
    no = lista_buscar_id(&s->lista, id);
    if (no == NULL) printf("  Nenhuma orientação com o ID %d.\n", id);
    else imprimir_ficha(&no->dado);
}

static void op_pesquisar(Sistema *s) {
    int campo, total, i, r, mostrar;
    char termo[TAM_TITULO];
    No *achados[MAX_RESULTADOS];

    titulo_secao("Pesquisar");
    if (avisar_lista_vazia(s)) return;
    printf("  1) Por título   2) Por categoria\n");
    do { r = ler_inteiro("  Campo: ", 1, 2, &campo); CHECA(r); } while (r == 0);
    do {
        r = ler_linha("  Termo (parte do texto, sem precisar de acento): ", termo, sizeof termo);
        CHECA(r);
        if (r == 1 && termo[0] == '\0') { printf("  ! Digite ao menos uma letra.\n"); r = 0; }
    } while (r == 0);

    total = lista_pesquisar(&s->lista, (CampoBusca)campo, termo, achados, MAX_RESULTADOS);
    if (total == 0) {
        char cats[MAX_CATEGORIAS][TAM_CATEGORIA];
        int qtd[MAX_CATEGORIAS], n = listar_categorias(&s->lista, cats, qtd, MAX_CATEGORIAS);
        printf("  Nenhuma orientação encontrada para \"%s\".\n  Categorias disponíveis:", termo);
        for (i = 0; i < n; i++) printf("%s %s", i ? "," : "", cats[i]);
        printf(".\n");
        return;
    }
    mostrar = total < MAX_RESULTADOS ? total : MAX_RESULTADOS;
    printf("  %d resultado(s), comparando %d registro(s) em busca sequencial:\n", total, s->lista.tamanho);
    for (i = 0; i < mostrar; i++) imprimir_resumo(&achados[i]->dado);
    if (total == 1) imprimir_ficha(&achados[0]->dado);
}

static void op_atualizar(Sistema *s) {
    int id;
    No *no;
    Orientacao novo;
    titulo_secao("Atualizar orientação");
    if (avisar_lista_vazia(s)) return;
    CHECA(pedir_id("  ID: ", &id));
    no = lista_buscar_id(&s->lista, id);
    if (no == NULL) { printf("  Nenhuma orientação com o ID %d.\n", id); return; }
    imprimir_ficha(&no->dado);
    printf("\n  Digite o novo valor de cada campo ou só Enter para manter.\n");
    novo = no->dado;
    CHECA(ler_campos(&s->lista, &novo, &no->dado));
    if (lista_atualizar(&s->lista, id, &novo) == LISTA_OK) {
        s->alterado = 1;
        printf("  Orientação %d atualizada.\n", id);
    } else {
        printf("  ! Não foi possível atualizar: %s\n", validar_orientacao(&novo));
    }
}

static void op_excluir(Sistema *s) {
    int id, sim = 0, r;
    No *no;
    titulo_secao("Excluir orientação");
    if (avisar_lista_vazia(s)) return;
    CHECA(pedir_id("  ID: ", &id));
    no = lista_buscar_id(&s->lista, id);
    if (no == NULL) { printf("  Nenhuma orientação com o ID %d.\n", id); return; }
    imprimir_resumo(&no->dado);
    do { r = ler_sim_nao("  Confirma a exclusão? (s/n): ", &sim); CHECA(r); } while (r == 0);
    if (!sim) { printf("  Exclusão cancelada.\n"); return; }
    if (lista_remover(&s->lista, id) == LISTA_OK) {
        s->alterado = 1;
        printf("  Orientação %d excluída. Total: %d.\n", id, s->lista.tamanho);
    }
}

static void listar(const Lista *l) {
    No *p;
    for (p = l->inicio; p != NULL; p = p->prox) imprimir_resumo(&p->dado);
}

static void op_ordenar(Sistema *s) {
    static const char *nomes[] = {"", "título (A-Z)", "categoria e título", "ID", "data de verificação (mais antiga primeiro)"};
    int criterio, alg, r, n;
    long comp;
    clock_t t0;
    double ms;

    titulo_secao("Ordenar e exibir");
    if (avisar_lista_vazia(s)) return;
    printf("  Critério: 1) Título  2) Categoria e título  3) ID  4) Data de verificação\n");
    do { r = ler_inteiro("  Critério: ", 1, 4, &criterio); CHECA(r); } while (r == 0);
    printf("  Algoritmo: 1) Merge Sort  2) Insertion Sort\n");
    do { r = ler_inteiro("  Algoritmo: ", 1, 2, &alg); CHECA(r); } while (r == 0);

    n = s->lista.tamanho;
    t0 = clock();
    comp = (alg == 1) ? lista_ordenar_merge(&s->lista, (CriterioOrdem)criterio)
                      : lista_ordenar_insercao(&s->lista, (CriterioOrdem)criterio);
    ms = 1000.0 * (double)(clock() - t0) / CLOCKS_PER_SEC;
    listar(&s->lista);
    printf("\n  %s por %s: n = %d, %ld comparações (%.3f ms).\n",
           alg == 1 ? "Merge Sort" : "Insertion Sort", nomes[criterio], n, comp, ms);
    if (n > 1) {
        int k = 0;
        while ((1L << k) < n) k++;                     /* k = teto(log2 n), sem precisar de math.h */
        printf("  Referência: n·teto(log2 n) = %d · pior caso do Insertion n(n-1)/2 = %ld.\n",
               n * k, (long)n * (n - 1) / 2);
    }
    printf("  A lista foi reorganizada trocando ponteiros (os registros não foram copiados).\n");
    s->alterado = 1;
}

static int dias_desde(const char *data) {
    struct tm tm;
    time_t agora = time(NULL), dt;
    memset(&tm, 0, sizeof tm);
    if (sscanf(data, "%4d-%2d-%2d", &tm.tm_year, &tm.tm_mon, &tm.tm_mday) != 3) return 0;
    tm.tm_year -= 1900; tm.tm_mon -= 1; tm.tm_hour = 12;
    dt = mktime(&tm);
    if (dt == (time_t)-1) return 0;
    return (int)(difftime(agora, dt) / 86400.0);
}

static void op_analise(Sistema *s) {
    char cats[MAX_CATEGORIAS][TAM_CATEGORIA];
    int qtd[MAX_CATEGORIAS], n, i, j, reconferir = 0, sem_req = 0;
    No *p, *antiga = NULL;

    titulo_secao("Análise da base");
    if (avisar_lista_vazia(s)) return;
    n = listar_categorias(&s->lista, cats, qtd, MAX_CATEGORIAS);
    printf("  Total de orientações: %d em %d categoria(s)\n\n", s->lista.tamanho, n);
    printf("  Quantidade por categoria:\n");
    for (i = 0; i < n; i++) {
        printf("    ");
        imprimir_coluna(cats[i], 30);
        printf(" %3d  ", qtd[i]);
        for (j = 0; j < qtd[i]; j++) printf("#");
        printf("\n");
    }
    for (p = s->lista.inicio; p != NULL; p = p->prox) {
        if (antiga == NULL || strcmp(p->dado.verificado_em, antiga->dado.verificado_em) < 0) antiga = p;
        if (dias_desde(p->dado.verificado_em) > DIAS_RECONFERIR) reconferir++;
        if (p->dado.requisitos[0] == '\0') sem_req++;
    }
    printf("\n  Fonte verificada há mais de %d dias: %d orientação(ões)\n", DIAS_RECONFERIR, reconferir);
    printf("  Verificação mais antiga: ID %d em %s\n", antiga->dado.id, antiga->dado.verificado_em);
    printf("  Sem documentos/requisitos informados: %d\n", sem_req);
    printf("  Memória da lista: %d nós × %zu bytes = %zu bytes (heap)\n",
           s->lista.tamanho, sizeof(No), (size_t)s->lista.tamanho * sizeof(No));
}

static void op_listar(Sistema *s) {
    titulo_secao("Todas as orientações (ordem atual da lista)");
    if (avisar_lista_vazia(s)) return;
    listar(&s->lista);
    printf("  Total: %d\n", s->lista.tamanho);
}

static void op_salvar(Sistema *s) {
    int n = arquivo_salvar(s->arquivo, &s->lista);
    if (n < 0) printf("  ! Não foi possível gravar %s.\n", s->arquivo);
    else { s->alterado = 0; printf("  %d orientação(ões) gravada(s) em %s.\n", n, s->arquivo); }
}

static void menu(void) {
    printf("\n----------------------------------------------\n");
    printf(" Citizen Assist AI Mobilidade · orientações\n");
    printf("----------------------------------------------\n");
    printf(" 1) Cadastrar          6) Ordenar e exibir\n");
    printf(" 2) Consultar por ID   7) Análise da base\n");
    printf(" 3) Pesquisar          8) Listar todas\n");
    printf(" 4) Atualizar          9) Salvar no arquivo\n");
    printf(" 5) Excluir            0) Sair\n");
}

int main(int argc, char *argv[]) {
    Sistema s;
    ResumoCarga carga;
    int opcao = -1, r;

#ifdef _WIN32
    SetConsoleOutputCP(CP_UTF8);
    SetConsoleCP(CP_UTF8);
#endif
    lista_iniciar(&s.lista);
    s.arquivo = (argc > 1) ? argv[1] : ARQUIVO_PADRAO;
    s.alterado = 0;

    printf("Citizen Assist AI Mobilidade · Data Structures and Algorithms (GS 2026.2)\n");
    printf("Protótipo acadêmico: não é canal oficial. Não pede nem guarda dados pessoais.\n");
    if (arquivo_carregar(s.arquivo, &s.lista, &carga) < 0)
        printf("Arquivo %s não encontrado: a lista começa vazia.\n", s.arquivo);
    else {
        printf("%d orientação(ões) carregada(s) de %s.\n", carga.carregados, s.arquivo);
        if (carga.ignorados) printf("Aviso: %d linha(s) inválida(s) ignorada(s).\n", carga.ignorados);
    }

    while (!fim_entrada) {
        menu();
        r = ler_inteiro("Opção: ", 0, 9, &opcao);
        if (r < 0) break;
        if (r == 0) continue;
        switch (opcao) {
            case 1: op_cadastrar(&s); break;
            case 2: op_consultar(&s); break;
            case 3: op_pesquisar(&s); break;
            case 4: op_atualizar(&s); break;
            case 5: op_excluir(&s);   break;
            case 6: op_ordenar(&s);   break;
            case 7: op_analise(&s);   break;
            case 8: op_listar(&s);    break;
            case 9: op_salvar(&s);    break;
            case 0: fim_entrada = 1;  break;
        }
    }

    if (s.alterado) {
        int sim = 0;
        if (opcao == 0) {
            do { r = ler_sim_nao("Há alterações não salvas. Salvar antes de sair? (s/n): ", &sim); }
            while (r == 0);
        } else {
            printf("Entrada encerrada: alterações não salvas foram descartadas.\n");
        }
        if (sim) op_salvar(&s);
    }
    lista_liberar(&s.lista);
    printf("Até logo! Memória da lista liberada.\n");
    return 0;
}
