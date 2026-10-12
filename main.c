#include <stdio.h>
#include <string.h>
#include <time.h>
#include "mapa.h"
#include "lercsv.h"
#include "nave/nave.h"

// AVISO YOKOO.
// nao tire as \ da funcao tempo porque o compilador nao vai achar o restante da funcao.
#define tempo(nome, chamada) do { \
    clock_t inicio = clock(); \
    chamada; \
    clock_t final = clock(); \
    printf("%s: %.3f ms\n", nome, (double)(final - inicio) * 1000.0 / CLOCKS_PER_SEC); \
} while (0)
// Deixe uma linha de espaco da funcao porque a funcao some com ela.
// Retorna 1 caso a leitura do arquivo e 0 caso não nao leu.
int lernome(char *destino) {
    int lido = scanf("%19s", destino);
    int ch;
    if (lido == EOF){
    	return EOF;
    }
    while ((ch = getchar()) != '\n' && ch != EOF) { }
    return lido;
}

// Mostra os vizinhos de um sistema e se a rota esta livre ou bloqueada.
void mostrarvizinhos(grafo *g, int s) {
    Conexao *rota;
    printf("Vizinhos de %s:\n", g->planeta[s].nome);
    for (rota = g->planeta[s].lista; rota != NULL; rota = rota->proximo) {
        printf("  %s (peso %.0f) %s\n", g->planeta[rota->destino].nome, rota->peso,
               rota->ativa ? "livre" : "BLOQUEADA");
    }
}

