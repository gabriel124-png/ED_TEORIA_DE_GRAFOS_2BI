#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>
#include "mapa.h"

#ifdef _WIN32
#include <conio.h>      // _getch(): ler tecla sem Enter no Windows.
#else
#include <termios.h>    // ler tecla sem Enter no Linux.
#include <unistd.h>
#endif

#define INF 1000000.0f
#define MAPA_LARGURA 252       // largura do mapa inteiro (colunas)
#define MAPA_ALTURA 86        // altura do mapa inteiro (linhas)
#define JANELA_LARGURA 100      // largura da janela mostrada no terminal
#define JANELA_ALTURA 26       // altura da janela mostrada no terminal
#define POSICAO_X 16      // numero de colunas que anda cada a ou d
#define POSICAO_Y 6       // numero de linhas que anda cada w ou s
#define PIXEL_COLUNA 8        // pixels da mapa por coluna.
#define PIXEL_LINHA 16       // pixels da mapa por linha.
#define PIXEL_TOPO 100     // pixel onde comeca a linha 0.

typedef struct { const char *nome;
	int x;
	int y;
} posicaomapa;

// Nomes das regioes do mapa.
typedef struct {
	const char *texto;
	int x; int y;
} textomapa;

int add_sistema(grafo *g, const char *nome, tipodesistema tipo) {
    int i = g->totalsistema;
    strncpy(g->planeta[i].nome, nome, MAX_NOME - 1);
    g->planeta[i].nome[MAX_NOME - 1] = '\0';
    g->planeta[i].tipo = tipo;
    g->planeta[i].lista = NULL;      // sistema novo comeca sem vizinhos
    g->totalsistema++;
    return i;
}

// Cria um no de vizinho na lista do sistema "de", apontando para o sistema "para".
int inserir(grafo *g, int de, int para, tipoderota tipo, float peso, int perigo) {
    Conexao *n = malloc(sizeof(Conexao));
    if (n == NULL){
    	return -1;
    }
    n->origem = de;
    n->destino = para;
    n->tipo = tipo;
    n->peso = peso;
    n->ativa = 1;
    n->perigo = perigo;
    n->proximo = g->planeta[de].lista;      // insere no comeco da lista
    g->planeta[de].lista = n;
    return 0;
}

// Cada rota vira 2 nos: um na lista de sistemaa e outro na lista de sistemab.
int add_conexao(grafo *g, int sistemaa, int sistemab, tipoderota tipo, float peso, int perigo) {
    if (inserir(g, sistemaa, sistemab, tipo, peso, perigo) < 0){
    	return -1;
    }
    if (inserir(g, sistemab, sistemaa, tipo, peso, perigo) < 0){
    	return -1;
    }
    g->totalconexoes++;                     // conta a rota uma vez so
    return 0;
}

// Se o sistema existe devolve o indice; se nao existe, cria e devolve o novo indice.
int buscarsistema(grafo *g, const char *nome) {
    for (int i = 0; i < g->totalsistema; i++) {
        if (strcmp(g->planeta[i].nome, nome) == 0){
            return i;// ja existe
        }
    }
    if (g->totalsistema >= MAX_SISTEMAS){
    	return -1;// sem espaco
    }
    return add_sistema(g, nome, normal); // cria
}

// Cria a rota entre sistemaa e sistemab. Devolve 0 se deu certo (ou se a rota
// ja existia) e -1 se nao ha mais espaco. O CSV traz cada rota nos dois
// sentidos, por isso a repetida nao e erro.
int adicionarrota(grafo *g, int sistemaa, int sistemab, int peso) {
    tipoderota tipo;
    if (acharconexao(g, sistemaa, sistemab) != NULL){
        return 0;// ja existe
    }
    if (g->totalconexoes >= MAX_CONEXOES){
        return -1;// sem espaco
    }
    if (peso <= 1){
        tipo = grande;// peso 1 = rota grande
    } else if (peso <= 3){
        tipo = media;// peso 2 e 3 = rota media
    } else {
        tipo = pequeno;// peso 4 ou mais = rota pequena
    }
    if (add_conexao(g, sistemaa, sistemab, tipo, peso, 0) < 0){
        return -1;// sem memoria
    }
    return 0;
}

