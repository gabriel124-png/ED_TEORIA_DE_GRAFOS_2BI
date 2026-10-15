#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include "nave.h"

#define COLUNAS 7
#define LINHAS  8

void limpartela(void) {
    for (int i = 0; i < 40; i++) { //limpa tela manual.
        printf("\n");
    }
}

resultadodafase jogar_fase(const char *origem, const char *destino, int perigo) {
    resultadodafase r = {0, 3};
    if (perigo < 1) {
        perigo = 1;
    }
    if (perigo > 5) {
        perigo = 5;
    }
    int meta = 5 + perigo * 3;                   // inimigos a abater para vencer
    int intervalo = 4 - perigo / 2;              // de quantos em quantos turnos nasce um inimigo
    int abatidos = 0, turno = 0;
    int nave = COLUNAS / 2;                      // coluna da nave
    int inimigo[LINHAS][COLUNAS] = {0};          // 1 = tem inimigo naquela casa
    srand((unsigned)time(NULL));

    while (abatidos < meta && r.vida > 0) {
        //Desenha o mapa
        limpartela();
        printf("%s -> %s\n", origem, destino);
        printf("Vida: %d   Abatidos: %d/%d\n\n", r.vida, abatidos, meta);
        for (int l = 0; l < LINHAS; l++) {
            for (int c = 0; c < COLUNAS; c++) {
                printf(" %c", inimigo[l][c] ? 'V' : '.');
            }
            printf("\n");
        }
        for (int c = 0; c < COLUNAS; c++) {
            printf(" %c", c == nave ? '^' : ' ');
        }
        printf("\n\nComando (a = esquerda, d = direita, l = atirar, q = sair): ");
        char game;
        if (scanf(" %c", &game) != 1) {// Inverte a tela.
            return r;
        }
        int cac;
        while((cac = getchar()!='\n' && cac != EOF)){}
        if (game == 'q') { //Retorna caso venceu ou vida chegou a zero.
            return r;
        }
        if (game == 'a' && nave > 0) {
            nave--;
        }
        if (game == 'd' && nave < COLUNAS - 1) {
            nave++;
        }
        if (game == 'l') {
            for (int l = LINHAS - 1; l >= 0; l--) {
                if (inimigo[l][nave]) {
                    inimigo[l][nave] = 0;
                    abatidos++;
                    break;                         // um tiro derruba so um inimigo
                }
            }
        }

        for (int c = 0; c < COLUNAS; c++) {        // quem esta na ultima linha chegou ao chao
            if (inimigo[LINHAS - 1][c]) {
                inimigo[LINHAS - 1][c] = 0;
                r.vida--;
            }
        }
        for (int l = LINHAS - 1; l > 0; l--) {       // copia cada linha para a de baixo
            for (int c = 0; c < COLUNAS; c++) {
                inimigo[l][c] = inimigo[l - 1][c];
            }
        }
        for (int c = 0; c < COLUNAS; c++) {          // a linha 0 fica vazia
            inimigo[0][c] = 0;
        }

        // NASCER um inimigo novo na linha 0
        if (turno % intervalo == 0) {
            inimigo[0][rand() % COLUNAS] = 1;
        }
        turno++;
    }
    r.venceu = (abatidos >= meta);
    return r;
}
