#ifndef SERVER_GAME_H
#define SERVER_GAME_H

#include "../config/config.h"

/*Struct para armazenar o tabuleiro e a solucao do jogo.
Aqui usei o typedef para criar um alias para a struct 
(evita repetir 'struct' no código sempre que quisermos usar a struct). */

// Função para gerar um ID único
int generateUniqueId();

// Carrega o jogo do ficheiro
Game *loadGame(ServerConfig *config, int gameID, int playerID);

// Carrega um jogo aleatório
Game *loadRandomGame(ServerConfig *config, int playerID);

// nova funcao para verificar linha a linha
int verifyLine(char *logFileName, char *solutionSent, Game *game, int insertLine[9], int lineNumber, int playerID);

// Verifica se a linha está correta
bool isLineCorrect(Game *game, int row);

// Criar room
Room *createRoom(ServerConfig *config);

// entrar no jogo
Room *joinRoom(ServerConfig *config, int roomID, int playerID);

// Obter jogos existentes
char *getGames(ServerConfig *config);

// Obter salas existentes
char *getRooms(ServerConfig *config);

#endif