//MAPA DO JOGO.
// O mapa abre uma janela no terminal.
// anda no mapa com wasd.
// Posicao em x e y em pixwls do mapa.
const posicaomapa posicoes[] = {
    {"Branaugh",514,150}, {"Kellog",671,241}, {"Tanga",671,298}, {"Volt",284,257}, {"Chronos",540,257},
    {"Veritas",181,351}, {"Vector",348,347}, {"Vega",484,390}, {"Bremen",566,361}, {"Vermilion",118,435},
    {"Tiber",263,424}, {"Oberon",411,434}, {"Orion",215,484}, {"Virgil",342,495}, {"Kallis",599,499},
    {"Virgo",66,530}, {"Viking",177,551}, {"Oso",621,589}, {"Caliban",275,599}, {"Vulture",31,633},
    {"Nul",248,661}, {"Sol",671,683}, {"Centauri",235,726}, {"Vagabond",14,740}, {"Elysium",230,792},
    {"Idris",371,792}, {"Rhetor",462,792}, {"Croshaw",561,792}, {"Vanguard",14,843}, {"Garron",385,886},
    {"Ferron",671,902}, {"Voodoo",31,946}, {"Oretani",671,1001}, {"Banshee",483,1027}, {"Kabal",301,1034},
    {"Vendetta",67,1058}, {"Fora",671,1093}, {"Vesper",119,1151}, {"Leir",327,1221}, {"Bacchus",568,1222},
    {"Geddon",671,1235}, {"Tamsa",411,1277}, {"Yulin",258,1307}, {"Gliese",671,1343},
    {"Trise",996,250}, {"Odin",855,272}, {"Eealus",1172,279}, {"Rihlah",1304,246}, {"Virtus",1059,341},
    {"Nyx",819,374}, {"Castra",1122,431}, {"Tohil",947,444}, {"Gurzil",1335,492}, {"Oya",1202,545},
    {"Pyro",997,564}, {"Cano",844,673}, {"Hadrian",1311,701}, {"Kiel",1405,686}, {"Davien",781,792},
    {"Kilian",881,792}, {"Ellis",973,792}, {"Magnus",1073,792}, {"Stanton",1165,792}, {"Terra",1265,792},
    {"Tayac",1284,856}, {"Cathcart",854,898}, {"Goss",1374,902}, {"Taranis",1165,939}, {"Nexus",929,950},
    {"Min",999,993}, {"Genesis",1050,1023}, {"Hades",863,1026}, {"Helios",1285,1031}, {"Charon",1268,1075},
    {"Nemo",772,1076}, {"Tyrol",1374,1093}, {"Corel",820,1209},
    {"El'sin",1555,157}, {"Khabari",1723,231}, {"Kayfa",1525,262}, {"Markahil",1866,349}, {"Horus",1456,501},
    {"Tal",1702,495}, {"Pallas",1597,590}, {"Baker",1482,789}, {"Hadur",1676,792}, {"Osiris",1447,875},
    {"Ayr'ka",1647,921}, {"Indra",1574,1018}, {"Kins",1666,1122}
};
#define TOTAL_POS ((int)(sizeof(posicoes) / sizeof(posicoes[0])))

const textomapa regioes[] = {
    {"V A N D U U L", 98, 243}, {"SOL", 671, 766}, {"NEXUS", 671, 812},
    {"TERRA", 1375, 768}, {"NEXUS", 1375, 812}, {"X I ' A N", 1635, 361}, {"B A N U", 515, 1348},
    {"GALACTIC TRANSIT NETWORK", 1510, 1283}
};
#define TOTAL_REG ((int)(sizeof(regioes) / sizeof(regioes[0])))
char canvas[MAPA_ALTURA][MAPA_LARGURA + 1];// o mapa inteiro em texto
char ocupado[MAPA_ALTURA][MAPA_LARGURA];// verifica a posicao e devolve 1 caso esteja ocupado.
int colsis[MAX_SISTEMAS];// coluna do mapa
int linsis[MAX_SISTEMAS];// linhas do mapa.
// Compara dois nomes mesmo se tenha letras maiusculas ou minusculas e retorna 1 caso for igual.
int mesmonome(const char *p, const char *q) {
    while (*p != '\0' && *q != '\0' && tolower((unsigned char)*p) == tolower((unsigned char)*q)) {
        p++;
        q++;
    }
    return *p == '\0' && *q == '\0';
}

