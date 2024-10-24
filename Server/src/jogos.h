#ifndef JOGOS_H
#define JOGOS_H
#include "../config/config.h"

#define MENU_INTERFACE "1. Mostrar tabuleiro\n2. Enviar solucao\n3. Sair\n"

/*Struct para armazenar o tabuleiro e a solucao do jogo.
Aqui usei o typedef para criar um alias para a struct 
(evita repetir 'struct' no código sempre que quisermos usar a struct). */

typedef struct {
    int id; 
    char tabuleiro[9][9];  // Tabuleiro (9x9)
    char solucao[9][9];    // Solucao correta
} Jogo;

// Carrega o jogo do ficheiro
Jogo carregaJogo(ServerConfig config, int idJogo, int idJogador);

// mostra tabuleiro do jogo
void mostraTabuleiro(char * logFileName, Jogo jogo, int idJogador);

// nova funcao para verificar linha a linha
int verificaLinha(char * logFileName, char * solucaoEnviada, Jogo *jogo, int linhaInserida[9], int numeroLinha, int idJogador);

#endif