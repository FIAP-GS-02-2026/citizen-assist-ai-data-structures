"""
Global Solution 2026.2 - Data Structures and Algorithms - FIAP
Integrantes: Gabriel Fagundes (RM 569074), Glauco Kelly (RM 572840),
             Thiago Renatino (RM 569073)

Gera docs/documentacao_tecnica_dsa.pdf a partir de:
- docs/diagrama_lista.png e docs/crescimento_ordenacao.png (gerar_figuras.py)
- tests/resultado_testes.txt e tests/evidencias/*.txt (sh tests/gerar_evidencias.sh)
- dados/orientacoes.txt
"""

import re
import textwrap
from pathlib import Path

from reportlab.lib import colors
from reportlab.lib.enums import TA_CENTER, TA_JUSTIFY
from reportlab.lib.pagesizes import A4
from reportlab.lib.styles import ParagraphStyle, getSampleStyleSheet
from reportlab.lib.units import cm
from reportlab.pdfbase import pdfmetrics
from reportlab.pdfbase.ttfonts import TTFont
from reportlab.platypus import (CondPageBreak, Image, KeepTogether, PageBreak, Paragraph, Preformatted, SimpleDocTemplate,
                                Spacer, Table, TableStyle)

DOCS = Path(__file__).resolve().parent
RAIZ = DOCS.parent
TESTS = RAIZ / "tests"
EVID = TESTS / "evidencias"
SAIDA = DOCS / "documentacao_tecnica_dsa.pdf"

REPO = "https://github.com/FIAP-GS-02-2026/citizen-assist-ai-data-structures"
INTEGRANTES = [("Gabriel Fagundes", "RM 569074"), ("Glauco Kelly", "RM 572840"),
               ("Thiago Renatino", "RM 569073")]
AZUL = colors.HexColor("#1F4E79")
CINZA = colors.HexColor("#F2F2F2")


def registrar_fontes():
    candidatos = [
        ("/usr/share/fonts/truetype/dejavu/DejaVuSans.ttf", "/usr/share/fonts/truetype/dejavu/DejaVuSans-Bold.ttf",
         "/usr/share/fonts/truetype/dejavu/DejaVuSansMono.ttf"),
        ("C:/Windows/Fonts/arial.ttf", "C:/Windows/Fonts/arialbd.ttf", "C:/Windows/Fonts/consola.ttf"),
    ]
    for normal, negrito, mono in candidatos:
        try:
            pdfmetrics.registerFont(TTFont("Texto", normal))
            pdfmetrics.registerFont(TTFont("Texto-Negrito", negrito))
            pdfmetrics.registerFont(TTFont("Mono", mono))
            pdfmetrics.registerFontFamily("Texto", normal="Texto", bold="Texto-Negrito")
            return "Texto", "Texto-Negrito", "Mono"
        except Exception:
            continue
    return "Helvetica", "Helvetica-Bold", "Courier"


FT, FN, FM = registrar_fontes()
base = getSampleStyleSheet()
CORPO = ParagraphStyle("c", parent=base["Normal"], fontName=FT, fontSize=9.8, leading=14,
                       alignment=TA_JUSTIFY, spaceAfter=6)
H1 = ParagraphStyle("h1", parent=CORPO, fontName=FN, fontSize=14, leading=18, textColor=AZUL,
                    spaceBefore=10, spaceAfter=8, alignment=0, keepWithNext=1)
H2 = ParagraphStyle("h2", parent=H1, fontSize=11.5, leading=15, spaceBefore=8, spaceAfter=5)
LEG = ParagraphStyle("leg", parent=CORPO, fontSize=8.3, leading=11, alignment=TA_CENTER,
                     textColor=colors.HexColor("#555555"), spaceAfter=10)
CEL = ParagraphStyle("cel", parent=CORPO, fontSize=8.4, leading=11, alignment=0, spaceAfter=0)
BUL = ParagraphStyle("bul", parent=CORPO, leftIndent=12)
DEST = ParagraphStyle("dest", parent=CORPO, backColor=colors.HexColor("#EAF1F8"), borderPadding=(6, 6, 6, 6),
                      borderColor=AZUL, borderWidth=0.6, spaceBefore=6, spaceAfter=12)
CODIGO = ParagraphStyle("cod", parent=CORPO, fontName=FM, fontSize=7.6, leading=9.6, alignment=0)
MONO = ParagraphStyle("mono", parent=CODIGO, fontSize=6.6, leading=8.4)


def p(t, e=CORPO):
    if e is H1:
        return [CondPageBreak(6 * cm), Paragraph(t, e)]
    return Paragraph(t, e)