// Devolve o indice do sistema ou -1. Aceita o nome (Terra) ou o numero (34).
// Diferente de buscarsistema, NAO cria o sistema se ele nao existe.
int acharsistema(grafo *g, const char *nome) {
    if (nome[0] >= '0' && nome[0] <= '9') {
        int v = 0;
        for (const char *p = nome; *p != '\0'; p++) {
            if (*p < '0' || *p > '9' || v > 100000){
            	return -1;
            }
            v = v * 10 + (*p - '0');
        }
        return v < g->totalsistema ? v : -1;
    }
    for (int i = 0; i < g->totalsistema; i++)
        if (mesmonome(g->planeta[i].nome, nome)){
        	return i;
        }
    return -1;
}

// Procura na lista de sistemaa a rota que leva a sistemab.
// Devolve o no da lista, ou NULL se a rota nao existe.
Conexao *acharconexao(grafo *g, int sistemaa, int sistemab) {
    for (Conexao *c = g->planeta[sistemaa].lista; c != NULL; c = c->proximo)
        if (c->destino == sistemab){
        	return c;
        }
    return NULL;
}

int novarota [MAX_CONEXOES][2];
int totalnovas =0;

void marcarrotanova(int pontoum, int pontodois){
	if (totalnovas < MAX_CONEXOES) {
        novarota[totalnovas][0] = pontoum;
        novarota[totalnovas][1] = pontodois;
        totalnovas++;
    }
}

int ehrotanova(int pontoum, int pontodois) {
    for (int i = 0; i < totalnovas; i++)
        if ((novarota[i][0] == pontoum && novarota[i][1] == pontodois) || (novarota[i][0] == pontodois && novarota[i][1] == pontoum)){
            return 1;
        }
    return 0;
}

// Sistemas criados durante o jogo (opcao 6) guardam a posicao aqui, assim o desenho
// nao muda de lugar toda vez que o mapa e montado de novo.
int temnovo[MAX_SISTEMAS];      // 1 = esse sistema foi criado no jogo e tem posicao fixa
int novocol[MAX_SISTEMAS];      // coluna fixa
int novolin[MAX_SISTEMAS];      // linha fixa

// Descobre a coluna/linha de cada sistema usando a tabela de posicoes.
// Sistema que nao esta na tabela (nome errado no CSV) vai para a faixa de baixo.
void posicionar(grafo *g) {
    int extras = 0;
    for (int i = 0; i < g->totalsistema; i++) {
        int achou = -1;
        if (temnovo[i]) {                                   // criado no jogo: usa a posicao guardada
            colsis[i] = novocol[i];
            linsis[i] = novolin[i];
            continue;
        }
        for (int k = 0; k < TOTAL_POS; k++)
            if (mesmonome(posicoes[k].nome, g->planeta[i].nome)) { achou = k; break; }
        if (achou >= 0) {
            colsis[i] = posicoes[achou].x / PIXEL_COLUNA;
            linsis[i] = (posicoes[achou].y - PIXEL_TOPO) / PIXEL_LINHA;
        } else {
            colsis[i] = 6 + (extras % 8) * 26;
            linsis[i] = MAPA_ALTURA - 2 - extras / 8;
            extras++;
        }
    }
}

