# Citizen Assist AI Mobilidade - Data Structures and Algorithms (GS 2026.2)
CC      = gcc
CFLAGS  = -std=c11 -Wall -Wextra -Wpedantic -O2
FONTES  = src/main.c src/lista.c src/orientacao.c src/arquivo.c src/entrada.c
MODULOS = src/lista.c src/orientacao.c src/arquivo.c src/entrada.c

citizen_assist: $(FONTES) src/*.h
	$(CC) $(CFLAGS) -o $@ $(FONTES)

testes: tests/testes.c $(MODULOS) src/*.h
	$(CC) $(CFLAGS) -Isrc -o $@ tests/testes.c $(MODULOS)

run: citizen_assist
	./citizen_assist

test: testes
	./testes

evidencias:
	sh tests/gerar_evidencias.sh

clean:
	rm -f citizen_assist citizen_assist.exe testes testes.exe

.PHONY: run test evidencias clean
