#ifndef LERCSV_H
#define LERCSV_H

#include "mapa.h" //Caminho para o grafo.

// Le o arquivo e monta o grafo.
// Retorna -1 se nao conseguiu abrir o arquivo.
int lercsv(grafo *g, const char *arquivo);

#endif
