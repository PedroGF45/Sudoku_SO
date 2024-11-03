#ifndef SERVER_GAME_H
#define SERVER_GAME_H

#include "../config/config.h"


// Gera um ID único para uma sala.
int generateUniqueId();

// Carrega um jogo específico a partir do ficheiro 'games.json'.
Game *loadGame(ServerConfig *config, int gameID, int playerID);

// Carrega um jogo aleatório do ficheiro 'games.json'.
Game *loadRandomGame(ServerConfig *config, int playerID);

// Verifica se a linha inserida pelo jogador está correta.
int verifyLine(char *logFileName, char *solutionSent, Game *game, int insertLine[9], int lineNumber, int playerID);

// Verifica se uma linha do tabuleiro está correta.
bool isLineCorrect(Game *game, int row);

// Cria uma nova sala de jogo.
Room *createRoom(ServerConfig *config);

// Junta um jogador a uma sala existente.
Room *joinRoom(ServerConfig *config, int roomID, int playerID);

// Obtém uma lista de IDs dos jogos disponíveis.
char *getGames(ServerConfig *config);

// Obtém uma lista das salas de jogo disponíveis.
char *getRooms(ServerConfig *config);

#endif