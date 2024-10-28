#ifndef SERVER_GAME_H
#define SERVER_GAME_H

#include "../config/config.h"

/*Struct para armazenar o tabuleiro e a solucao do jogo.
Aqui usei o typedef para criar um alias para a struct 
(evita repetir 'struct' no código sempre que quisermos usar a struct). */

typedef struct {
    int id; 
    char board[9][9];  // Tabuleiro (9x9)
    char solution[9][9];    // Solucao correta
} Game;

// Carrega o jogo do ficheiro
Game loadGame(ServerConfig config, int gameID, int playerID);

// Carrega um jogo aleatório
Game loadRandomGame(ServerConfig config, int playerID);

// nova funcao para verificar linha a linha
int verifyLine(char *logFileName, char *solutionSent, Game *game, int insertLine[9], int lineNumber, int playerID);

#endif