def bullets(itens):
    return [p(f"• {i}", BUL) for i in itens]


def tabela(linhas, larguras, cab=True):
    dados = [[Paragraph(str(c), CEL) for c in l] for l in linhas]
    t = Table(dados, colWidths=larguras, hAlign="CENTER", repeatRows=1 if cab else 0)
    est = [("GRID", (0, 0), (-1, -1), 0.4, colors.HexColor("#BFBFBF")), ("VALIGN", (0, 0), (-1, -1), "MIDDLE"),
           ("TOPPADDING", (0, 0), (-1, -1), 3), ("BOTTOMPADDING", (0, 0), (-1, -1), 3)]
    if cab:
        est.append(("BACKGROUND", (0, 0), (-1, 0), CINZA))
    t.setStyle(TableStyle(est))
    return t


def codigo(texto, estilo=CODIGO):
    t = Table([[Preformatted(texto.strip("\n"), estilo)]], colWidths=[17 * cm])
    t.setStyle(TableStyle([("BACKGROUND", (0, 0), (-1, -1), colors.HexColor("#F6F8FA")),
                           ("BOX", (0, 0), (-1, -1), 0.4, colors.HexColor("#D0D7DE")),
                           ("LEFTPADDING", (0, 0), (-1, -1), 6), ("TOPPADDING", (0, 0), (-1, -1), 5),
                           ("BOTTOMPADDING", (0, 0), (-1, -1), 5)]))
    return t


def figura(caminho, largura_cm, legenda):
    img = Image(str(caminho))
    prop = img.imageHeight / img.imageWidth
    img.drawWidth, img.drawHeight = largura_cm * cm, largura_cm * cm * prop
    return KeepTogether([img, p(legenda, LEG)])


def com_legenda(obj, legenda):
    return KeepTogether([obj, p(legenda, LEG)])


def trecho_funcao(arquivo, assinatura):
    """Copia do código-fonte o corpo de uma função (do cabeçalho até a chave que fecha)."""
    txt = (RAIZ / "src" / arquivo).read_text(encoding="utf-8")
    i = txt.index(assinatura)
    nivel, j, aberto = 0, i, False
    while j < len(txt):
        if txt[j] == "{":
            nivel += 1
            aberto = True
        elif txt[j] == "}":
            nivel -= 1
            if aberto and nivel == 0:
                return txt[i:j + 1]
        j += 1
    return txt[i:]


def trecho_sessao(nome, inicio, fim=None, limite=40):
    linhas = (EVID / nome).read_text(encoding="utf-8").splitlines()
    i = next(k for k, l in enumerate(linhas) if inicio in l)
    sel = []
    for l in linhas[i:]:
        if fim and fim in l and sel:
            break
        if re.match(r"^ \d\) |^-{10}| Citizen Assist AI Mobilidade · orientações$", l):
            continue
        sel += textwrap.wrap(l, 118, subsequent_indent="      ", drop_whitespace=False) or [""]
        if len(sel) >= limite:
            break
    return "\n".join(sel)


def ler_testes():
    linhas = (TESTS / "resultado_testes.txt").read_text(encoding="utf-8").splitlines()
    casos, cresc, resumo = [], [], ""
    for l in linhas:
        partes = [x.strip() for x in l.split("|")]
        if len(partes) == 5 and partes[4] in ("OK", "FALHOU"):
            casos.append(partes)
        m = re.match(r"^\s*(\d+) \|\s+(\d+) \|\s+(\d+) \|\s+(\d+) \|\s+(\d+)", l)
        if m:
            cresc.append([int(x) for x in m.groups()])
        if "testes," in l:
            resumo = l.strip()
    return casos, cresc, resumo


def rodape(c, doc):
    c.saveState()
    c.setFont(FT, 7.5)
    c.setFillColor(colors.HexColor("#777777"))
    c.drawString(2 * cm, 1.2 * cm, "Citizen Assist AI Mobilidade · Data Structures and Algorithms · GS 2026.2")
    c.drawRightString(A4[0] - 2 * cm, 1.2 * cm, f"Página {doc.page}")
    c.restoreState()