// Desenha uma rota entre dois sistemas: grande = linha cheia, media = tracejada,
// pequena = pontilhada, bloqueada = x.
void desenharrota(int x0, int y0, int x1, int y1, Conexao *c) {
    int dx = x1 - x0, dy = y1 - y0;
    int adx = dx < 0 ? -dx : dx;
    int ady = dy < 0 ? -dy : dy;
    int passos = adx > ady ? adx : ady;
    int ady2 = ady * 2;                 // uma linha vale ~2 colunas
    char ch;
    if (passos < 1) passos = 1;
    if (adx * 2 > 5 * ady2)             ch = '-';
    else if (adx * 5 < 2 * ady2)        ch = '|';
    else                                ch = (dx * dy < 0) ? '/' : '\\';
    if (c->ativa && c->tipo == pequeno) ch = '.';
    if (!c->ativa)                      ch = 'x';
    for (int s = 0; s <= passos; s++) {
        int x = x0 + (dx * s * 2 + (dx < 0 ? -passos : passos)) / (2 * passos);
        int y = y0 + (dy * s * 2 + (dy < 0 ? -passos : passos)) / (2 * passos);
        char *cel;
        if (x < 0 || x >= MAPA_LARGURA || y < 0 || y >= MAPA_ALTURA) continue;
        if (c->ativa && c->tipo == media && s % 3 == 2) continue;      // buraco do tracejado
        if (c->ativa && c->tipo == pequeno && s % 2 == 1) continue;    // buraco do pontilhado
        cel = &canvas[y][x];
        if (*cel == ' ' || *cel == ch) *cel = ch;
        else if (ch == 'x')            *cel = 'x';
        else if (*cel == 'x')          ;                // bloqueada continua visivel
        else                           *cel = '+';      // cruzamento de rotas
    }
}

// Custo de escrever um texto de tamanho "tam" em (x,y): quanto menor, melhor.
int custotexto(int x, int y, int tam) {
    int custo = 0;
    if (y < 0 || y >= MAPA_ALTURA || x < 0 || x + tam > MAPA_LARGURA){
    	return 100000;
    }
    for (int k = -1; k <= tam; k++) {
        int xx = x + k;
        if (xx < 0 || xx >= MAPA_LARGURA) continue;
        if (ocupado[y][xx]) custo += (k < 0 || k == tam) ? 20 : 1000;   // encostar custa pouco, sobrepor muito
        else if (k >= 0 && k < tam && canvas[y][xx] != ' ') custo += 3; // por cima de uma rota
    }
    return custo;
}

// Conta quantas casas do caminho (x0,y0)-(x1,y1) ja tem alguma coisa desenhada.
// As 2 casas perto de cada ponta nao contam (e onde fica o sistema e o nome dele).
int cobertura(int x0, int y0, int x1, int y1) {
    int dx = x1 - x0, dy = y1 - y0;
    int adx = dx < 0 ? -dx : dx;
    int ady = dy < 0 ? -dy : dy;
    int passos = adx > ady ? adx : ady;
    int usadas = 0;
    if (passos < 1) passos = 1;
    for (int s = 2; s <= passos - 2; s++) {
        int x = x0 + (dx * s * 2 + (dx < 0 ? -passos : passos)) / (2 * passos);
        int y = y0 + (dy * s * 2 + (dy < 0 ? -passos : passos)) / (2 * passos);
        if (x < 0 || x >= MAPA_LARGURA || y < 0 || y >= MAPA_ALTURA) {
            usadas += 1000;                                 // saiu do mapa: nao serve
        } else if (canvas[y][x] != ' ') {
            usadas++;
        }
    }
    return usadas;
}

