# Citizen Assist AI Mobilidade · Cadastro de orientações em lista encadeada

Global Solution 2026.2 · FIAP · 1CC-FEV · **Data Structures and Algorithms** · linguagem **C (C11)**

| Integrante | RM |
|---|---|
| Gabriel Fagundes | 569074 |
| Glauco Kelly | 572840 |
| Thiago Renatino | 569073 |

Sistema de terminal que **cadastra, consulta, pesquisa, atualiza, exclui, ordena e analisa** orientações ao cidadão
sobre o ônibus municipal de São Paulo, o Bilhete Único, gratuidades, acessibilidade, canais de reclamação e mobilidade
elétrica. Os registros ficam numa **lista simplesmente encadeada própria** (`struct`, `typedef`, ponteiros,
`malloc` e `free`), e as ordenações **Merge Sort** e **Insertion Sort** foram implementadas pela equipe, sem `qsort`.

A base inicial tem **24 orientações reais**, com fonte oficial e data de conferência, as mesmas do assistente em
Python do grupo ([citizen-assist-ai](https://github.com/FIAP-GS-02-2026/citizen-assist-ai)).

> Protótipo acadêmico: não é canal oficial e não pede nem guarda dados pessoais. Confirme sempre na fonte indicada.

📄 Documentação técnica completa (recorte, modelo, lista, menu, Big-O e testes):
[`docs/documentacao_tecnica_dsa.pdf`](docs/documentacao_tecnica_dsa.pdf)

## Como compilar e executar

Precisa de um compilador C (GCC/MinGW, Clang ou o do Code::Blocks/Dev-C++). Não usa bibliotecas externas.

**Windows** (MinGW-w64 no PATH), na pasta do projeto:
```
gcc -std=c11 -Wall -Wextra -O2 -o citizen_assist.exe src\main.c src\lista.c src\orientacao.c src\arquivo.c src\entrada.c
citizen_assist.exe
```
Use o **Windows Terminal** para os acentos aparecerem e serem digitados corretamente.

**Linux, macOS ou WSL:**
```
make            # ou: gcc -std=c11 -Wall -Wextra -O2 -o citizen_assist src/*.c
./citizen_assist
```

O programa carrega `dados/orientacoes.txt`. Para usar outro arquivo: `./citizen_assist minha_base.txt`
(se o arquivo não existir, a lista começa vazia).

## Testes

```
make test                    # 76 testes automatizados + medição de crescimento das ordenações
sh tests/gerar_evidencias.sh # refaz todas as evidências (usa valgrind, se instalado)
```

| Evidência | Conteúdo |
|---|---|
| `tests/resultado_testes.txt` | 76 casos com esperado × observado (lista vazia, inexistentes, inválidos, normais) e tabela de comparações por tamanho de lista |
| `tests/evidencias/sessao_01_operacoes.txt` | menu real com a base: erros de digitação, cadastro, consulta, pesquisas, atualização, ordenações, análise, exclusão |
| `tests/evidencias/sessao_02_lista_vazia.txt` | todas as opções com a lista vazia, primeira inserção e remoção do único nó |
| `tests/evidencias/sessao_03_fim_da_entrada.txt` | entrada encerrada no meio de um cadastro (programa sai com segurança) |
| `tests/evidencias/resumo_memoria.txt` | valgrind: 0 erros e 0 vazamentos em todas as execuções |

Os roteiros de teclas digitadas estão em `tests/roteiros/`. Para ver o que foi digitado na saída, rode com a
variável `CITIZEN_ECO=1`.

## Menu

```
 1) Cadastrar          6) Ordenar e exibir
 2) Consultar por ID   7) Análise da base
 3) Pesquisar          8) Listar todas
 4) Atualizar          9) Salvar no arquivo
 5) Excluir            0) Sair
```

- **Cadastrar:** ID único (sugere o próximo livre), título, categoria (número de uma existente ou nome novo),
  descrição, documentos, local, fonte e data de verificação. Recusa ID duplicado, campos obrigatórios vazios, data
  inválida, texto longo demais e o caractere `|`.
- **Pesquisar:** por trecho do título ou da categoria, sem diferenciar maiúsculas e acentos (`onibus` acha `Ônibus`).
  Sem resultado, mostra as categorias disponíveis.
- **Atualizar:** Enter mantém o valor de cada campo; dado inválido não altera o registro.
- **Ordenar:** título, categoria e título, ID ou data de verificação, com Merge Sort ou Insertion Sort. Mostra o
  número de comparações de cada execução.
- **Análise:** total de registros, quantidade por categoria, fontes verificadas há mais de 30 dias e memória
  ocupada pelos nós.
- Opções inválidas, letras no lugar de números, lista vazia e fim da entrada (Ctrl+D / Ctrl+Z) são tratados sem
  encerrar o programa de forma inesperada.

## Estrutura

```
citizen-assist-ai-data-structures/
├── src/
│   ├── main.c            menu e opções
│   ├── lista.c/.h        lista encadeada: inserir, buscar, pesquisar, atualizar, remover, Merge e Insertion Sort
│   ├── orientacao.c/.h   struct Orientacao, validação, normalização de texto (acentos) e exibição
│   ├── arquivo.c/.h      carregar e salvar a base em texto (extensão opcional)
│   └── entrada.c/.h      leitura segura do teclado
├── dados/orientacoes.txt base inicial (24 orientações, UTF-8, campos separados por |)
├── tests/                testes automatizados, roteiros do menu e evidências
├── docs/                 documentação técnica (PDF), figuras e scripts que geram o PDF
├── ferramentas/          conversor da base JSON do grupo para o formato do programa
└── Makefile
```

## Complexidade (resumo)

| Operação | Melhor | Pior |
|---|---|---|
| Inserir (com checagem de ID duplicado) | O(n) | O(n) |
| Buscar por ID | O(1) | O(n) |
| Pesquisar por título/categoria | O(n·L) | O(n·L) |
| Atualizar | O(1) | O(n) |
| Remover | O(1) | O(n) |
| Merge Sort | O(n log n) | O(n log n) |
| Insertion Sort | O(n) | O(n²) |

*n* = registros na lista; *L* = tamanho do texto comparado. As justificativas e a verificação prática (comparações
medidas de 250 a 4.000 registros) estão na seção 6 da documentação.

## Limitações

- Busca por ID e checagem de duplicado são lineares; para milhares de registros, uma tabela hash por ID seria melhor.
- A pesquisa acha trechos exatos (sem acento), sem tolerar erro de digitação.
- No `cmd` antigo do Windows, letras acentuadas digitadas podem não ser lidas; no Windows Terminal funcionam.
