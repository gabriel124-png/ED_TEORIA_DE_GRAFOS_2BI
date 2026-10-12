#ifndef MAPA_H
#define MAPA_H

#define MAX_SISTEMAS 120
#define MAX_CONEXOES 200
#define MAX_NOME 20

typedef enum{
    grande,
    media,
    pequeno
} tipoderota;

typedef enum{
    normal,
    nexo
} tipodesistema;

typedef struct Conexao {
    int origem;
    int destino;
    tipoderota tipo;
    float peso; // custo da rota.
    int ativa; // Verifica as rotas estao livres ou nao. 
    int perigo; // verifica perigo de 1 a 6.
    struct Conexao *proximo;
} Conexao;

typedef struct Sistema{
    char nome[MAX_NOME];
    tipodesistema tipo;
    Conexao *lista;
} Sistema;

typedef struct {
    Sistema planeta[MAX_SISTEMAS];
    int totalsistema;
    int totalconexoes;
} grafo;

void iniciarmapa(grafo *g);
void imprimirmapa(grafo *g, int atual);      // mostra a janela do mapa envolta da nave.
int navegarmapa(grafo *g, int atual, int viajar);   // mover no mapa com wasd e sai no esc e retorna um para destino.
int buscarsistema(grafo *g, const char *nome);
int acharsistema(grafo *g, const char *nome);   //procura sistema.
Conexao *acharconexao(grafo *g, int a, int b);
void liberarmapa(grafo *g);
int adicionarrota(grafo *g, int sistemaa, int sistemab, int peso);
int bloquearrota(grafo *g, int a, int b);
//float caminhominimo(grafo *g, int origem, int destino, int caminho[], int *tam);

#endif