// Desenha a rota. Se o caminho reto passa por cima de outra rota (ex: Terra-Magnus
// passando por Stanton), desvia: sobe ou desce no meio ate achar um caminho livre.
void desenharrotalivre(int x0, int y0, int x1, int y1, Conexao *c) {
    int dx = x1 - x0, dy = y1 - y0;
    int adx = dx < 0 ? -dx : dx;
    int ady = dy < 0 ? -dy : dy;
    int passos = adx > ady ? adx : ady;
    int mx = (x0 + x1) / 2, my = (y0 + y1) / 2;
    int reto = cobertura(x0, y0, x1, y1);
    static const int desvios[6] = { 3, -3, 5, -5, 7, -7 };
    int melhor = reto, bx = 0, by = 0, achou = 0;
    if (passos >= 6 && reto * 3 > passos) {                 // mais de 1/3 do caminho esta ocupado
        for (int k = 0; k < 6; k++) {
            int wx = mx, wy = my, custo;
            if (adx >= ady * 2) wy = my + desvios[k];       // rota deitada: desvia para cima/baixo
            else                wx = mx + desvios[k] * 2;   // rota em pe: desvia para os lados
            custo = cobertura(x0, y0, wx, wy) + cobertura(wx, wy, x1, y1);
            if (custo < melhor) {
                melhor = custo;
                bx = wx;
                by = wy;
                achou = 1;
            }
        }
    }
    if (achou) {
        desenharrota(x0, y0, bx, by, c);
        desenharrota(bx, by, x1, y1, c);
    } else {
        desenharrota(x0, y0, x1, y1, c);
    }
}

// Monta o mapa inteiro em "canvas". "atual" e onde a nave esta.
void montarcanvas(grafo *g, int atual) {
    static const int ox[10] = { 2, -1, 0, 0, 1, 1, 0, 0, 0, 0 };    // jeitos de colocar o nome
    for (int y = 0; y < MAPA_ALTURA; y++) {
        for (int x = 0; x < MAPA_LARGURA; x++) {
            canvas[y][x] = ' ';
            ocupado[y][x] = 0;
        }
        canvas[y][MAPA_LARGURA] = '\0';
    }
    posicionar(g);
    for (int i = 0; i < g->totalsistema; i++)               // 1) rotas
        for (Conexao *c = g->planeta[i].lista; c != NULL; c = c->proximo)
            if (c->origem < c->destino)                     // cada rota esta em 2 listas: desenha uma vez
                desenharrota(colsis[c->origem], linsis[c->origem], colsis[c->destino], linsis[c->destino], c);
    for (int r = 0; r < TOTAL_REG; r++) {                   // 2) nomes das regioes
        int len = (int)strlen(regioes[r].texto);
        int x0 = regioes[r].x / PIXEL_COLUNA - len / 2;
        int y0 = (regioes[r].y - PIXEL_TOPO) / PIXEL_LINHA;
        for (int k = 0; k < len; k++)
            if (x0 + k >= 0 && x0 + k < MAPA_LARGURA && y0 >= 0 && y0 < MAPA_ALTURA && canvas[y0][x0 + k] == ' ')
                canvas[y0][x0 + k] = regioes[r].texto[k];
    }
    for (int i = 0; i < g->totalsistema; i++) {             // 3) ponto de cada sistema
        char marca = 'o';
        if (i == atual) marca = '@';
        else {
            Conexao *c = acharconexao(g, atual, i);
            if (c != NULL && c->ativa) marca = '*';         // da para viajar direto para ca
        }
        canvas[linsis[i]][colsis[i]] = marca;
        ocupado[linsis[i]][colsis[i]] = 1;
    }
    for (int i = 0; i < g->totalsistema; i++) {             // 4) nomes (com o numero do sistema)
        char texto[MAX_NOME + 24];
        int tam, melhor = 0, customelhor = 1 << 30, x0 = 0, y0 = 0;
        if (i == atual) snprintf(texto, sizeof(texto), "[%s %d]", g->planeta[i].nome, i);
        else            snprintf(texto, sizeof(texto), "%s %d", g->planeta[i].nome, i);
        tam = (int)strlen(texto);
        for (int m = 0; m < 10; m++) {
            int x, y, custo;
            // m: 0 direita, 1 esquerda, 2 em cima, 3 embaixo, 4 cima-direita,
            //    5 baixo-direita, 6 cima-esquerda, 7 baixo-esquerda, 8 e 9 duas linhas longe
            x = colsis[i] + ox[m];
            y = linsis[i];
            if (m == 1) x = colsis[i] - 1 - tam;
            if (m == 2 || m == 3 || m == 8 || m == 9) x = colsis[i] - tam / 2;
            if (m == 6 || m == 7) x = colsis[i] - tam;
            if (m == 2 || m == 4 || m == 6) y = linsis[i] - 1;
            if (m == 3 || m == 5 || m == 7) y = linsis[i] + 1;
            if (m == 8) y = linsis[i] - 2;
            if (m == 9) y = linsis[i] + 2;
            custo = custotexto(x, y, tam) * 10 + m;
            if (custo < customelhor) {
                customelhor = custo;
                melhor = m;
                x0 = x;
                y0 = y;
            }
        }
        (void)melhor;
        if (customelhor >= 100000 * 10) continue;           // nao coube em lugar nenhum
        for (int k = 0; k < tam; k++) {
            canvas[y0][x0 + k] = texto[k];
            ocupado[y0][x0 + k] = 1;
        }
    }
}

