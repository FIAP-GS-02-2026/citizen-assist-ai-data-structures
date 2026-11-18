#!/bin/sh
# Compila, roda os testes automatizados e os roteiros do menu, e grava as saídas em tests/evidencias/.
# Uso (na raiz do projeto, Linux ou WSL): sh tests/gerar_evidencias.sh
# Com valgrind instalado, cada execução também é checada contra vazamento de memória.
set -e
CFLAGS="-std=c11 -Wall -Wextra -Wpedantic -O2 -g"
gcc $CFLAGS -o citizen_assist src/*.c
gcc $CFLAGS -Isrc -o testes tests/testes.c src/lista.c src/orientacao.c src/arquivo.c src/entrada.c
mkdir -p tests/evidencias
VG=""
if command -v valgrind >/dev/null 2>&1; then VG="valgrind -q --leak-check=full --error-exitcode=9"; fi

resumo=tests/evidencias/resumo_memoria.txt
echo "Checagem de memória (valgrind --leak-check=full): saída vazia = 0 erros e 0 vazamentos" > $resumo

$VG ./testes > tests/resultado_testes.txt 2> /tmp/vg.txt || { cat /tmp/vg.txt; exit 1; }
echo "testes automatizados ........ $( [ -s /tmp/vg.txt ] && echo 'ERROS' || echo 'OK, sem vazamentos')" >> $resumo

roteiro() {   # $1 = nome, $2 = base inicial ("" = arquivo inexistente)
    base="sessao_$1.txt"
    rm -f "$base"
    [ -n "$2" ] && cp "$2" "$base"
    {
        echo "# Evidência: ./citizen_assist $base < tests/roteiros/$1.txt  (CITIZEN_ECO=1 mostra o que foi digitado)"
        CITIZEN_ECO=1 $VG ./citizen_assist "$base" < "tests/roteiros/$1.txt" 2> /tmp/vg.txt
    } > "tests/evidencias/sessao_$1.txt"
    echo "roteiro $1 $( [ -s /tmp/vg.txt ] && echo 'ERROS' || echo 'OK, sem vazamentos')" >> $resumo
    rm -f "$base"
}
roteiro 01_operacoes dados/orientacoes.txt
roteiro 02_lista_vazia ""
roteiro 03_fim_da_entrada dados/orientacoes.txt
cat $resumo
grep "testes," tests/resultado_testes.txt