def main():
    casos, cresc, resumo_testes = ler_testes()
    base_txt = (RAIZ / "dados" / "orientacoes.txt").read_text(encoding="utf-8").splitlines()
    registros = [l.split("|") for l in base_txt if l and not l.startswith("#")]
    cats = {}
    for r in registros:
        cats[r[1]] = cats.get(r[1], 0) + 1
    memoria = (EVID / "resumo_memoria.txt").read_text(encoding="utf-8").splitlines()

    s = []
    # ---------------------------------------------------------------- CAPA
    s += [Spacer(1, 2.6 * cm),
          p("Documentação Técnica", ParagraphStyle("t0", parent=H1, fontSize=12, alignment=TA_CENTER,
                                                   textColor=colors.HexColor("#555555"))),
          p("Cadastro de Orientações ao Cidadão em Lista Encadeada",
            ParagraphStyle("t1", parent=H1, fontSize=21, leading=27, alignment=TA_CENTER)),
          p("Citizen Assist AI Mobilidade · ônibus, Bilhete Único, gratuidades e mobilidade elétrica em São Paulo",
            ParagraphStyle("t2", parent=CORPO, fontSize=11, alignment=TA_CENTER)),
          Spacer(1, 1.2 * cm),
          p("Global Solution 2026.2 · Desafio CitizenTech<br/>Disciplina: Data Structures and Algorithms<br/>"
            "Linguagem: C (C11)<br/>1CC-FEV · FIAP", ParagraphStyle("t3", parent=CORPO, alignment=TA_CENTER)),
          Spacer(1, 1 * cm)]
    cap = tabela([["Integrante", "RM"]] + [list(i) for i in INTEGRANTES], [7 * cm, 4 * cm])
    s += [cap, Spacer(1, 1.6 * cm),
          p(f"Repositório: {REPO}<br/>"
            "Compilar: gcc -std=c11 -Wall -Wextra -O2 -o citizen_assist src/*.c<br/>"
            "São Paulo, 18/11/2026", ParagraphStyle("t4", parent=CORPO, fontSize=8.8, alignment=TA_CENTER)),
          PageBreak()]

    # ---------------------------------------------------------------- RESUMO
    s += [p("Resumo", H1),
          p("Este trabalho implementa, em C, o módulo de cadastro e consulta do Citizen Assist AI Mobilidade: um "
            "sistema de terminal que guarda orientações ao cidadão (gratuidades, tarifas, Bilhete Único, "
            "acessibilidade, canais de reclamação, linhas noturnas e mobilidade elétrica) numa <b>lista "
            "simplesmente encadeada própria</b>, com nós alocados por <i>malloc</i> e liberados por <i>free</i>. "
            "O sistema cadastra, consulta por identificador, pesquisa por título ou categoria, atualiza, exclui, "
            "ordena (Merge Sort e Insertion Sort implementados pela equipe, sem <i>qsort</i>) e apresenta "
            "indicadores da base. O menu trata opções inválidas, identificadores duplicados, lista vazia, textos "
            "longos e fim da entrada."),
          p(f"A base inicial tem <b>{len(registros)} orientações reais</b> em {len(cats)} categorias, com fonte "
            "oficial e data de conferência, as mesmas usadas pelo assistente em Python do grupo. "
            f"Os testes automatizados somam <b>{resumo_testes.split(',')[0]}</b>, todos aprovados, e três roteiros "
            "do menu foram executados com valgrind sem nenhum erro ou vazamento de memória."),
          ]

    # ---------------------------------------------------------------- 1 RECORTE
    s += [p("1. Recorte escolhido", H1),
          p("O grupo trabalha o tema <b>mobilidade urbana em São Paulo</b> em todas as disciplinas da Global "
            "Solution. O público é quem depende do ônibus municipal, principalmente pessoas idosas, pessoas com "
            "deficiência, estudantes e quem tem pouca familiaridade digital. A informação sobre direitos e "
            "serviços existe, mas está espalhada entre SPTrans, SP156 e Prefeitura. Este módulo é a parte que "
            "<b>organiza essa informação</b>: quem mantém a base cadastra, corrige, remove e encontra orientações "
            "com rapidez, e sabe quais fontes precisam ser conferidas de novo."),
          p("O recorte se enquadra em <i>serviços urbanos</i> e <i>documentação e cidadania</i>, sugeridos no "
            "enunciado. O foco é a aplicação de estruturas de dados: o programa não usa IA generativa, não integra "
            "serviços reais, não se apresenta como canal oficial e não pede nem guarda dados pessoais."),
          com_legenda(tabela([["Categoria", "Orientações", "Exemplos"]] +
                             [[c, str(n), ex] for (c, n), ex in zip(cats.items(), [
                                 "idoso 65+, 60 a 64 anos, PcD, estudante",
                                 "Tarifa Zero aos domingos, integração, bilhete mensal",
                                 "como tirar, recarga, segunda via",
                                 "Atende+, desembarque fora do ponto",
                                 "SP156 e Ouvidoria SPTrans",
                                 "Rede Noturno",
                                 "recarga de bike/patinete, solar, regras de circulação"])],
                             [5.2 * cm, 2.3 * cm, 9.5 * cm]),
                      "Tabela 1 – Base inicial (dados/orientacoes.txt), convertida da base do assistente do grupo.")]

    # ---------------------------------------------------------------- 2 MODELO
    s += [p("2. Modelo dos registros", H1),
          p("Cada orientação é uma <i>struct</i> com tamanho fixo, definida com <i>typedef</i> em "
            "<i>src/orientacao.h</i>. Os textos são guardados em UTF-8; as funções de cópia nunca cortam uma "
            "letra acentuada ao meio, e as comparações ignoram maiúsculas e acentos."),
          codigo('''typedef struct {
    int  id;                              /* identificador único (1 a 99999) */
    char titulo[TAM_TITULO];              /* 120 */
    char categoria[TAM_CATEGORIA];        /*  48 */
    char descricao[TAM_DESCRICAO];        /* 1024: a orientação em si */
    char requisitos[TAM_REQUISITOS];      /* 512: documentos ou condições, separados por "; " */
    char local[TAM_LOCAL];                /* 200: onde o cidadão resolve */
    char fonte[TAM_FONTE];                /* 200: URL oficial consultada */
    char verificado_em[TAM_DATA];         /*  11: AAAA-MM-DD */
} Orientacao;'''),
          Spacer(1, 6),
          com_legenda(tabela([
              ["Campo", "Tipo", "Regra de validação"],
              ["id", "int", "Inteiro de 1 a 99999, único na lista (checado no cadastro e na carga do arquivo)."],
              ["titulo, categoria, descricao, local, fonte", "char[]", "Obrigatórios; só espaços não vale; "
               "texto maior que o campo é recusado; o caractere '|' é proibido (separador do arquivo)."],
              ["requisitos", "char[]", "Opcional (algumas orientações não exigem documento)."],
              ["verificado_em", "char[11]", "Data real no formato AAAA-MM-DD, com ano bissexto (2026-02-30 é "
               "recusada, 2028-02-29 é aceita)."]], [4.6 * cm, 1.8 * cm, 10.6 * cm]),
              "Tabela 2 – Campos e validações (validar_orientacao e data_valida, em src/orientacao.c).")]

    # ---------------------------------------------------------------- 3 LISTA
    s += [p("3. Lista encadeada", H1),
          p("A lista é simplesmente encadeada e guarda três informações no descritor: o ponteiro para o primeiro "
            "nó, o ponteiro para o último e o tamanho. Cada nó contém uma <i>Orientacao</i> e o ponteiro para o "
            "próximo. Guardar o ponteiro <i>fim</i> permite ligar um nó novo ao final sem percorrer a lista."),
          codigo('''typedef struct No {
    Orientacao dado;
    struct No *prox;
} No;

typedef struct {
    No *inicio;
    No *fim;
    int tamanho;
} Lista;'''),
          Spacer(1, 6),
          figura(DOCS / "diagrama_lista.png", 16.5, "Figura 1 – Encadeamento dos nós e remoção de um nó do meio."),
          p("3.1 Inserção", H2),
          p("O cadastro valida o registro, percorre a lista para garantir que o ID não existe, aloca o nó com "
            "<i>malloc</i> (verificando falta de memória) e o liga ao final. Na lista vazia, o novo nó passa a "
            "ser início e fim."),
          codigo(trecho_funcao("lista.c", "ResultadoLista lista_inserir")),
          p("3.2 Percurso e busca por ID", H2),
          p("O percurso segue os ponteiros <i>prox</i> a partir de <i>inicio</i> até chegar a NULL. É assim que "
            "a lista é exibida, que o ID é procurado e que os indicadores são calculados."),
          codigo(trecho_funcao("lista.c", "No *lista_buscar_id")),
          p("3.3 Remoção", H2),
          p("A remoção percorre a lista guardando o nó anterior. Ao achar o ID, religa o anterior ao sucessor "
            "(ou move o <i>inicio</i>, se era o primeiro), corrige o <i>fim</i> se era o último e só então libera "
            "a memória. Assim a lista nunca fica com ponteiro para memória já liberada."),
          codigo(trecho_funcao("lista.c", "ResultadoLista lista_remover")),
          p("3.4 Atualização e integridade", H2),
          p("A atualização localiza o nó, valida o novo conteúdo numa cópia e só então substitui o dado, mantendo "
            "o ID e os ponteiros. Se a validação falha, o registro original fica intacto. A função "
            "<i>lista_integra</i> confere, nos testes, que o tamanho bate com a contagem real, que <i>fim</i> é "
            "o último nó e que não há IDs repetidos. Ao sair, <i>lista_liberar</i> percorre a lista liberando "
            "cada nó; o valgrind confirma que nenhum bloco fica alocado.")]

    # ---------------------------------------------------------------- 4 ALGORITMOS
    s += [p("4. Algoritmos de busca e ordenação", H1),
          p("4.1 Busca sequencial", H2),
          p("A consulta por ID e a pesquisa por título ou categoria usam <b>busca sequencial</b>: começam no "
            "primeiro nó e comparam um a um até o fim (pesquisa) ou até achar (ID). A busca binária não serve "
            "aqui: ela depende de acesso direto ao elemento do meio, e numa lista encadeada chegar ao meio já "
            "custa percorrer metade dos nós."),
          p("Na pesquisa por texto, título (ou categoria) e termo passam por <i>normalizar()</i>: tudo em "
            "minúsculas, sem acentos e sem espaços nas pontas. Depois o programa verifica se o termo aparece "
            "como trecho (<i>strstr</i>). Assim \"onibus\" encontra \"Ônibus\" e \"BILHETE UNICO\" encontra "
            "\"Bilhete Único\". Sem resultado, o sistema avisa e mostra as categorias disponíveis; termo vazio "
            "é recusado em vez de devolver a base inteira."),
          codigo(trecho_funcao("lista.c", "int lista_pesquisar")),
          p("4.2 Merge Sort na lista", H2),
          p("O Merge Sort foi escolhido como ordenação principal porque combina muito bem com listas encadeadas: "
            "a lista é dividida ao meio com dois ponteiros (um anda um nó, o outro anda dois), cada metade é "
            "ordenada recursivamente e as duas são intercaladas <b>religando ponteiros</b>, sem copiar registros "
            "e sem vetor auxiliar. Na intercalação, o empate usa <i>&lt;=</i>, o que torna a ordenação "
            "<b>estável</b>: registros iguais pelo critério mantêm a ordem anterior. Há quatro critérios: título, "
            "categoria e título, ID e data de verificação (as fontes conferidas há mais tempo aparecem primeiro)."),
          codigo(trecho_funcao("lista.c", "static No *intercalar") + "\n\n" +
                 trecho_funcao("lista.c", "static No *merge_sort")),
          p("4.3 Insertion Sort na lista", H2),
          p("O Insertion Sort foi implementado para comparação. Ele monta uma nova lista já ordenada, retirando "
            "um nó por vez da lista original e inserindo-o na posição certa. Durante os testes percebemos que, "
            "numa lista simplesmente encadeada, procurar a posição a partir do início torna a <b>lista já "
            "ordenada o pior caso</b> (o contrário do vetor). Por isso a implementação compara primeiro com o "
            "último nó e depois com o primeiro: com isso, entrada já ordenada ou invertida custa O(n) e só a "
            "ordem embaralhada leva ao custo quadrático."),
          codigo(trecho_funcao("lista.c", "long lista_ordenar_insercao"))]

    # ---------------------------------------------------------------- 5 MENU
    s += [CondPageBreak(14 * cm), p("5. Menu e tratamento de entradas", H1),
          com_legenda(tabela([
              ["Opção", "Função", "Casos tratados"],
              ["1 Cadastrar", "Lê ID (sugere o próximo livre) e os campos", "ID inválido ou duplicado, campo "
               "obrigatório vazio, texto longo, '|' no texto, data inválida; categoria por número ou nome novo"],
              ["2 Consultar por ID", "Mostra a ficha completa com fonte e aviso", "Lista vazia, ID inexistente, "
               "ID não numérico"],
              ["3 Pesquisar", "Título ou categoria, sem acento", "Lista vazia, campo inválido, termo vazio, "
               "nenhum resultado (sugere categorias)"],
              ["4 Atualizar", "Enter mantém o valor de cada campo", "Lista vazia, ID inexistente, dado inválido "
               "(registro preservado)"],
              ["5 Excluir", "Mostra o registro e pede confirmação", "Lista vazia, ID inexistente, resposta "
               "diferente de s/n, cancelamento"],
              ["6 Ordenar e exibir", "4 critérios × Merge ou Insertion", "Lista vazia; mostra comparações e "
               "tempo de cada execução"],
              ["7 Análise", "Indicadores da base", "Total, quantidade por categoria, fontes a reconferir, "
               "memória ocupada pelos nós"],
              ["8 Listar", "Ordem atual da lista", "Lista vazia"],
              ["9 Salvar", "Grava dados/orientacoes.txt", "Falha de gravação (usa arquivo temporário)"],
              ["0 Sair", "Pergunta se salva as alterações", "Fim da entrada (Ctrl+D/Ctrl+Z) encerra com "
               "segurança e libera a memória"]], [3 * cm, 5.2 * cm, 8.8 * cm]),
              "Tabela 3 – Opções do menu."),
          p("Toda leitura passa por <i>ler_linha</i>, que usa <i>fgets</i> com limite de tamanho; números são "
            "convertidos por <i>texto_para_inteiro</i>, que aceita só dígitos e confere a faixa. Por isso "
            "\"abc\", \"12a\", \"1.5\", texto vazio e números enormes nunca travam o programa nem viram valores "
            "inesperados. A extensão opcional de <b>persistência em arquivo</b> carrega a base no início, "
            "ignorando e contando linhas com formato errado ou ID repetido."),
          p("Exemplo real (roteiro 01): opção inválida e cadastro com ID duplicado, ID inválido, descrição vazia "
            "e data em formato errado.", H2),
          codigo(trecho_sessao("sessao_01_operacoes.txt", "Opção: abc", "Opção: 2", 40), MONO)]

    # ---------------------------------------------------------------- 6 BIG-O
    s += [p("6. Análise dos algoritmos (Big-O)", H1),
          p("A análise considera a implementação efetiva do código. <i>n</i> é o número de registros na lista e "
            "<i>L</i> o tamanho dos textos comparados (no máximo 1.024 bytes, uma constante)."),
          com_legenda(tabela([
              ["Operação", "Melhor", "Pior", "Justificativa"],
              ["Inserir (cadastro)", "O(n)", "O(n)", "O encadeamento no fim é O(1) graças ao ponteiro fim, mas "
               "antes é preciso percorrer a lista inteira para garantir que o ID não está repetido."],
              ["Buscar por ID", "O(1)", "O(n)", "Busca sequencial: o ID pode ser o primeiro nó ou não existir "
               "(percorre tudo). Em média, n/2 comparações."],
              ["Pesquisar título/categoria", "O(n·L)", "O(n·L)", "Precisa olhar todos os nós para achar todos os "
               "resultados; cada comparação normaliza e procura o trecho (custo proporcional a L)."],
              ["Atualizar", "O(1)", "O(n)", "Busca o nó (O(n) no pior caso) e troca o dado em O(1), sem mexer em "
               "ponteiros."],
              ["Remover", "O(1)", "O(n)", "Percorre até o nó guardando o anterior; religar e liberar é O(1). O "
               "primeiro nó é removido sem percorrer."],
              ["Listar / liberar", "O(n)", "O(n)", "Um passo por nó."],
              ["Merge Sort", "O(n log n)", "O(n log n)", "log n níveis de divisão; em cada nível, dividir e "
               "intercalar passam por n nós. Memória extra O(log n) (pilha da recursão), sem vetor auxiliar."],
              ["Insertion Sort", "O(n)", "O(n²)", "Entrada ordenada ou invertida: 1 ou 2 comparações por nó. "
               "Ordem embaralhada: em média n²/4 comparações. Memória extra O(1)."],
              ["Análise por categoria", "O(n·k)", "O(n·k)", "Para cada nó, procura a categoria entre as k já "
               "vistas (k ≤ 32)."],
              ["Carregar arquivo", "O(n²)", "O(n²)", "Cada linha é uma inserção O(n). Com 24 registros é "
               "imperceptível; para milhares, uma tabela hash por ID deixaria a checagem em O(1)."]],
              [3.4 * cm, 1.8 * cm, 1.8 * cm, 10 * cm]),
              "Tabela 4 – Complexidade de tempo das operações implementadas."),
          p("6.1 Verificação prática", H2),
          p("O programa de testes ordena listas de 250 a 4.000 registros embaralhados e conta as comparações de "
            "cada algoritmo. Os números medidos acompanham as curvas teóricas: dobrar <i>n</i> faz o Merge Sort "
            "pouco mais que dobrar as comparações, enquanto o Insertion Sort as multiplica por cerca de quatro."),
          com_legenda(tabela([["n", "Merge Sort (medido)", "n·⌈log₂ n⌉", "Insertion Sort (medido)", "n²/4"]] +
                             [[f"{a:,}".replace(",", "."), f"{b:,}".replace(",", "."), f"{c:,}".replace(",", "."),
                               f"{d:,}".replace(",", "."), f"{e:,}".replace(",", ".")] for a, b, c, d, e in cresc],
                             [2 * cm, 3.6 * cm, 3 * cm, 4.2 * cm, 3 * cm]),
                      "Tabela 5 – Comparações medidas (tests/resultado_testes.txt)."),
          figura(DOCS / "crescimento_ordenacao.png", 14.5,
                 "Figura 2 – Crescimento das comparações em escala logarítmica."),
          p("No menu, cada ordenação também mostra o número de comparações. Na base real (25 registros), o Merge "
            "Sort por título fez 86 comparações, abaixo do limite n·⌈log₂ n⌉ = 125, e o Insertion Sort por data "
            "fez 183, abaixo do pior caso n(n−1)/2 = 300.", DEST)]

    # ---------------------------------------------------------------- 7 TESTES
    grupos = {}
    for g, *_ in casos:
        grupos[g] = grupos.get(g, 0) + 1
    descr = {"vazia": "operações com lista vazia", "inserir": "cadastro, duplicados e dados inválidos",
             "busca": "busca por ID e pesquisa por texto, inclusive sem resultado",
             "atualizar": "atualização válida, inexistente e inválida", "remover": "remoção no início, meio, fim e "
             "inexistente", "ordenar": "Merge e Insertion: critérios, estabilidade e contagem",
             "entrada": "conversão de números e normalização de texto",
             "arquivo": "gravação, leitura e arquivo com linhas inválidas"}
    s += [p("7. Testes e evidências", H1),
          p("Os testes têm duas partes, ambas reproduzíveis com <i>sh tests/gerar_evidencias.sh</i>:"),
          *bullets(["<b>Testes automatizados</b> (<i>tests/testes.c</i>): chamam diretamente as funções da lista e "
                    "comparam o resultado esperado com o observado.",
                    "<b>Roteiros do menu</b> (<i>tests/roteiros/</i>): sequências de teclas digitadas no programa "
                    "real; a saída completa, com o texto digitado, está em <i>tests/evidencias/</i>."]),
          com_legenda(tabela([["Grupo", "Casos", "O que cobre"]] +
                             [[g, str(n), descr.get(g, "")] for g, n in grupos.items()] +
                             [["Total", str(len(casos)), resumo_testes]], [2.6 * cm, 1.6 * cm, 12.8 * cm]),
                      "Tabela 6 – Testes automatizados por grupo (todos aprovados)."),
          CondPageBreak(6 * cm), p("7.1 Casos representativos", H2)]
    escolhidos = ["remover ID 1 em lista vazia", "Merge Sort em lista vazia (comparações)",
                  "inserir ID 3", "ID 3 duplicado é recusado", "data 2026-02-30 é recusada",
                  "buscar ID 999 (não existe)", "título \"onibus\" sem acento acha \"ônibus\"",
                  "termo inexistente \"metrô noturno\"", "registro original preservado após recusa",
                  "fim volta para o anterior", "Merge Sort por título (A-Z, sem acento)",
                  "Merge Sort é estável (empate mantém 8,6)", "\"abc\" é recusado",
                  "\"99999999999\" (estouro) é recusado", "arquivo com erros: 5 linhas ignoradas"]
    linhas = [["Tipo", "Entrada / caso", "Esperado", "Observado", "Situação"]]
    tipos = {"vazia": "Lista vazia", "inserir": "Normal / inválido", "busca": "Inexistente / normal",
             "atualizar": "Inválido", "remover": "Normal", "ordenar": "Normal", "entrada": "Entrada inválida",
             "arquivo": "Entrada inválida"}
    for g, caso, esp, obs, res in casos:
        if caso in escolhidos:
            linhas.append([tipos.get(g, g), caso, esp, obs, "Aprovado" if res == "OK" else "Reprovado"])
    s += [tabela(linhas, [2.6 * cm, 6.4 * cm, 3 * cm, 3 * cm, 2 * cm]),
          p("Tabela 7 – Entradas, resultados esperados e observados (lista completa no Anexo A).", LEG),
          p("7.2 Roteiros do menu", H2),
          com_legenda(tabela([
              ["Roteiro", "Situação demonstrada", "Resultado observado"],
              ["01_operacoes", "Base real: opções inválidas, cadastro com erros, consulta, pesquisas (com e sem "
               "acento, por categoria), atualização com Enter mantendo campos, duas ordenações, análise, exclusão "
               "cancelada e confirmada, exclusão de ID inexistente, salvar e sair",
               "Cada erro foi recusado com mensagem e o programa seguiu; totais 24 → 25 → 24"],
              ["02_lista_vazia", "Arquivo inexistente: opções 2 a 8 com a lista vazia, primeira inserção, "
               "exclusão do único nó e nova listagem", "Todas as opções avisam que a lista está vazia; "
               "início/fim corretos após inserir e remover"],
              ["03_fim_da_entrada", "Entrada termina no meio de um cadastro", "Programa encerra sem travar, "
               "libera a memória e não altera o arquivo"]], [3 * cm, 8.4 * cm, 5.6 * cm]),
              "Tabela 8 – Roteiros executados no programa real."),
          p("Trecho do roteiro 02: lista vazia e primeira inserção.", H2),
          codigo(trecho_sessao("sessao_02_lista_vazia.txt", "Opção: 2", "Opção: 8", 14) + "\n...\n" +
                 trecho_sessao("sessao_02_lista_vazia.txt", "== Excluir orientação ==", None, 12), MONO),
          p("7.3 Memória", H2),
          p("Todas as execuções foram feitas com <i>valgrind --leak-check=full</i>. Não houve leitura inválida, "
            "uso de memória liberada nem vazamento:"),
          codigo("\n".join(memoria), MONO)]

    # ---------------------------------------------------------------- 8 EXECUÇÃO
    s += [p("8. Como compilar e executar", H1),
          *bullets(["<b>Windows</b> (MinGW-w64/GCC, Code::Blocks ou Dev-C++): na pasta do projeto, "
                    "<i>gcc -std=c11 -Wall -Wextra -O2 -o citizen_assist.exe src\\*.c</i> e depois "
                    "<i>citizen_assist.exe</i>. Use o Windows Terminal para os acentos aparecerem corretamente.",
                    "<b>Linux, macOS ou WSL</b>: <i>gcc -std=c11 -Wall -Wextra -O2 -o citizen_assist src/*.c</i> "
                    "e depois <i>./citizen_assist</i> (ou <i>make</i> e <i>make run</i>).",
                    "<b>Testes</b>: <i>make test</i>, ou <i>sh tests/gerar_evidencias.sh</i> para refazer todas as "
                    "evidências.",
                    "O programa abre <i>dados/orientacoes.txt</i>; para usar outro arquivo, passe o caminho: "
                    "<i>./citizen_assist minha_base.txt</i>."]),
          p("9. Limitações e melhorias possíveis", H1),
          *bullets(["Busca por ID e checagem de duplicado são O(n); com milhares de registros, uma tabela hash "
                    "ou árvore de busca por ID reduziria o custo.",
                    "A pesquisa encontra trechos exatos (sem acento); não tolera erro de digitação.",
                    "No console antigo do Windows (cmd), letras acentuadas digitadas podem não ser lidas; no "
                    "Windows Terminal funcionam.",
                    "As orientações dependem de regras que mudam: cada registro mostra a data de verificação, e a "
                    "análise aponta as fontes conferidas há mais de 30 dias."]),
          p("Referências", H1),
          *bullets(["CORMEN, T. H. et al. <i>Algoritmos: teoria e prática</i>. 3. ed. Rio de Janeiro: Elsevier, "
                    "2012 (cap. 2, 10).",
                    "Material das aulas de Data Structures and Algorithms (FIAP, 2026).",
                    "SPTrans e Prefeitura de São Paulo (SP156): fontes de cada orientação, listadas no campo "
                    "fonte da base."])]

    # ---------------------------------------------------------------- ANEXO
    s += [PageBreak(), p("Anexo A – Saída completa dos testes automatizados", H1),
          p("Arquivo tests/resultado_testes.txt (colunas: grupo, caso, esperado, observado, resultado).", LEG)]
    mini = ParagraphStyle("mini", parent=MONO, fontSize=5.6, leading=7)
    todas = (TESTS / "resultado_testes.txt").read_text(encoding="utf-8").splitlines()
    for i in range(0, len(todas), 60):                       # blocos menores que uma página
        s += [codigo("\n".join(todas[i:i + 60]), mini), Spacer(1, 4)]

    doc = SimpleDocTemplate(str(SAIDA), pagesize=A4, leftMargin=2 * cm, rightMargin=2 * cm, topMargin=1.8 * cm,
                            bottomMargin=2 * cm, title="Documentação técnica - DSA - Citizen Assist AI Mobilidade",
                            author="Gabriel Fagundes, Glauco Kelly, Thiago Renatino")
    plano = []
    for item in s:                       # p(..., H1) devolve [CondPageBreak, Paragraph]
        plano.extend(item if isinstance(item, list) else [item])
    doc.build(plano, onFirstPage=lambda c, d: None, onLaterPages=rodape)
    print(f"Documentação gerada em {SAIDA}")


if __name__ == "__main__":
    main()