int criarsistema(grafo *g, const char *nome, int referencia, int atual) {
    int tam = (int)strlen(nome);
    int idx, rx, ry, tamlabel, melhor = 1 << 30, bx = -1, by = -1;
    if (tam == 0 || tam >= MAX_NOME || (nome[0] >= '0' && nome[0] <= '9')){
    	return -1;
    }
    if (acharsistema(g, nome) >= 0){
    	return -2;
    }
    if (g->totalsistema >= MAX_SISTEMAS){
    	return -3;
    }
    idx = g->totalsistema;                                  // o novo sistema ainda nao conta no desenho
    montarcanvas(g, atual);
    if (referencia < 0 || referencia >= idx) referencia = atual;
    rx = colsis[referencia];
    ry = linsis[referencia];
    tamlabel = tam + 6;                                     // "Nome 87" mais folga dos lados
    for (int dy = -16; dy <= 16; dy++) {
        for (int dx = -50; dx <= 50; dx++) {
            int x = rx + dx, y = ry + dy, livre = 1, score, cob;
            if ((dx < 0 ? -dx : dx) < 8 && (dy < 0 ? -dy : dy) < 3) continue;   // longe do vizinho
            if (x < 3 || x + tamlabel >= MAPA_LARGURA || y < 2 || y >= MAPA_ALTURA - 3) continue;
            for (int yy = y - 1; yy <= y + 1 && livre; yy++)    // area do ponto e do nome tem que estar vazia
                for (int xx = x - 1; xx <= x + tamlabel; xx++)
                    if (canvas[yy][xx] != ' ') { livre = 0; break; }
            if (!livre) continue;
            cob = cobertura(rx, ry, x, y);                      // rota ate o vizinho tambem tem que estar livre
            score = ((dx < 0 ? -dx : dx) / 2 + (dy < 0 ? -dy : dy)) + cob * 10;
            if (score < melhor){
            	melhor = score;
            	bx = x;
            	by = y;
            }
        }
    }
    if (bx < 0){
    	return -4;
    }
    idx = add_sistema(g, nome, normal);
    temnovo[idx] = 1;
    novocol[idx] = bx;
    novolin[idx] = by;
    return idx;
}

// Teclado sem enter no Linux e Windows.
#ifdef _WIN32
void modoraw(int ligar) { (void)ligar; }

// Devolve a tecla: letras/numeros normais, 27 = ESC, '\n' = Enter, 127 = Backspace, 0 = ignorar.
int lertecla(void) {
    int c = _getch();
    if (c == 0 || c == 224) { _getch(); return 0; }       // setas e teclas especiais
    if (c == '\r'){
    	return '\n';
    }
    if (c == 8){
    	return 127;
    }
    return c;
}
#else
static struct termios termantigo;
int rawativo = 0;

void modoraw(int ligar) {
    if (ligar) {
        struct termios t;
        if (!isatty(0) || tcgetattr(0, &termantigo) != 0){
        	return;
        }
        t = termantigo;
        t.c_lflag &= ~(ICANON | ECHO);      // ler tecla por tecla e nao mostrar o que digita
        t.c_cc[VMIN] = 0;
        t.c_cc[VTIME] = 1;                  // espera ate 0,1 s por uma tecla
        tcsetattr(0, TCSANOW, &t);
        rawativo = 1;
    } else if (rawativo) {
        tcsetattr(0, TCSANOW, &termantigo);
        rawativo = 0;
    }
}