// deixe pelo menos uma linha de distancia da funcao tempo porque o compilador vai sumir com ela.
int main() {
    grafo mapa;
    resultadodafase resultado;
    Conexao *rota;
    char nomea[MAX_NOME], nomeb[MAX_NOME];     // nomes digitados
    int r;                  // retorno do lercsv e das funcoes de rota
    int opcao = -1;         // opcao escolhida no menu
    int lido;               // quantos numeros o scanf conseguiu ler
    int ch;                 // usado para descartar o que sobrou na linha digitada
    int a = -1, b = -1;     // indices dos sistemas digitados
    int atual;              // sistema onde a nave esta agora (comeca na Terra)
    int peso = 0;           // peso da rota nova
    int perigo;             // perigo da fase (vem do peso da rota)
    tempo("lercsv", r = lercsv(&mapa, "mapa.csv"));
    if (r != 0) {
        printf("Nao abriu mapa.csv, rode o jogo de dentro da pasta ED_MAPA\n");
        return 1;
    }
    printf("Mapa: %d sistemas, %d rotas\n", mapa.totalsistema, mapa.totalconexoes);
    atual = acharsistema(&mapa, "Terra");       // o jogo sempre comeca na Terra
    if (atual < 0) atual = 0;
    do {
        printf("\n=== MAPA ESPACIAL === (voce esta em %s)\n", mapa.planeta[atual].nome);
        printf("1 - Ver o mapa (W A S D anda, ESC volta)\n");
        printf("2 - Ver vizinhos de um sistema\n");
        printf("3 - Viajar (ve o mapa e digita o numero do vizinho)\n");
        printf("4 - Existe rota entre dois sistemas?\n");
        printf("5 - Bloquear uma rota\n");
        printf("6 - Adicionar uma rota\n");
        printf("0 - Sair\n");
        printf("Opcao: ");
        lido = scanf("%d", &opcao);
        if (lido == EOF) {
            opcao = 0;                                  // acabou a entrada: sai do jogo
        } else if (lido == 0) {
            opcao = -1;                                 // digitou letra: cai no default
        }
        if (lido != EOF) {
            while ((ch = getchar()) != '\n' && ch != EOF) { }
        }
        switch (opcao) {
            case 1:
                navegarmapa(&mapa, atual, 0);           // anda com W A S D, ESC volta ao menu
                break;
            case 2:
                printf("Nome ou numero do sistema (Enter = onde voce esta): ");
                a = atual;
                r = lernome(nomea);
                if (r == EOF) { opcao = 0; break; }
                if (r == 1) a = acharsistema(&mapa, nomea);
                if (a < 0) {
                    printf("Sistema '%s' nao existe.\n", nomea);
                } else {
                    mostrarvizinhos(&mapa, a);
                }
                break;
            case 3:
                b = navegarmapa(&mapa, atual, 1);       // ve o mapa e digita o numero do vizinho
                if (b < 0) {
                    printf("Viagem cancelada.\n");
                    break;
                }
                rota = acharconexao(&mapa, atual, b);
                perigo = (int)rota->peso;               // o CSV nao traz perigo: usa o peso (1 a 5)
                if (perigo < 1) perigo = 1;
                if (perigo > 5) perigo = 5;
                resultado = jogar_fase(mapa.planeta[atual].nome, mapa.planeta[b].nome, perigo);
                if (resultado.venceu) {
                    atual = b;                          // a nave chegou: mudou de lugar
                    printf("Voce chegou em %s! A rota continua livre.\n", mapa.planeta[atual].nome);
                } else if (resultado.vida <= 0) {
                    bloquearrota(&mapa, atual, b);
                    printf("Voce perdeu e a rota %s - %s foi BLOQUEADA.\n",
                           mapa.planeta[atual].nome, mapa.planeta[b].nome);
                } else {
                    printf("Voce saiu da fase. Nada mudou.\n");
                }
                imprimirmapa(&mapa, atual);             // o mapa se atualiza sozinho
                break;
            case 4:
                printf("Nome ou numero dos 2 sistemas (ex: Terra 5): ");
                if (scanf("%19s %19s", nomea, nomeb) != 2) {
                    if (feof(stdin)) { opcao = 0; break; }
                    printf("Digite os dois nomes.\n");
                    while ((ch = getchar()) != '\n' && ch != EOF) { }
                    break;
                }
                while ((ch = getchar()) != '\n' && ch != EOF) { }
                a = acharsistema(&mapa, nomea);
                b = acharsistema(&mapa, nomeb);
                if (a < 0 || b < 0) {
                    printf("Sistema nao encontrado.\n");
                } else {
                    tempo("acharconexao", rota = acharconexao(&mapa, a, b));
                    if (rota == NULL) {
                        printf("Nao ha rota direta entre %s e %s.\n", mapa.planeta[a].nome, mapa.planeta[b].nome);
                    } else {
                        printf("Rota %s - %s: peso %.0f, %s.\n", mapa.planeta[a].nome, mapa.planeta[b].nome,
                               rota->peso, rota->ativa ? "livre" : "BLOQUEADA");
                    }
                }
                break;
            case 5:
                printf("Nome ou numero dos 2 sistemas da rota (ex: Terra 5): ");
                if (scanf("%19s %19s", nomea, nomeb) != 2) {
                    if (feof(stdin)) { opcao = 0; break; }
                    printf("Digite os dois nomes.\n");
                    while ((ch = getchar()) != '\n' && ch != EOF) { }
                    break;
                }
                while ((ch = getchar()) != '\n' && ch != EOF) { }
                a = acharsistema(&mapa, nomea);
                b = acharsistema(&mapa, nomeb);
                if (a < 0 || b < 0) {
                    printf("Sistema nao encontrado.\n");
                } else {
                    tempo("bloquearrota", r = bloquearrota(&mapa, a, b));
                    if (r) {
                        printf("Rota %s - %s bloqueada.\n", mapa.planeta[a].nome, mapa.planeta[b].nome);
                        imprimirmapa(&mapa, atual);
                    } else {
                        printf("Essa rota nao existe.\n");
                    }
                }
                break;
            case 6:
                printf("Nome ou numero dos 2 sistemas e o peso (ex: Terra 5 3): ");
                if (scanf("%19s %19s %d", nomea, nomeb, &peso) != 3) {
                    if (feof(stdin)) { opcao = 0; break; }
                    printf("Digite dois nomes e um numero.\n");
                    while ((ch = getchar()) != '\n' && ch != EOF) { }
                    break;
                }
                while ((ch = getchar()) != '\n' && ch != EOF) { }
                a = acharsistema(&mapa, nomea);
                b = acharsistema(&mapa, nomeb);
                if (a < 0 || b < 0) {
                    printf("Sistema nao encontrado.\n");
                } else if (a == b) {
                    printf("Uma rota precisa ligar dois sistemas diferentes.\n");
                } else if (peso <= 0) {
                    printf("O peso precisa ser maior que 0.\n");
                } else if (acharconexao(&mapa, a, b) != NULL) {
                    printf("Essa rota ja existe.\n");
                } else {
                    tempo("adicionarrota", r = adicionarrota(&mapa, a, b, peso));
                    if (r < 0) {
                        printf("Nao foi possivel criar a rota (limite de %d rotas ou sem memoria).\n", MAX_CONEXOES);
                    } else {
                        printf("Rota %s - %s criada com peso %d.\n", mapa.planeta[a].nome, mapa.planeta[b].nome, peso);
                        imprimirmapa(&mapa, atual);
                    }
                }
                break;
            case 0:
                printf("Saindo...\n");
                break;
            default:
                printf("Opcao invalida.\n");
                break;
        }
    } while (opcao != 0);                               // repete ate escolher 0

    liberarmapa(&mapa);
    return 0;
}
