#include <stdio.h>
#include <stdlib.h>
#include "fila.h"
#define esp 50

void inciarfila(Fila *f){
    f->inicio = 0;
    f->fim = -1;
}

int filavazia(Fila *f){
    return f->fim < f->inicio;
}

int filacheia(Fila *f){
    return f->fim == esp - 1;
}

int Enfileirar(Fila *f, int valor){
    if (filacheia(f)) {
        return 0; // Fila cheia
    }
    f->fim++;
    f->capacidade[f->fim] = valor;
    return 1; // Sucesso
}

int Desenfileirar(Fila *f, int valor){
    if(filavazia(f)){
        return 0; // Fila vazia
    }
    
    valor = f->capacidade[f->inicio];
    f->inicio++;
    return 1; // Sucesso
}

void ExibirFila(Fila *f){
    if(filavazia(f)){
        printf("Fila vazia.\n");
        return;
    }
    printf("Lugares Visitados: ");
    for(int i = f->inicio; i <=f->fim; i++){
        printf("| %d |", f->capacidade[i]);
    }
}