int lertecla(void) {
    unsigned char c, d;
    int n;
    if (!rawativo) {                        // entrada de arquivo/pipe: usa getchar
        int ch = getchar();
        return ch == EOF ? 27 : ch;
    }
    do {
    	n = (int)read(0, &c, 1);
    }while(n == 0);
    if (n < 0){
    	return 27;
    }
    if (c == 27) {                          // ESC sozinho ou comeco de seta (ESC [ A)?
        n = (int)read(0, &d, 1);
        if (n == 0) return 27;              // ESC sozinho
        if (d == '[' || d == 'O') {
            do { n = (int)read(0, &d, 1); } while (n == 1 && !(d >= 0x40 && d <= 0x7E));
        }
        return 0;                           // seta: ignora
    }
    return c;
}
#endif

// Mostra a janela do mapa que comeca em (vx, vy).
void desenharquadro(grafo *g, int atual, int vx, int vy, int viajar, const char *digitos, const char *msg) {
    Conexao *c;
    int usado = 0;
    char viz[200];

    for (int i = 0; i < 45; i++){
    	putchar('\n');             // limpa a tela sem system("clear")
    }
    printf("MAPA GALACTICO  |  voce esta em: %s %d  |  janela: colunas %d-%d de %d, linhas %d-%d de %d\n",
           g->planeta[atual].nome, atual, vx + 1, vx + JANELA_LARGURA, MAPA_LARGURA, vy + 1, vy + JANELA_ALTURA, MAPA_ALTURA);
    printf("+");
    for (int x = 0; x < JANELA_LARGURA; x++) putchar('-');
    printf("+\n");
    for (int y = 0; y < JANELA_ALTURA; y++)
        printf("|%.*s|\n", JANELA_LARGURA, &canvas[vy + y][vx]);
    printf("+");
    for (int x = 0; x < JANELA_LARGURA; x++) putchar('-');
    printf("+\n");

    viz[0] = '\0';
    for (c = g->planeta[atual].lista; c != NULL; c = c->proximo) {
        if (!c->ativa) continue;
        usado += snprintf(viz + usado, sizeof(viz) - usado, "%s%s %d", usado ? ", " : "",
                          g->planeta[c->destino].nome, c->destino);
        if (usado > (int)sizeof(viz) - 30) break;
    }
    printf("@ voce | * vizinho livre | o sistema | - cheia = grande | -- -- media | . . pequena | x bloqueada\n");
    printf("Vizinhos livres: %s\n", viz[0] ? viz : "nenhum");
    if (viajar)
        printf("W A S D move o mapa | digite o NUMERO do vizinho + Enter para viajar | ESC volta ao menu\n");
    else
        printf("W A S D move o mapa | ESC volta ao menu\n");
    if (viajar) printf("Destino: %s_   %s\n", digitos, msg);
    else        printf("%s\n", msg);
}

// Centraliza a janela no sistema "s".
void centralizar(int s, int *vx, int *vy) {
    *vx = colsis[s] - JANELA_LARGURA / 2;
    *vy = linsis[s] - JANELA_ALTURA / 2;
    if (*vx < 0) *vx = 0;
    if (*vy < 0) *vy = 0;
    if (*vx > MAPA_LARGURA - JANELA_LARGURA) *vx = MAPA_LARGURA - JANELA_LARGURA;
    if (*vy > MAPA_ALTURA - JANELA_ALTURA) *vy = MAPA_ALTURA - JANELA_ALTURA;
}

// Imprime uma vez a janela do mapa em volta da nave (sem esperar tecla).
void imprimirmapa(grafo *g, int atual) {
    int vx, vy;
    if (g->totalsistema == 0) {
        printf("Mapa vazio.\n");
        return;
    }
    montarcanvas(g, atual);
    centralizar(atual, &vx, &vy);
    desenharquadro(g, atual, vx, vy, 0, "", "Use a opcao 1 do menu para andar pelo mapa com W A S D.");
}

