#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "lercsv.h"

#define MAX_LINHA2 200
#define MAX_CORTE 15
#define SEPARADORES ";,\r\n"   // Separador do csv.

// Pega uma palavra e guarda em *s e pega um numero inteiro e guarda o numero em *valor.
int custo(const char *s, int *valor) {
    char *fim;
    long v = strtol(s, &fim, 10); // strtol converte para texto a vairiavel.
    if (fim == s || *fim != '\0' || v <= 0)
    return 0;
    *valor = (int)v; // Estou convertendo v para um inteiro depois guardando  em *valor.
    return 1;
}

int lercsv(grafo *g, const char *arquivo) {
    FILE *f = fopen(arquivo, "r");
    if (f == NULL)// Verifica se o arquivo não está vazio.
    return -1;
    g->totalsistema = 0;
    g->totalconexoes = 0;
    char linha2[MAX_LINHA2];
    int numlinha = 0;
    while (fgets(linha2, sizeof(linha2), f) != NULL) { // Laco principal
        numlinha++;
        char *espaco[MAX_CORTE];
        int n = 0;
        char *t = strtok(linha2, SEPARADORES);
        while (t != NULL && n < MAX_CORTE) {
            espaco[n++] = t;
            t = strtok(NULL, SEPARADORES);
        }
        if (n == 0) continue; // linha vazia

        // Pega o sistema.
        int a = buscarsistema(g, espaco[0]);
        if (a < 0) {
            printf("Linha %d: sem espaco para '%s'\n", numlinha, espaco[0]);
            continue;
        }

        // Verifica se são pares.
        int i = 1;
        while (i < n) {
            int peso;
            if (i + 1 < n && custo(espaco[i + 1], &peso)) {
                int b = buscarsistema(g, espaco[i]);    // vizinho sem linha propria tambem e criado
                if (b < 0 || adicionarrota(g, a, b, peso) < 0)
                    printf("Linha %d: rota %s - %s ignorada\n", numlinha, espaco[0], espaco[i]);
                i += 2;                                  // pegou o par inteiro
            } else {
                printf("Linha %d: '%s' sem peso valido, ignorado\n", numlinha, espaco[i]);
                i += 1;                                  // anda 1 para voltar a casar os pares
            }
        }
    }

    fclose(f);
    return 0;
}
