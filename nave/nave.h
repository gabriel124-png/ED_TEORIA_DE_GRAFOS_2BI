#ifndef NAVE_H
#define NAVE_H

typedef struct {
    int venceu;
    int vida;
} resultadodafase;

resultadodafase jogar_fase(const char *origem, const char *destino, int perigo);

#endif