// Mapa interativo. Anda com W A S D e sai com ESC.
// Se viajar for 1, digite o numero de um vizinho e Enter: devolve o indice do destino.
// Devolve -1 se saiu com ESC.
int navegarmapa(grafo *g, int atual, int viajar) {
    return navegarmapafoco(g, atual, viajar, atual, "");
}

// Igual ao navegarmapa, mas abre a janela centrada no sistema "foco" e mostra "aviso" na primeira tela.
int navegarmapafoco(grafo *g, int atual, int viajar, int foco, const char *aviso) {
    int vx, vy, tecla, nd = 0, destino = -1, fim = 0;
    char digitos[8] = "";
    char msg[160] = "";

    snprintf(msg, sizeof(msg), "%s", aviso);
    if (foco < 0 || foco >= g->totalsistema) foco = atual;

    if (g->totalsistema == 0) {
        printf("Mapa vazio.\n");
        return -1;
    }
    montarcanvas(g, atual);
    centralizar(foco, &vx, &vy);
    modoraw(1);
    while (!fim) {
        desenharquadro(g, atual, vx, vy, viajar, digitos, msg);
        msg[0] = '\0';
        fflush(stdout);
        tecla = lertecla();
        if (tecla == 'w' || tecla == 'W') vy -= POSICAO_Y;
        else if (tecla == 's' || tecla == 'S') vy += POSICAO_Y;
        else if (tecla == 'a' || tecla == 'A') vx -= POSICAO_X;
        else if (tecla == 'd' || tecla == 'D') vx += POSICAO_X;
        else if (tecla == 27) fim = 1;
        else if (viajar && tecla >= '0' && tecla <= '9') {
            if (nd < 3) { digitos[nd++] = (char)tecla; digitos[nd] = '\0'; }
        } else if (viajar && tecla == 127) {
            if (nd > 0) digitos[--nd] = '\0';
        } else if (viajar && tecla == '\n' && nd > 0) {
            int b = atoi(digitos);
            Conexao *rota;
            if (b >= g->totalsistema) {
                snprintf(msg, sizeof(msg), "O sistema %d nao existe (vai de 0 a %d).", b, g->totalsistema - 1);
            } else if (b == atual) {
                snprintf(msg, sizeof(msg), "Voce ja esta em %s.", g->planeta[b].nome);
            } else if ((rota = acharconexao(g, atual, b)) == NULL) {
                snprintf(msg, sizeof(msg), "%s %d nao e vizinho de %s: so da para ir para os marcados com *.",
                         g->planeta[b].nome, b, g->planeta[atual].nome);
            } else if (!rota->ativa) {
                snprintf(msg, sizeof(msg), "A rota para %s %d esta BLOQUEADA.", g->planeta[b].nome, b);
            } else {
                destino = b;
                fim = 1;
            }
            nd = 0;
            digitos[0] = '\0';
        }
        if (vx < 0) vx = 0;
        if (vy < 0) vy = 0;
        if (vx > MAPA_LARGURA - JANELA_LARGURA) vx = MAPA_LARGURA - JANELA_LARGURA;
        if (vy > MAPA_ALTURA - JANELA_ALTURA) vy = MAPA_ALTURA - JANELA_ALTURA;
    }
    modoraw(0);
    return destino;
}

// Bloqueia a rota e retorna 1 caso tenha dado sucesso.
int bloquearrota(grafo *g, int sistemaa, int sistemab) {
    Conexao *x = acharconexao(g, sistemaa, sistemab);
    Conexao *y = acharconexao(g, sistemab, sistemaa);
    if (x == NULL || y == NULL){
    	return 0;
    }
    x->ativa = 0;
    y->ativa = 0;
    return 1;
}

// Libera da memoria todos os nos das listas e zera o grafo.
void liberarmapa(grafo *g) {
    for (int i = 0; i < g->totalsistema; i++) {
        Conexao *c = g->planeta[i].lista;
        while (c != NULL) {
            Conexao *prox = c->proximo;// guarda o proximo antes de liberar
            free(c);
            c = prox;
        }
        g->planeta[i].lista = NULL;
    }
    g->totalsistema = 0;
    g->totalconexoes = 0;
}
