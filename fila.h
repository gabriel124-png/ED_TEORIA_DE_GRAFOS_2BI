#ifndef fila_H  
#define fila_H

#include <stdio.h>
#include <stdlib.h>
#define esp 50

typedef struct {
    int capacidade[esp];
    int inicio;
    int fim;
} Fila;

void inciarfila(Fila *f);
int filavazia(Fila *f);
int filacheia(Fila *f);
int Enfileirar(Fila *f, int valor);
int Desenfileirar(Fila *f, int valor);
void ExibirFila(Fila *f);

#endif
